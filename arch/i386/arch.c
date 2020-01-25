#include <sys/io.h>

#include "arch.h"
#include "globals.h"
#include "smp.h"
#include "test.h"
#include "cpuid.h"

volatile int    cpu_ord=0;

/* This is the test entry point. We get here on statup and also whenever
 * we relocate. */
void test_start(void)
{
	const char *cmdline;
	int my_cpu_num, my_cpu_ord;
	/* If this is the first time here we are CPU 0 */
	if (start_seq == 0) {
		my_cpu_num = 0;
	} else {
		my_cpu_num = smp_my_cpu_num();
	}
	/* First thing, switch to main stack */
	switch_to_main_stack(my_cpu_num);

	/* First time (for this CPU) initialization */
	if (start_seq < 2) {

	    /* These steps are only done by the boot cpu */
	    if (my_cpu_num == 0) {
		my_cpu_ord = cpu_ord++;
		smp_set_ordinal(my_cpu_num, my_cpu_ord);
		if (*OLD_CL_MAGIC_ADDR == OLD_CL_MAGIC) {
			unsigned short offset = *OLD_CL_OFFSET_ADDR;
			cmdline = MK_PTR(INITSEG, offset);
			parse_command_line(cmdline);
		}
		clear_screen();
		/* Initialize the barrier so the lock in btrace will work.
		 * Will get redone later when we know how many CPUs we have */
		barrier_init(1);
		btrace(my_cpu_num, __LINE__, "Begin     ", 1, 0, 0);
		/* Find memory size */
		mem_size();	/* must be called before initialise_cpus(); */
		/* Fill in the CPUID table */
		get_cpuid();
		/* Startup the other CPUs */
		start_seq = 1;
		//initialise_cpus();
		btrace(my_cpu_num, __LINE__, "BeforeInit", 1, 0, 0);
		/* Draw the screen and get system information */
	  init();

		/* Set defaults and initialize variables */
		set_defaults();

		/* Setup base address for testing, 1 MB */
		win0_start = 0x100;

		/* Set relocation address to 32Mb if there is enough
		 * memory. Otherwise set it to 3Mb */
		/* Large reloc addr allows for more testing overlap */
	        if ((ulong)v->pmap[v->msegs-1].end > 0x2f00) {
			high_test_adr = 0x2000000;
	        } else {
			high_test_adr = 0x300000;
		}
		win1_end = (high_test_adr >> 12);
		/* Adjust the map to not test the page at 939k,
		 *  reserved for locks */
		v->pmap[0].end--;

		find_ticks_for_pass();
       	    } else {
		/* APs only, Register the APs */
		btrace(my_cpu_num, __LINE__, "AP_Start  ", 0, my_cpu_num,
			cpu_ord);
		smp_ap_booted(my_cpu_num);
		/* Asign a sequential CPU ordinal to each active cpu */
		spin_lock(&barr->mutex);
		my_cpu_ord = cpu_ord++;
		smp_set_ordinal(my_cpu_num, my_cpu_ord);
		spin_unlock(&barr->mutex);
		btrace(my_cpu_num, __LINE__, "AP_Done   ", 0, my_cpu_num,
			my_cpu_ord);
	    }

	} else {
	    /* Unlock after a relocation */
	    spin_unlock(&barr->mutex);
	    /* Get the CPU ordinal since it is lost during relocation */
	    my_cpu_ord = smp_my_ord_num(my_cpu_num);
	    btrace(my_cpu_num, __LINE__, "Reloc_Done",0,my_cpu_num,my_cpu_ord);
	}

	/* A barrier to insure that all of the CPUs are done with startup */
	barrier();
	btrace(my_cpu_num, __LINE__, "1st Barr  ", 1, my_cpu_num, my_cpu_ord);


	/* Setup Memory Management and measure memory speed, we do it here
	 * because we need all of the available CPUs */
	if (start_seq < 2) {

		/* Enable floating point processing */
	   if (cpu_id.fid.bits.fpu)
        	__asm__ __volatile__ (
		    "movl %%cr0, %%eax\n\t"
		    "andl $0x7, %%eax\n\t"
		    "movl %%eax, %%cr0\n\t"
                    : :
                    : "ax"
                );
	   if (cpu_id.fid.bits.sse)
        	__asm__ __volatile__ (
                    "movl %%cr4, %%eax\n\t"
                    "orl $0x00000200, %%eax\n\t"
                    "movl %%eax, %%cr4\n\t"
                    : :
                    : "ax"
                );

	    btrace(my_cpu_num, __LINE__, "Mem Mgmnt ", 1, cpu_id.fid.bits.pae, cpu_id.fid.bits.lm);
	   /* Setup memory management modes */
	    /* If we have PAE, turn it on */
	    if (cpu_id.fid.bits.pae == 1) {
		__asm__ __volatile__(
                    "movl %%cr4, %%eax\n\t"
                    "orl $0x00000020, %%eax\n\t"
                    "movl %%eax, %%cr4\n\t"
                    : :
                    : "ax"
                );
        cprint(LINE_TITLE+1, COL_MODE, "(PAE Mode)");
       	    }
	    /* If this is a 64 CPU enable long mode */
	    if (cpu_id.fid.bits.lm == 1) {
		__asm__ __volatile__(
		    "movl $0xc0000080, %%ecx\n\t"
		    "rdmsr\n\t"
		    "orl $0x00000100, %%eax\n\t"
		    "wrmsr\n\t"
		    : :
		    : "ax", "cx"
		);
		cprint(LINE_TITLE+1, COL_MODE, "(X64 Mode)");
            }
	    /* Get the memory Speed with all CPUs */
	    get_mem_speed(my_cpu_num, num_cpus);
	}

	/* Set the initialized flag only after all of the CPU's have
	 * Reached the barrier. This insures that relocation has
	 * been completed for each CPU. */
	btrace(my_cpu_num, __LINE__, "Start Done", 1, 0, 0);
	start_seq = 2;
	memtest_main(my_cpu_num, my_cpu_ord);
}

char *codes[] = {
	"  Divide",
	"   Debug",
	"     NMI",
	"  Brkpnt",
	"Overflow",
	"   Bound",
	"  Inv_Op",
	" No_Math",
	"Double_Fault",
	"Seg_Over",
	" Inv_TSS",
	"  Seg_NP",
	"Stack_Fault",
	"Gen_Prot",
	"Page_Fault",
	"   Resvd",
	"     FPE",
	"Alignment",
	" Mch_Chk",
	"SIMD FPE"
};

struct eregs {
	ulong ss;
	ulong ds;
	ulong esp;
	ulong ebp;
	ulong esi;
	ulong edi;
	ulong edx;
	ulong ecx;
	ulong ebx;
	ulong eax;
	ulong vect;
	ulong code;
	ulong eip;
	ulong cs;
	ulong eflag;
};


/* Handle an interrupt */
void inter(struct eregs *trap_regs)
{
	int i, line;
	unsigned char *pp;
	ulong address = 0;
	int my_cpu_num = smp_my_cpu_num();

	/* Get the page fault address */
	if (trap_regs->vect == 14) {
		__asm__("movl %%cr2,%0":"=r" (address));
	}
#ifdef PARITY_MEM

	/* Check for a parity error */
	if (trap_regs->vect == 2) {
		parity_err(trap_regs->edi, trap_regs->esi);
		return;
	}
#endif

	/* clear scrolling region */
        pp=(unsigned char *)(SCREEN_ADR+(2*80*(LINE_SCROLL-2)));
        for(i=0; i<2*80*(24-LINE_SCROLL-2); i++, pp+=2) {
                *pp = ' ';
        }
	line = LINE_SCROLL-2;

	cprint(line, 0, "Unexpected Interrupt - Halting CPU");
	dprint(line, COL_MID + 4, my_cpu_num, 2, 1);
	cprint(line+2, 0, " Type: ");
	if (trap_regs->vect <= 19) {
		cprint(line+2, 7, codes[trap_regs->vect]);
	} else {
		hprint(line+2, 7, trap_regs->vect);
	}
	cprint(line+3, 0, "   PC: ");
	hprint(line+3, 7, trap_regs->eip);
	cprint(line+4, 0, "   CS: ");
	hprint(line+4, 7, trap_regs->cs);
	cprint(line+5, 0, "Eflag: ");
	hprint(line+5, 7, trap_regs->eflag);
	cprint(line+6, 0, " Code: ");
	hprint(line+6, 7, trap_regs->code);
	cprint(line+7, 0, "   DS: ");
	hprint(line+7, 7, trap_regs->ds);
	cprint(line+8, 0, "   SS: ");
	hprint(line+8, 7, trap_regs->ss);
	if (trap_regs->vect == 14) {
		/* Page fault address */
		cprint(line+7, 0, " Addr: ");
		hprint(line+7, 7, address);
	}

	cprint(line+2, 20, "eax: ");
	hprint(line+2, 25, trap_regs->eax);
	cprint(line+3, 20, "ebx: ");
	hprint(line+3, 25, trap_regs->ebx);
	cprint(line+4, 20, "ecx: ");
	hprint(line+4, 25, trap_regs->ecx);
	cprint(line+5, 20, "edx: ");
	hprint(line+5, 25, trap_regs->edx);
	cprint(line+6, 20, "edi: ");
	hprint(line+6, 25, trap_regs->edi);
	cprint(line+7, 20, "esi: ");
	hprint(line+7, 25, trap_regs->esi);
	cprint(line+8, 20, "ebp: ");
	hprint(line+8, 25, trap_regs->ebp);
	cprint(line+9, 20, "esp: ");
	hprint(line+9, 25, trap_regs->esp);

	cprint(line+1, 38, "Stack:");
	for (i=0; i<10; i++) {
		hprint(line+2+i, 38, trap_regs->esp+(4*i));
		hprint(line+2+i, 47, *(ulong*)(trap_regs->esp+(4*i)));
		hprint(line+2+i, 57, trap_regs->esp+(4*(i+10)));
		hprint(line+2+i, 66, *(ulong*)(trap_regs->esp+(4*(i+10))));
	}

	cprint(line+11, 0, "CS:EIP:                          ");
	pp = (unsigned char *)trap_regs->eip;
	for(i = 0; i < 9; i++) {
		hprint2(line+11, 8+(3*i), pp[i], 2);
	}
	while(1) {
		check_input();
	}
}

/* Beep function */

void beep(unsigned int frequency)
{
	unsigned int count = 1193180 / frequency;

	// Switch on the speaker
	outb_p(inb_p(0x61)|3, 0x61);

	// Set command for counter 2, 2 byte write
	outb_p(0xB6, 0x43);

	// Select desired Hz
	outb_p(count & 0xff, 0x42);
	outb((count >> 8) & 0xff, 0x42);

	// Block for 100 microseconds
	sleep(100, 0, 0, 1);

	// Switch off the speaker
	outb(inb_p(0x61)&0xFC, 0x61);
}
