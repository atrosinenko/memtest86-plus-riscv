/*
 * Common code shared between real bare-metal MemTest implementation
 * and dummy test-runner.c that can be used for debugging test
 * implementations (including optimized versions) in user space.
 */

#include <stddef.h>

#include "arch.h"
#include "smp.h"
#include "test.h"

bool rdtsc_is_available;

volatile ulong win0_start;	/* Start test address for window 0 */
volatile ulong win1_end;	/* End address for relocation */


struct tseq tseq[] = {
	{1, -1,  0,   6, 0, "[Address test, walking ones, no cache] "},
	{1, -1,  1,   6, 0, "[Address test, own address Sequential] "},
	{1, 32,  2,   6, 0, "[Address test, own address Parallel]   "},
	{1, 32,  3,   6, 0, "[Moving inversions, 1s & 0s Parallel]  "},
	{1, 32,  5,   3, 0, "[Moving inversions, 8 bit pattern]     "},
	{1, 32,  6,  30, 0, "[Moving inversions, random pattern]    "},
	{1, 32,  7,  81, 0, "[Block move]                           "},
	{1,  1,  8,   3, 0, "[Moving inversions, 32 bit pattern]    "},
	{1, 32,  9,  48, 0, "[Random number sequence]               "},
  {1, 32, 10,   6, 0, "[Modulo 20, Random pattern]            "},
	{1, 1,  11, 240, 0, "[Bit fade test, 2 patterns]            "},
	{1, 0,   0,   0, 0, NULL}
};

volatile int    mstr_cpu;
volatile int	run_cpus;
int		maxcpus=MAX_CPUS;
volatile short  cpu_sel;
volatile short	cpu_mode;
char		cpu_mask[MAX_CPUS];
long 		bin_mask=0xffffffff;
short		onepass;
volatile short	btflag = 0;
volatile int	test;
short	        restart_flag;
int 		bitf_seq = 0;
char		cmdline_parsed = 0;
struct 		vars variables = {};
struct 		vars * const v = &variables;
volatile int 	bail;
volatile int 	segs;
int	ltest;
int	pass_flag = 0;
volatile short	start_seq = 0;
int	c_iter;
volatile int window;
volatile unsigned long win_next;

/*
volatile int    mstr_cpu;
volatile int	run_cpus;
int		maxcpus=MAX_CPUS;
char		cpu_mask[MAX_CPUS];
volatile int 	segs;
int		pass_flag = 0;
short	        restart_flag;
int 		bitf_seq = 0;
char		cmdline_parsed = 0;
volatile short	btflag = 0;
long 		bin_mask=0xffffffff;
volatile short	start_seq = 0;
volatile short  cpu_sel;
volatile short	cpu_mode;
int		c_iter;
volatile int	test;
short		onepass;
volatile int 	bail;
int	ltest;
volatile int window;
volatile unsigned long win_next;
struct 		vars variables = {};
struct 		vars * const v = &variables;

struct tseq tseq[] = {
	{1, -1,  0,   6, 0, "[Address test, walking ones, no cache] "},
	{1, -1,  1,   6, 0, "[Address test, own address Sequential] "},
	{1, 32,  2,   6, 0, "[Address test, own address Parallel]   "},
	{1, 32,  3,   6, 0, "[Moving inversions, 1s & 0s Parallel]  "},
	{1, 32,  5,   3, 0, "[Moving inversions, 8 bit pattern]     "},
	{1, 32,  6,  30, 0, "[Moving inversions, random pattern]    "},
	{1, 32,  7,  81, 0, "[Block move]                           "},
	{1,  1,  8,   3, 0, "[Moving inversions, 32 bit pattern]    "},
	{1, 32,  9,  48, 0, "[Random number sequence]               "},
	{1, 32, 10,   6, 0, "[Modulo 20, Random pattern]            "},
	{1, 1,  11, 240, 0, "[Bit fade test, 2 patterns]            "},
	{1, 0,   0,   0, 0, NULL}
};
*/
void parse_command_line(const char *cp)
{
	int i, j, k;

	if (cmdline_parsed)
		return;

	/* Fill in the cpu mask array with the default */
	for (i=0; i<MAX_CPUS; i++) {
		cpu_mask[i] = 1;
	}

	/* skip leading spaces */
	while (*cp == ' ')
		cp++;

	while (*cp) {
		if (!strncmp(cp, "console=", 8)) {
			cp += 8;
			serial_console_setup(cp);
		}
		/* Enable boot trace? */
		if (!strncmp(cp, "btrace", 6)) {
			cp += 6;
			btflag++;
		}
		/* Limit number of CPUs */
		if (!strncmp(cp, "maxcpus=", 8)) {
			cp += 8;
			maxcpus=(int)simple_strtoul(cp, NULL, 10);
		}
		/* Run one pass and exit if there are no errors */
		if (!strncmp(cp, "onepass", 7)) {
			cp += 7;
			onepass++;
		}
		/* Setup a list of tests to run */
		if (!strncmp(cp, "tstlist=", 8)) {
			cp += 8;
			/* Clear all of the tests first */
			k = 0;
			while (tseq[k].cpu_sel) {
				tseq[k].sel = 0;
				k++;
			}

			/* Now enable all of the tests in the list */
			j = 0;
			while(*cp && isdigit(*cp)) {
			    i = *cp-'0';
			    j = j*10 + i;
			    cp++;
			    if (*cp == ',' || !isdigit(*cp)) {
				if (j < k) {
				    tseq[j].sel = 1;
				}
				if (*cp != ',') break;
				j = 0;
			    	cp++;
			    }
			}
		}
		/* Set a CPU mask to select CPU's to use for testing */
		if (!strncmp(cp, "cpumask=", 8)) {
		    cp += 8;
		    if (cp[0] == '0' && toupper(cp[1]) == 'X') cp += 2;
		    while (*cp && *cp != ' ' && isxdigit(*cp)) {
			i = isdigit(*cp) ? *cp-'0' : toupper(*cp)-'A'+10;
			bin_mask = bin_mask * 16 + i;
			cp++;
		    }
		    /* Force CPU zero to always be selected */
		    bin_mask |= 1;
		    for (i=0; i<32; i++) {
			if (((bin_mask>>i) & 1) == 0) {
			     cpu_mask[i] = 0;
			}
		    }
		}
		/* go to the next parameter */
		while (*cp && *cp != ' ') cp++;
		while (*cp == ' ') cp++;
	}

	cmdline_parsed = 1;
}

/* Find the next selected test to run */
void next_test(void)
{
	test++;
	while (tseq[test].sel == 0 && tseq[test].cpu_sel != 0) {
	    test++;
	}

	if (tseq[test].cpu_sel == 0) {
	    /* We hit the end of the list so we completed a pass */
	    pass_flag++;
	    /* Find the next test to run, start searching from 0 */
	    test = 0;
	    while (tseq[test].sel == 0 && tseq[test].cpu_sel != 0) {
		test++;
	    }
	}
}

/* Set default values for all parameters */
void set_defaults(void)
{
	int i;

	if (start_seq == 2) {
		/* This is a restart so we reset everything */
		onepass = 0;
		i = 0;
		while (tseq[i].cpu_sel) {
			tseq[i].sel = 1;
			i++;
		}
		test = 0;
		if (tseq[0].sel == 0) {
			next_test();
		}
	}
	ltest = -1;
	win_next = 0;
	window = 0;
	bail = 0;
	cpu_mode = CPM_ALL;
	cpu_sel = 0;
	v->printmode=PRINTMODE_ADDRESSES;
	v->numpatn=0;
	v->plim_lower = v->pmap[0].start;
	v->plim_upper = v->pmap[v->msegs-1].end;
	v->pass = 0;
	v->msg_line = 0;
	v->ecount = 0;
	v->ecc_ecount = 0;
	v->msg_line = LINE_SCROLL-1;
	v->scroll_start = v->msg_line * 160;
	v->erri.low_addr.page = 0x7fffffff;
	v->erri.low_addr.offset = 0xfff;
	v->erri.high_addr.page = 0;
	v->erri.high_addr.offset = 0;
	v->erri.min_bits = 32;
	v->erri.max_bits = 0;
	v->erri.min_bits = 32;
	v->erri.max_bits = 0;
	v->erri.maxl = 0;
	v->erri.cor_err = 0;
	v->erri.ebits = 0;
	v->erri.hdr_flag = 0;
	v->erri.tbits = 0;
	for (i=0; tseq[i].msg != NULL; i++) {
		tseq[i].errors = 0;
	}
	restart_flag = 0;
	tseq[10].sel = 0;
}

/* A couple static variables for when all cpus share the same pattern */
static ulong sp1, sp2;

int invoke_test(int my_ord)
{
	static int bitf_sleep;
	int i, j;
	ulong p0, p1, p2 = 0;

	p1 = page_of(v->map[segs-1].end);

	switch(tseq[test].pat) {

	/* Do the testing according to the selected pattern */

	case 0: /* Address test, walking ones (test #0) */
		/* Run with cache turned off */
		set_cache(0);
		addr_tst1(my_ord);
		set_cache(1);
		BAILOUT;
		break;

	case 1:
	case 2: /* Address test, own address (test #1, 2) */
		addr_tst2(my_ord);
		BAILOUT;
		break;

	case 3:
	case 4:	/* Moving inversions, all ones and zeros (tests #3, 4) */
		p1 = 0;
		p2 = ~p1;
		s_barrier();
		movinv1(c_iter,p1,p2,my_ord);
		BAILOUT;

		/* Switch patterns */
		s_barrier();
		movinv1(c_iter,p2,p1,my_ord);
		BAILOUT;
		break;

	case 5: /* Moving inversions, 8 bit walking ones and zeros (test #5) */
		p0 = 0x80;
		for (i=0; i<8; i++, p0=p0>>1) {
			p1 = p0 | (p0<<8) | (p0<<16) | (p0<<24);
			p2 = ~p1;
			s_barrier();
			movinv1(c_iter,p1,p2, my_ord);
			BAILOUT;

			/* Switch patterns */
			s_barrier();
			movinv1(c_iter,p2,p1, my_ord);
			BAILOUT
		}
		break;

	case 6: /* Random Data (test #6) */
		/* Seed the random number generator */
		if (my_ord == mstr_cpu) {
		    if (RDTSC_AVAILABLE()) {
			RDTSC_LH(sp1, sp2);
        	    } else {
                	sp1 = 521288629 + v->pass;
                	sp2 = 362436069 - v->pass;
        	    }
		    rand_seed(sp1, sp2, 0);
		}

		s_barrier();
		for (i=0; i < c_iter; i++) {
			if (my_ord == mstr_cpu) {
				sp1 = memtest_rand(0);
				sp2 = ~p1;
			}
			s_barrier();
			movinv1(2,sp1,sp2, my_ord);
			BAILOUT;
		}
		break;


	case 7: /* Block move (test #7) */
		block_move(c_iter, my_ord);
		BAILOUT;
		break;

	case 8: /* Moving inversions, 32 bit shifting pattern (test #8) */
		for (i=0, p1=1; p1; p1=p1<<1, i++) {
			s_barrier();
			movinv32(c_iter,p1, 1, 0x80000000, 0, i, my_ord);
			BAILOUT
			s_barrier();
			movinv32(c_iter,~p1, 0xfffffffe,
				0x7fffffff, 1, i, my_ord);
			BAILOUT
		}
		break;

	case 9: /* Random Data Sequence (test #9) */
		for (i=0; i < c_iter; i++) {
			s_barrier();
			movinvr(my_ord);
			BAILOUT;
		}
		break;

	case 10: /* Modulo 20 check, Random pattern (test #10) */
		for (j=0; j<c_iter; j++) {
			p1 = memtest_rand(0);
			for (i=0; i<MOD_SZ; i++) {
				p2 = ~p1;
				s_barrier();
				modtst(i, 2, p1, p2, my_ord);
				BAILOUT

				/* Switch patterns */
				s_barrier();
				modtst(i, 2, p2, p1, my_ord);
				BAILOUT
			}
		}
		break;

	case 11: /* Bit fade test, fill (test #11) */
		/* Use a sequence to process all windows for each stage */
		switch(bitf_seq) {
		case 0:	/* Fill all of memory 0's */
			bit_fade_fill(0, my_ord);
			bitf_sleep = 1;
			break;
		case 1: /* Sleep for the specified time */
			/* Only sleep once */
			if (bitf_sleep) {
				sleep(c_iter, 1, my_ord, 0);
				bitf_sleep = 0;
			}
			break;
		case 2: /* Now check all of memory for changes */
			bit_fade_chk(0, my_ord);
			break;
		case 3:	/* Fill all of memory 1's */
			bit_fade_fill(-1, my_ord);
			bitf_sleep = 1;
			break;
		case 4: /* Sleep for the specified time */
			/* Only sleep once */
			if (bitf_sleep) {
				sleep(c_iter, 1, my_ord, 0);
				bitf_sleep = 0;
			}
			break;
		case 5: /* Now check all of memory for changes */
			bit_fade_chk(-1, my_ord);
			break;
		}
		BAILOUT;
		break;

	case 90: /* Modulo 20 check, all ones and zeros (unused) */
		p1=0;
		for (i=0; i<MOD_SZ; i++) {
			p2 = ~p1;
			modtst(i, c_iter, p1, p2, my_ord);
			BAILOUT

			/* Switch patterns */
			p2 = p1;
			p1 = ~p2;
			modtst(i, c_iter, p1,p2, my_ord);
			BAILOUT
		}
		break;

	case 91: /* Modulo 20 check, 8 bit pattern (unused) */
		p0 = 0x80;
		for (j=0; j<8; j++, p0=p0>>1) {
			p1 = p0 | (p0<<8) | (p0<<16) | (p0<<24);
			for (i=0; i<MOD_SZ; i++) {
				p2 = ~p1;
				modtst(i, c_iter, p1, p2, my_ord);
				BAILOUT

				/* Switch patterns */
				p2 = p1;
				p1 = ~p2;
				modtst(i, c_iter, p1, p2, my_ord);
				BAILOUT
			}
		}
		break;
	}
	return(0);
}
