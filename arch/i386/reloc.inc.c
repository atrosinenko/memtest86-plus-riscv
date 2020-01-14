
/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static inline Elf32_Addr __attribute__ ((unused))
elf_machine_dynamic (void)
{
	register Elf32_Addr *got asm ("%ebx");
	return *got;
}

/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr __attribute__ ((unused))
elf_machine_load_address (void)
{
	Elf32_Addr addr;
	asm volatile ("leal _start@GOTOFF(%%ebx), %0\n"
		: "=r" (addr) : : "cc");
	return addr;
}

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */
static inline void
elf_machine_rel (struct link_map *map, const Elf32_Rel *reloc,
		 const Elf32_Sym *sym, Elf32_Addr *const reloc_addr)
{
	Elf32_Addr ls_addr, s_addr;
	Elf32_Addr value;
	if (ELF32_R_TYPE (reloc->r_info) == R_386_RELATIVE)
	{
		*reloc_addr += map->l_addr - map->ll_addr;
		return;
	}
	if (ELF32_R_TYPE(reloc->r_info) == R_386_NONE) {
		return;
	}
	value = sym->st_value;
	/* Every section except the undefined section has a base of map->l_addr */
	ls_addr = sym->st_shndx == SHN_UNDEF ? 0 : map->ll_addr;
	s_addr = sym->st_shndx == SHN_UNDEF ? 0 : map->l_addr;

	switch (ELF32_R_TYPE (reloc->r_info))
	{
	case R_386_COPY:
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
	case R_386_GLOB_DAT:
		*reloc_addr = s_addr + value;
		break;
	case R_386_JMP_SLOT:
		*reloc_addr = s_addr + value;
		break;
	case R_386_32:
		if (map->ll_addr == 0) {
			*reloc_addr += value;
		}
		*reloc_addr += s_addr - ls_addr;
		break;
	case R_386_PC32:
		if (map->ll_addr == 0) {
			*reloc_addr += value - reloc->r_offset;
		}
		*reloc_addr += (s_addr - map->l_addr) - (ls_addr - map->ll_addr);
		break;
	default:
		assert (! "unexpected dynamic reloc type");
		break;
	}
}
