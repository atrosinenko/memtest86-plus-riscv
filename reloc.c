/* reloc.c - MemTest-86  Version 3.3
 *
 * Released under version 2 of the Gnu Public License.
 * By Eric Biederman
 */

#include "arch.h"
#include "stddef.h"
#include "stdint.h"
#include "elf.h"

/* We use this macro to refer to ELF types independent of the native wordsize.
   `ElfW(TYPE)' is used in place of `Elf32_TYPE' or `Elf64_TYPE'.  */

#define ElfW(type)	_ElfW (Elf, __ELF_NATIVE_CLASS, type)
#define _ElfW(e,w,t)	_ElfW_1 (e, w, _##t)
#define _ElfW_1(e,w,t)	e##w##t
/* We use this macro to refer to ELF types independent of the native wordsize.
   `ElfW(TYPE)' is used in place of `Elf32_TYPE' or `Elf64_TYPE'.  */
#define ELFW(type)	_ElfW (ELF, __ELF_NATIVE_CLASS, type)

#ifndef assert
#define assert(expr) ((void) 0)
#endif

  /* This #define produces dynamic linking inline functions for
     bootstrap relocation instead of general-purpose relocation.  */
#define RTLD_BOOTSTRAP

struct link_map 
{
	ElfW(Addr) l_addr;  /* Current load address */
	ElfW(Addr) ll_addr; /* Last load address */
	ElfW(Dyn)  *l_ld;
    /* Indexed pointers to dynamic section.
       [0,DT_NUM) are indexed by the processor-independent tags.
       [DT_NUM,DT_NUM+DT_PROCNUM) are indexed by the tag minus DT_LOPROC.
       [DT_NUM+DT_PROCNUM,DT_NUM+DT_PROCNUM+DT_EXTRANUM) are indexed
       by DT_EXTRATAGIDX(tagvalue) and
       [DT_NUM+DT_PROCNUM,
        DT_NUM+DT_PROCNUM+DT_EXTRANUM)
       are indexed by DT_EXTRATAGIDX(tagvalue) (see <elf.h>).  */

	ElfW(Dyn)  *l_info[DT_NUM + DT_PROCNUM + DT_EXTRANUM];
};

#include "reloc.inc.c"

/* Read the dynamic section at DYN and fill in INFO with indices DT_*.  */

static inline void __attribute__ ((unused))
elf_get_dynamic_info(ElfW(Dyn) *dyn, ElfW(Addr) l_addr,
	ElfW(Dyn) *info[DT_NUM + DT_PROCNUM + DT_EXTRANUM])
{
	if (! dyn)
		return;
	
	while (dyn->d_tag != DT_NULL)
	{
		if (dyn->d_tag < DT_NUM)
			info[dyn->d_tag] = dyn;
		else if (dyn->d_tag >= DT_LOPROC &&
			dyn->d_tag < DT_LOPROC + DT_PROCNUM)
			info[dyn->d_tag - DT_LOPROC + DT_NUM] = dyn;
		else if ((Elf32_Word) DT_EXTRATAGIDX (dyn->d_tag) < DT_EXTRANUM)
			info[DT_EXTRATAGIDX (dyn->d_tag) + DT_NUM + DT_PROCNUM
				] = dyn;
#if 0
		else
			assert (! "bad dynamic tag");
#endif
		++dyn;
	}
	
	if (info[DT_PLTGOT] != NULL) 
		info[DT_PLTGOT]->d_un.d_ptr += l_addr;
	if (info[DT_STRTAB] != NULL)
		info[DT_STRTAB]->d_un.d_ptr += l_addr;
	if (info[DT_SYMTAB] != NULL)
		info[DT_SYMTAB]->d_un.d_ptr += l_addr;
#if ! ELF_MACHINE_NO_RELA
	if (info[DT_RELA] != NULL)
	{
		assert (info[DT_RELAENT]->d_un.d_val == sizeof (ElfW(Rela)));
		info[DT_RELA]->d_un.d_ptr += l_addr;
	}
#endif
#if ! ELF_MACHINE_NO_REL
	if (info[DT_REL] != NULL)
	{
		assert (info[DT_RELENT]->d_un.d_val == sizeof (ElfW(Rel)));
		info[DT_REL]->d_un.d_ptr += l_addr;
	}
#endif
	if (info[DT_PLTREL] != NULL)
	{
#if ELF_MACHINE_NO_RELA
		assert (info[DT_PLTREL]->d_un.d_val == DT_REL);
#elif ELF_MACHINE_NO_REL
		assert (info[DT_PLTREL]->d_un.d_val == DT_RELA);
#else
		assert (info[DT_PLTREL]->d_un.d_val == DT_REL
			|| info[DT_PLTREL]->d_un.d_val == DT_RELA);
#endif
	}
	if (info[DT_JMPREL] != NULL)
		info[DT_JMPREL]->d_un.d_ptr += l_addr;
}



/* Perform the relocations in MAP on the running program image as specified
   by RELTAG, SZTAG.  If LAZY is nonzero, this is the first pass on PLT
   relocations; they should be set up to call _dl_runtime_resolve, rather
   than fully resolved now.  */

static inline void
elf_dynamic_do_rel (struct link_map *map,
		    ElfW(Addr) reladdr, ElfW(Addr) relsize)
{
	const ElfW(Rel) *r = (const void *) reladdr;
	const ElfW(Rel) *end = (const void *) (reladdr + relsize);

	const ElfW(Sym) *const symtab =
		(const void *) map->l_info[DT_SYMTAB]->d_un.d_ptr;
	
	for (; r < end; ++r) {
		elf_machine_rel (map, r, &symtab[ELFW(R_SYM) (r->r_info)],
			(void *) (map->l_addr + r->r_offset));
	}
}

#if ! ELF_MACHINE_NO_RELA
static inline void
elf_dynamic_do_rela (struct link_map *map,
		    ElfW(Addr) relaaddr, ElfW(Addr) relasize)
{
	const ElfW(Rela) *r = (const void *) relaaddr;
	const ElfW(Rela) *end = (const void *) (relaaddr + relasize);

	const ElfW(Sym) *const symtab =
		(const void *) map->l_info[DT_SYMTAB]->d_un.d_ptr;
	
	for (; r < end; ++r) {
		elf_machine_rela (map, r, &symtab[ELFW(R_SYM) (r->r_info)],
			(void *) (map->l_addr + r->r_offset));
	}
}
#endif


void _dl_start(void)
{
	static ElfW(Addr) last_load_address = 0;
	struct link_map map;
	size_t cnt;


	/* Partly clean the `map' structure up.  Don't use `memset'
	   since it might nor be built in or inlined and we cannot make function
	   calls at this point.  */
	for (cnt = 0; cnt < sizeof(map.l_info) / sizeof(map.l_info[0]); ++cnt) {
		map.l_info[cnt] = 0;
	}

	/* Get the last load address */
	map.ll_addr = last_load_address;

	/* Figure out the run-time load address of the dynamic linker itself.  */
	last_load_address = map.l_addr = elf_machine_load_address();
	
	/* Read our own dynamic section and fill in the info array.  */
	map.l_ld = (void *)map.l_addr + elf_machine_dynamic();

	elf_get_dynamic_info (map.l_ld, map.l_addr - map.ll_addr, map.l_info);

	/* Relocate ourselves so we can do normal function calls and
	 * data access using the global offset table.  
	 */
#if !ELF_MACHINE_NO_REL
	elf_dynamic_do_rel(&map, 
		map.l_info[DT_REL]->d_un.d_ptr,
		map.l_info[DT_RELSZ]->d_un.d_val);
	if (map.l_info[DT_PLTREL]->d_un.d_val == DT_REL) {
		elf_dynamic_do_rel(&map, 
			map.l_info[DT_JMPREL]->d_un.d_ptr,
			map.l_info[DT_PLTRELSZ]->d_un.d_val);
	}
#endif

#if !ELF_MACHINE_NO_RELA
	elf_dynamic_do_rela(&map, 
		map.l_info[DT_RELA]->d_un.d_ptr,
		map.l_info[DT_RELASZ]->d_un.d_val);
	if (map.l_info[DT_PLTREL]->d_un.d_val == DT_RELA) {
		elf_dynamic_do_rela(&map, 
			map.l_info[DT_JMPREL]->d_un.d_ptr,
			map.l_info[DT_PLTRELSZ]->d_un.d_val);
	}
#endif

	/* Now life is sane; we can call functions and access global data.
	   Set up to use the operating system facilities, and find out from
	   the operating system's program loader where to find the program
	   header table in core.  Put the rest of _dl_start into a separate
	   function, that way the compiler cannot put accesses to the GOT
	   before ELF_DYNAMIC_RELOCATE.  */
	return;
}

