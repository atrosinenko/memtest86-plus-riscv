#ifndef ARCH_H
#define ARCH_H

#include <stdint.h>

#include "config.h"
#include "globals.h"

extern struct	cpu_ident cpu_id;

#define HAS_SMP 1
#define HAS_FLAT_MEM 0

#define RDTSC_AVAILABLE() (cpu_id.fid.bits.rdtsc)
#define RDTSC_LH(low, high) asm __volatile__ ("rdtsc":"=a" (low),"=d" (high))

#define __ELF_NATIVE_CLASS 32
#define ELF_MACHINE_NO_RELA 1

#define SCREEN_ADR  0xb8000
#define SCREEN_END_ADR  (SCREEN_ADR + 80*25*2)


#define E88     0x00
#define E801    0x04
#define E820NR  0x08           /* # entries in E820MAP */
#define E820MAP 0x0c           /* our map */
#define E820MAX 127            /* number of entries in E820MAP */
#define E820ENTRY_SIZE 20
#define MEMINFO_SIZE (E820MAP + E820MAX * E820ENTRY_SIZE)


#ifndef __ASSEMBLY__

#define E820_RAM        1
#define E820_RESERVED   2
#define E820_ACPI       3 /* usable as RAM once ACPI tables have been read */
#define E820_NVS        4

struct e820entry {
        unsigned long long addr;        /* start of memory segment */
        unsigned long long size;        /* size of memory segment */
        unsigned long type;             /* type of memory segment */
};

struct mem_info_t {
	unsigned long e88_mem_k;	/* 0x00 */
	unsigned long e801_mem_k;	/* 0x04 */
	unsigned long e820_nr;		/* 0x08 */
	struct e820entry e820[E820MAX];	/* 0x0c */
					/* 0x28c */
};

#define BARRIER_ADDR (0x9ff00)

#define RES_START	0xa0000
#define RES_END		0x100000

#define DMI_SEARCH_START  0x0000F000
#define DMI_SEARCH_LENGTH 0x000F0FFF
#define MAX_DMI_MEMDEVS 16

#define getCx86(reg) ({ outb((reg), 0x22); inb(0x23); })


static inline void cache_off(void)
{
        asm(
		"push %eax\n\t"
		"movl %cr0,%eax\n\t"
    "orl $0x40000000,%eax\n\t"  /* Set CD */
    "movl %eax,%cr0\n\t"
		"wbinvd\n\t"
		"pop  %eax\n\t");
}

static inline void cache_on(void)
{
        asm(
		"push %eax\n\t"
		"movl %cr0,%eax\n\t"
    "andl $0x9fffffff,%eax\n\t" /* Clear CD and NW */
    "movl %eax,%cr0\n\t"
		"pop  %eax\n\t");
}

/*
 * Checked against the Intel manual and GCC --hpreg
 *
 * volatile because the tsc always changes without the compiler knowing it.
 */
static inline uint64_t
RDTSC(void)
{
   uint64_t tim;
   __asm__ __volatile__(
      "rdtsc"
      : "=A" (tim)
   );
   return tim;
}

#define ADJUST_STACK(offs) { __asm__ __volatile__ ("subl %%eax, %%esp" : : "a" (offs) : "memory"); }

#define X86_FEATURE_PAE		(0*32+ 6) /* Physical Address Extensions */

#define MAX_MEM_SEGMENTS E820MAX

#endif

#endif
