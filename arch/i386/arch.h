#ifndef ARCH_H
#define ARCH_H

#include <stdint.h>

#include "defs.h"
#include "config.h"
#include "globals.h"

extern struct	cpu_ident cpu_id;

#define RDTSC_LH(low, high) asm __volatile__ ("rdtsc":"=a" (low),"=d" (high))

#define __ELF_NATIVE_CLASS 32
#define ELF_MACHINE_NO_RELA 1

#define SCREEN_ADR  0xb8000
#define SCREEN_END_ADR  (SCREEN_ADR + 80*25*2)

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

extern struct mem_info_t mem_info;

/* command line passing using the 'old' boot protocol */
#define MK_PTR(seg,off) ((void*)(((unsigned long)(seg) << 4) + (off)))
#define OLD_CL_MAGIC_ADDR ((unsigned short*) MK_PTR(INITSEG,0x20))
#define OLD_CL_MAGIC 0xA33F
#define OLD_CL_OFFSET_ADDR ((unsigned short*) MK_PTR(INITSEG,0x22))

#define ADJUST_STACK(offs) { __asm__ __volatile__ ("subl %%eax, %%esp" : : "a" (offs) : "memory"); }

#endif
