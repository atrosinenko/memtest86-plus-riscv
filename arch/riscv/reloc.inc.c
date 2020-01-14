// based on reloc.c from MemTest86+ v5.1

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static inline ElfW(Addr) __attribute__ ((unused))
elf_machine_dynamic (void)
{
	uintptr_t start, dyn;
	asm volatile (
		"lla %0, _start\n"
		"lla %1, _dynamic\n"
		: "=r" (start), "=r" (dyn) : : "cc");
	return dyn - start;
}

/* Return the run-time load address of the shared object.  */
static inline ElfW(Addr) __attribute__ ((unused))
elf_machine_load_address (void)
{
	uintptr_t res;
	asm volatile (
		"lla %0, _start\n"
		: "=r" (res) : : "cc");
	return res;
}

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */
static inline void
elf_machine_rel_rela (struct link_map *map, const ElfW(Word) r_info, const ElfW(Addr) r_addend,
		 const ElfW(Sym) *sym, void *const reloc_addr)
{
	ElfW(Addr) ls_addr, s_addr;
	ElfW(Addr) value;
	if (ELFW(R_TYPE) (r_info) == R_RISCV_NONE) {
		return;
	}
	value = sym->st_value;
	/* Every section except the undefined section has a base of map->l_addr */
	ls_addr = sym->st_shndx == SHN_UNDEF ? 0 : map->ll_addr;
	s_addr = sym->st_shndx == SHN_UNDEF ? 0 : map->l_addr;

	switch (ELFW(R_TYPE) (r_info))
	{
	case R_RISCV_RELATIVE:
		if (map->ll_addr == 0) {
			*(uint64_t *)reloc_addr += r_addend;
		}
		*(uint64_t *)reloc_addr += map->l_addr - map->ll_addr;
		break;
	case R_RISCV_COPY:
	{
		/* Roll memcpy by hand as we don't have function calls yet. */
		unsigned char *dest, *src;
		long i;
		dest = (unsigned char *)reloc_addr;
		src = (unsigned char *)(value + s_addr);
		for(i = 0; i < sym->st_size; i++) {
			dest[i] = src[i];
		}
	}
		break;
	case R_RISCV_32:
		*(uint32_t *)reloc_addr = value + r_addend;
		break;
	case R_RISCV_64:
		*(uint64_t *)reloc_addr = value + r_addend;
		break;
	default:
		assert (! "unexpected dynamic reloc type");
		break;
	}
}

static inline void
elf_machine_rel (struct link_map *map, const ElfW(Rel) *reloc,
         const ElfW(Sym) *sym, ElfW(Addr) *const reloc_addr)
{
	elf_machine_rel_rela(map, reloc->r_info, 0, sym, reloc_addr);
}


static inline void
elf_machine_rela (struct link_map *map, const ElfW(Rela) *reloc,
		 const ElfW(Sym) *sym, ElfW(Addr) *const reloc_addr)
{
	elf_machine_rel_rela(map, reloc->r_info, reloc->r_addend, sym, reloc_addr);
}
