/* test.c - MemTest-86  Version 3.4
 *
 * Released under version 2 of the Gnu Public License.
 * By Chris Brady
 * ----------------------------------------------------
 * MemTest86+ V5 Specific code (GPL V2.0)
 * By Samuel DEMEULEMEESTER, sdemeule@memtest.org
 * http://www.canardpc.com - http://www.memtest.org
 * Thanks to Passmark for calculate_chunk() and various comments !
 */

#include "arch.h"
#include "test.h"
#include "config.h"
#include "stdint.h"
#include "smp.h"
#include "io.h"

#define OPTIMIZED_SNIPPET static inline __attribute__((always_inline))

// Set to 0 to disable all optimizations
#ifndef OPTIMIZED
#   define OPTIMIZED 1
#endif

// Selectively disable snippets to check enabled snippets against plain C
#ifndef OPTIMIZED_FIRST_SNIPPET
#   define OPTIMIZED_FIRST_SNIPPET 1
#endif
#ifndef OPTIMIZED_SECOND_SNIPPET
#   define OPTIMIZED_SECOND_SNIPPET 1
#endif
#ifndef OPTIMIZED_THIRD_SNIPPET
#   define OPTIMIZED_THIRD_SNIPPET 1
#endif

OPTIMIZED_SNIPPET void addr_tst2_snippet1(uint32_t *p, uint32_t *pe);
OPTIMIZED_SNIPPET void addr_tst2_snippet2(uint32_t *p, uint32_t *pe);
OPTIMIZED_SNIPPET void movinvr_snippet1(uint32_t *p, uint32_t *pe, unsigned me);
OPTIMIZED_SNIPPET void movinvr_snippet2(uint32_t *p, uint32_t *pe, uint32_t xorVal, unsigned me);
OPTIMIZED_SNIPPET void movinv1_snippet1(uint32_t *p, uint32_t *pe, uint32_t p1);
OPTIMIZED_SNIPPET void movinv1_snippet2(uint32_t *p, uint32_t *pe, uint32_t p1, uint32_t p2);
OPTIMIZED_SNIPPET void movinv1_snippet3(uint32_t *p, uint32_t *pe, uint32_t p1, uint32_t p2);
OPTIMIZED_SNIPPET void movinv32_snippet1(
	int *p_k, uint32_t *p_pat, // inputs-outputs
	uint32_t *p, uint32_t *pe, uint32_t sval, uint32_t lb // inputs only
);
OPTIMIZED_SNIPPET void movinv32_snippet2(
	int *p_k, uint32_t *p_pat, // inputs-outputs
	uint32_t *p, uint32_t *pe, uint32_t sval, uint32_t lb // inputs only
);
OPTIMIZED_SNIPPET void movinv32_snippet3(
	int *p_k, uint32_t *p_pat, // inputs-outputs
	uint32_t *p, uint32_t *pe, uint32_t p3, uint32_t hb // inputs only
);
OPTIMIZED_SNIPPET void modtst_snippet1(uint32_t **p_p, uint32_t *pe, uint32_t p1);
OPTIMIZED_SNIPPET void modtst_snippet2(int *p_k, uint32_t *p, uint32_t *pe, uint32_t p2, int offset);
OPTIMIZED_SNIPPET void modtst_snippet3(uint32_t **p_p,	uint32_t *pe, uint32_t p1);
OPTIMIZED_SNIPPET void block_move_snippet1(uint32_t **p_p, ulong len);
OPTIMIZED_SNIPPET void block_move_snippet2(uint32_t *p, ulong pp, ulong len);
OPTIMIZED_SNIPPET void block_move_snippet3(uint32_t **p_p, uint32_t *pe);

#include "test.inc.c"

extern volatile int    mstr_cpu;
extern volatile int    run_cpus;
extern volatile int    test;
extern volatile int segs, bail;
extern int test_ticks, nticks;
extern struct tseq tseq[];
extern void update_err_counts(void);
extern void print_err_counts(void);
void rand_seed( unsigned int seed1, unsigned int seed2, int me);
void poll_errors();

static inline ulong roundup(ulong value, ulong mask)
{
	return (value + mask) & ~mask;
}

// start / end - return values for range to test
// me - this threads CPU number
// j - index into v->map for current segment we are testing
// align - number of bytes to align each block to
void calculate_chunk(uint32_t** start, uint32_t** end, int me, int j, int makeMultipleOf)
{
	ulong chunk;


	// If we are only running 1 CPU then test the whole block
	if (run_cpus == 1) {
		*start = v->map[j].start;
		*end = v->map[j].end;
	} 
	else{

		// Divide the current segment by the number of CPUs
		chunk = (ulong)v->map[j].end-(ulong)v->map[j].start;
		chunk /= run_cpus;
		
		// Round down to the nearest desired bitlength multiple
		chunk = (chunk + (makeMultipleOf-1)) &  ~(makeMultipleOf-1);

		// Figure out chunk boundaries
		*start = (uint32_t*)((uintptr_t)v->map[j].start+(chunk*me));
		/* Set end addrs for the highest CPU num to the
			* end of the segment for rounding errors */
		// Also rounds down to boundary if needed, may miss some ram but better than crashing or producing false errors.
		// This rounding probably will never happen as the segments should be in 4096 bytes pages if I understand correctly.
		if (me == mstr_cpu) {
			*end = (uint32_t*)(v->map[j].end);
		} else {
			*end = (uint32_t*)((uintptr_t)(*start) + chunk);
			(*end)--;
		}
	}
}

/*
 * Memory address test, walking ones
 */
void addr_tst1(int me)
{
	int i, j, k;
	volatile uint32_t *p, *pt, *end;
	uint32_t bad, mask, p1;
	ulong bank;

	/* Test the global address bits */
	for (p1=0, j=0; j<2; j++) {
        	hprint(LINE_PAT, COL_PAT, p1);

		/* Set pattern in our lowest multiple of 0x20000 */
		p = (uint32_t *)roundup((ulong)v->map[0].start, 0x1ffff);
		*p = p1;
	
		/* Now write pattern compliment */
		p1 = ~p1;
		end = v->map[segs-1].end;
		for (i=0; i<100; i++) {
			mask = 4;
			do {
				pt = (uint32_t *)((ulong)p | mask);
				if (pt == p) {
					mask = mask << 1;
					continue;
				}
				if (pt >= end) {
					break;
				}
				*pt = p1;
				if ((bad = *p) != ~p1) {
					ad_err1((uint32_t *)p, mask,
						bad, ~p1);
					i = 1000;
				}
				mask = mask << 1;
			} while(mask);
		}
		do_tick(me);
		BAILR
	}

	/* Now check the address bits in each bank */
	/* If we have more than 8mb of memory then the bank size must be */
	/* bigger than 256k.  If so use 1mb for the bank size. */
	if (v->pmap[v->msegs - 1].end > (0x800000 >> 12)) {
		bank = 0x100000;
	} else {
		bank = 0x40000;
	}
	for (p1=0, k=0; k<2; k++) {
        	hprint(LINE_PAT, COL_PAT, p1);

		for (j=0; j<segs; j++) {
			p = v->map[j].start;
			/* Force start address to be a multiple of 256k */
			p = (uint32_t *)roundup((ulong)p, bank - 1);
			end = v->map[j].end;
			/* Redundant checks for overflow */
                        while (p < end && p > v->map[j].start && p != 0) {
				*p = p1;

				p1 = ~p1;
				for (i=0; i<50; i++) {
					mask = 4;
					do {
						pt = (uint32_t *)
						    ((ulong)p | mask);
						if (pt == p) {
							mask = mask << 1;
							continue;
						}
						if (pt >= end) {
							break;
						}
						*pt = p1;
						if ((bad = *p) != ~p1) {
							ad_err1((uint32_t *)p,
							    mask,bad,~p1);
							i = 200;
						}
						mask = mask << 1;
					} while(mask);
				}
				if (p + bank > p) {
					p += bank;
				} else {
					p = end;
				}
				p1 = ~p1;
			}
		}
		do_tick(me);
		BAILR
		p1 = ~p1;
	}
}

/*
 * Memory address test, own address
 */
void addr_tst2(int me)
{
	int j, done;
	uint32_t *p, *pe, *end, *start, bad;

        cprint(LINE_PAT, COL_PAT, "address ");

	/* Write each address with it's own address */
	for (j=0; j<segs; j++) {
		start = v->map[j].start;
		end = v->map[j].end;
		pe = (uint32_t *)start;
		p = start;
		done = 0;
		do {
			do_tick(me);
			BAILR

			/* Check for overflow */
			if (pe + SPINSZ > pe && pe != 0) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}

#if OPTIMIZED && HAS_OPT_ADDR_TST2 && OPTIMIZED_FIRST_SNIPPET
			addr_tst2_snippet1(p, pe);
#else
			for (; p <= pe; p++) {
				*p = (ulong)p;
			}
#endif
			p = pe + 1;
		} while (!done);
	}

	/* Each address should have its own address */
	for (j=0; j<segs; j++) {
		start = v->map[j].start;
		end = v->map[j].end;
		pe = (uint32_t *)start;
		p = start;
		done = 0;
		do {
			do_tick(me);
			BAILR

			/* Check for overflow */
			if (pe + SPINSZ > pe && pe != 0) {
                                pe += SPINSZ;
                        } else {
                                pe = end;
                        }
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
#if OPTIMIZED && HAS_OPT_ADDR_TST2 && OPTIMIZED_SECOND_SNIPPET
			addr_tst2_snippet2(p, pe);
#else
			for (; p <= pe; p++) {
				if((bad = *p) != (uint32_t)p) {
					ad_err2((uint32_t)p, bad);
				}
			}
#endif
			p = pe + 1;
		} while (!done);
	}
}

/*
 * Test all of memory using a "half moving inversions" algorithm using random
 * numbers and their complment as the data pattern. Since we are not able to
 * produce random numbers in reverse order testing is only done in the forward
 * direction.
 */
void movinvr(int me)
{
	int i, j, done, seed1, seed2;
	uint32_t *p;
	uint32_t *pe;
	uint32_t *start,*end;
	uint32_t bad, num;

	/* Initialize memory with initial sequence of random numbers.  */
	if (RDTSC_AVAILABLE()) {
		RDTSC_LH(seed1, seed2);
	} else {
		seed1 = 521288629 + v->pass;
		seed2 = 362436069 - v->pass;
	}

	/* Display the current seed */
        if (mstr_cpu == me) hprint(LINE_PAT, COL_PAT, seed1);
	rand_seed(seed1, seed2, me);
	for (j=0; j<segs; j++) {
		calculate_chunk(&start, &end, me, j, 4);
		pe = start;
		p = start;
		done = 0;
		do {
			do_tick(me);
			BAILR

			/* Check for overflow */
			if (pe + SPINSZ > pe && pe != 0) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
#if OPTIMIZED && HAS_OPT_MOVINVR && OPTIMIZED_FIRST_SNIPPET
			movinvr_snippet1(p, pe, me);
#else
			for (; p <= pe; p++) {
				*p = memtest_rand(me);
			}
#endif
			p = pe + 1;
		} while (!done);
	}

	/* Do moving inversions test. Check for initial pattern and then
	 * write the complement for each memory location.
	 */
	for (i=0; i<2; i++) {
		uint32_t xorVal = i ? 0xffffffffu : 0;
		rand_seed(seed1, seed2, me);
		for (j=0; j<segs; j++) {
			calculate_chunk(&start, &end, me, j, 4);
			pe = start;
			p = start;
			done = 0;
			do {
				do_tick(me);
				BAILR

				/* Check for overflow */
				if (pe + SPINSZ > pe && pe != 0) {
					pe += SPINSZ;
				} else {
					pe = end;
				}
				if (pe >= end) {
					pe = end;
					done++;
				}
				if (p == pe ) {
					break;
				}
#if OPTIMIZED && HAS_OPT_MOVINVR && OPTIMIZED_SECOND_SNIPPET
				movinvr_snippet2(p, pe, xorVal, me);
#else
				for (; p <= pe; p++) {
					num = memtest_rand(me);
					if (i) {
						num = ~num;
					}
					if ((bad=*p) != num) {
						error((uint32_t*)p, num, bad);
					}
					*p = ~num;
				}
#endif
				p = pe + 1;
			} while (!done);
		}
	}
}

/*
 * Test all of memory using a "moving inversions" algorithm using the
 * pattern in p1 and it's complement in p2.
 */
void movinv1 (int iter, uint32_t p1, uint32_t p2, int me)
{
	int i, j, done;
	uint32_t *p, *pe, *start, *end, bad;
	ulong len;

	/* Display the current pattern */
        if (mstr_cpu == me) hprint(LINE_PAT, COL_PAT, p1);

	/* Initialize memory with the initial pattern.  */
	for (j=0; j<segs; j++) {
		calculate_chunk(&start, &end, me, j, 4);


		pe = start;
		p = start;
		done = 0;
		do {
			do_tick(me);
			BAILR

			/* Check for overflow */
			if (pe + SPINSZ > pe && pe != 0) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			len = pe - p + 1;
			if (p == pe ) {
				break;
			}

#if OPTIMIZED && HAS_OPT_MOVINV1 && OPTIMIZED_FIRST_SNIPPET
			movinv1_snippet1(p, pe, p1);
#else
			for (; p <= pe; p++) {
				*p = p1;
			}
#endif
			p = pe + 1;
		} while (!done);
	}

	/* Do moving inversions test. Check for initial pattern and then
	 * write the complement for each memory location. Test from bottom
	 * up and then from the top down.  */
	for (i=0; i<iter; i++) {
		for (j=0; j<segs; j++) {
			calculate_chunk(&start, &end, me, j, 4);
			pe = start;
			p = start;
			done = 0;
			do {
				do_tick(me);
				BAILR

				/* Check for overflow */
				if (pe + SPINSZ > pe && pe != 0) {
					pe += SPINSZ;
				} else {
					pe = end;
				}
				if (pe >= end) {
					pe = end;
					done++;
				}
				if (p == pe ) {
					break;
				}

#if OPTIMIZED && HAS_OPT_MOVINV1 && OPTIMIZED_SECOND_SNIPPET
				movinv1_snippet2(p, pe, p1, p2);
#else
				for (; p <= pe; p++) {
					if ((bad = *p) != p1) {
 						error((uint32_t*)p, p1, bad);
 					}
 					*p = p2;
				}
#endif
				p = pe + 1;
			} while (!done);
		}
		for (j=segs-1; j>=0; j--) {
		    calculate_chunk(&start, &end, me, j, 4);
			pe = end;
			p = end;
			done = 0;
			do {
				do_tick(me);
				BAILR

				/* Check for underflow */
				if (pe - SPINSZ < pe && pe != 0) {
					pe -= SPINSZ;
				} else {
					pe = start;
					done++;
				}

				/* Since we are using unsigned addresses a 
				 * redundent check is required */
				if (pe < start || pe > end) {
					pe = start;
					done++;
				}
				if (p == pe ) {
					break;
				}

#if OPTIMIZED && HAS_OPT_MOVINV1 && OPTIMIZED_THIRD_SNIPPET
				movinv1_snippet3(p, pe, p1, p2);
#else
				do {
					if ((bad = *p) != p2) {
						error((uint32_t*)p, p2, bad);
					}
					*p = p1;
				} while (--p >= pe);
#endif
				p = pe - 1;
			} while (!done);
		}
	}
}

void movinv32(int iter, uint32_t p1, uint32_t lb, uint32_t hb, uint32_t sval, int off,int me)
{
	int i, j, k=0, n=0, done;
	uint32_t *p, *pe, *start, *end, pat = 0, p3, bad;

	p3 = sval << 31;
	/* Display the current pattern */
	if (mstr_cpu == me) hprint(LINE_PAT, COL_PAT, p1);

	/* Initialize memory with the initial pattern.  */
	for (j=0; j<segs; j++) {
		calculate_chunk(&start, &end, me, j, 64);
		pe = start;
		p = start;
		done = 0;
		k = off;
		pat = p1;
		do {
			do_tick(me);
			BAILR

			/* Check for overflow */
			if (pe + SPINSZ > pe && pe != 0) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
			/* Do a SPINSZ section of memory */
#if OPTIMIZED && HAS_OPT_MOVINV32 && OPTIMIZED_FIRST_SNIPPET
			movinv32_snippet1(&k, &pat, p, pe, sval, lb);
#else
			for ( ; p <= pe; ++p) {
				*p = pat;
				if (++k >= 32) {
					pat = lb;
					k = 0;
				} else {
					pat = pat << 1;
					pat |= sval;
				}
			}
#endif
			p = pe + 1;
		} while (!done);
	}

	/* Do moving inversions test. Check for initial pattern and then
	 * write the complement for each memory location. Test from bottom
	 * up and then from the top down.  */
	for (i=0; i<iter; i++) {
		for (j=0; j<segs; j++) {
			calculate_chunk(&start, &end, me, j, 64);
			pe = start;
			p = start;
			done = 0;
			k = off;
			pat = p1;
			do {
				do_tick(me);
				BAILR

				/* Check for overflow */
				if (pe + SPINSZ > pe && pe != 0) {
					pe += SPINSZ;
				} else {
					pe = end;
				}
				if (pe >= end) {
					pe = end;
					done++;
				}
				if (p == pe ) {
					break;
				}
#if OPTIMIZED && HAS_OPT_MOVINV32 && OPTIMIZED_SECOND_SNIPPET
				movinv32_snippet2(&k, &pat, p, pe, sval, lb);
#else
				for ( ; p <= pe; ++p) {
					if ((bad=*p) != pat) {
						error((uint32_t*)p, pat, bad);
					}
					*p = ~pat;

					if (++k >= 32) {
						pat = lb;
						k = 0;
					} else {
						pat = pat << 1;
						pat |= sval;
					}
				}
#endif
				p = pe + 1;
			} while (!done);
		}

                if (--k < 0) {
                        k = 31;
                }
                for (pat = lb, n = 0; n < k; n++) {
                        pat = pat << 1;
                        pat |= sval;
                }
		k++;

		for (j=segs-1; j>=0; j--) {
			calculate_chunk(&start, &end, me, j, 64);
			p = end;
			pe = end;
			done = 0;
			do {
				do_tick(me);
				BAILR

				/* Check for underflow */
                                if (pe - SPINSZ < pe && pe != 0) {
                                        pe -= SPINSZ;
                                } else {
                                        pe = start;
					done++;
                                }
				/* We need this redundant check because we are
				 * using unsigned longs for the address.
				 */
				if (pe < start || pe > end) {
					pe = start;
					done++;
				}
				if (p == pe ) {
					break;
				}
#if OPTIMIZED && HAS_OPT_MOVINV32 && OPTIMIZED_THIRD_SNIPPET
				movinv32_snippet3(&k, &pat, p, pe, p3, hb);
#else
				for ( ; p >= pe; --p) {
					if ((bad=*p) != ~pat) {
						error((uint32_t*)p, ~pat, bad);
					}
					*p = pat;
					if (--k <= 0) {
						pat = hb;
						k = 32;
					} else {
						pat = pat >> 1;
						pat |= p3;
					}
					if (p == 0) break; // TODO isn't this UB?
				};
#endif
				p = pe - 1;
			} while (!done);
		}
	}
}

/*
 * Test all of memory using modulo X access pattern.
 */
void modtst(int offset, int iter, uint32_t p1, uint32_t p2, int me)
{
	int j, k, l, done;
	uint32_t *p;
	uint32_t *pe;
	uint32_t *start, *end;
	uint32_t bad;

	/* Display the current pattern */
        if (mstr_cpu == me) {
		hprint(LINE_PAT, COL_PAT-2, p1);
		cprint(LINE_PAT, COL_PAT+6, "-");
       		dprint(LINE_PAT, COL_PAT+7, offset, 2, 1);
	}

	/* Write every nth location with pattern */
	for (j=0; j<segs; j++) {
		calculate_chunk(&start, &end, me, j, 4);
		end -= MOD_SZ;	/* adjust the ending address */
		pe = (uint32_t *)start;
		p = start+offset;
		done = 0;
		do {
			do_tick(me);
			BAILR

			/* Check for overflow */
			if (pe + SPINSZ > pe && pe != 0) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
#if OPTIMIZED && HAS_OPT_MODTST && OPTIMIZED_FIRST_SNIPPET
			modtst_snippet1(&p, pe, p1);
#else
			for (; p <= pe; p += MOD_SZ) {
				*p = p1;
			}
#endif
		} while (!done);
	}

	/* Write the rest of memory "iter" times with the pattern complement */
	for (l=0; l<iter; l++) {
		for (j=0; j<segs; j++) {
			calculate_chunk(&start, &end, me, j, 4);
			pe = (uint32_t *)start;
			p = start;
			done = 0;
			k = 0;
			do {
				do_tick(me);
				BAILR

				/* Check for overflow */
				if (pe + SPINSZ > pe && pe != 0) {
					pe += SPINSZ;
				} else {
					pe = end;
				}
				if (pe >= end) {
					pe = end;
					done++;
				}
				if (p == pe ) {
					break;
				}
#if OPTIMIZED && HAS_OPT_MODTST && OPTIMIZED_SECOND_SNIPPET
				modtst_snippet2(&k, p, pe, p2, offset);
#else
				for (; p <= pe; p++) {
					if (k != offset) {
						*p = p2;
					}
					if (++k > MOD_SZ-1) {
						k = 0;
					}
				}
#endif
				p = pe + 1;
			} while (!done);
		}
	}

	/* Now check every nth location */
	for (j=0; j<segs; j++) {
		calculate_chunk(&start, &end, me, j, 4);
		pe = (uint32_t *)start;
		p = start+offset;
		done = 0;
		end -= MOD_SZ;	/* adjust the ending address */
		do {
			do_tick(me);
			BAILR

			/* Check for overflow */
			if (pe + SPINSZ > pe && pe != 0) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
#if OPTIMIZED && HAS_OPT_MODTST && OPTIMIZED_THIRD_SNIPPET
			modtst_snippet3(&p, pe, p1);
#else
			for (; p < pe; p += MOD_SZ) {
				if ((bad=*p) != p1) {
					error((uint32_t*)p, p1, bad);
				}
			}
#endif
		} while (!done);
	}
}

#if !(OPTIMIZED_FIRST_SNIPPET && OPTIMIZED_SECOND_SNIPPET && OPTIMIZED_THIRD_SNIPPET)
// no unoptimized versions
#undef HAS_OPT_BLOCK_MOVE
#define HAS_OPT_BLOCK_MOVE 0
#endif

/*
 * Test memory using block moves 
 * Adapted from Robert Redelmeier's burnBX test
 */
void block_move(int iter, int me)
{
	int i, j, done;
	ulong len, pp;
	uint32_t *p, *pe;
	uint32_t *start, *end;

        cprint(LINE_PAT, COL_PAT-2, "          ");

	/* Initialize memory with the initial pattern.  */
	for (j=0; j<segs; j++) {
		calculate_chunk(&start, &end, me, j, 64);

		// end is always xxxxxffc, so increment so that length calculations are correct
		end = end + 1;

		pe = start;
		p = start;

		done = 0;
		do {
			do_tick(me);
			BAILR

			/* Check for overflow */
			if (pe + SPINSZ > pe && pe != 0) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
			len  = ((ulong)pe - (ulong)p) / 64;
			//len++;
#warning
#if OPTIMIZED && HAS_OPT_BLOCK_MOVE && OPTIMIZED_FIRST_SNIPPET
			block_move_snippet1(&p, len);
#endif
		} while (!done);
	}
	s_barrier();

	/* Now move the data around 
	 * First move the data up half of the segment size we are testing
	 * Then move the data to the original location + 32 bytes
	 */
	for (j=0; j<segs; j++) {
		calculate_chunk(&start, &end, me, j, 64);

		// end is always xxxxxffc, so increment so that length calculations are correct
		end = end + 1;
		pe = start;
		p = start;
		done = 0;

		do {

			/* Check for overflow */
			if (pe + SPINSZ > pe && pe != 0) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
			pp = (ulong)p + (((ulong)pe - (ulong)p) / 2); // Mid-point of this block
			len  = ((ulong)pe - (ulong)p) / 8; // Half the size of this block in DWORDS
			for(i=0; i<iter; i++) {
				do_tick(me);
				BAILR
#warning
#if OPTIMIZED && HAS_OPT_BLOCK_MOVE && OPTIMIZED_SECOND_SNIPPET
			block_move_snippet2(p, pp, len);
#endif
			}
			p = pe;
		} while (!done);
	}
	s_barrier();

	/* Now check the data 
	 * The error checking is rather crude.  We just check that the
	 * adjacent words are the same.
	 */
	for (j=0; j<segs; j++) {
		calculate_chunk(&start, &end, me, j, 64);

		// end is always xxxxxffc, so increment so that length calculations are correct
		end = end + 1;
		pe = start;
		p = start;
		done = 0;
		do {
			do_tick(me);
			BAILR

			/* Check for overflow */
			if (pe + SPINSZ > pe && pe != 0) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
			pe-=2;	/* the last dwords to test are pe[0] and pe[1] */
#warning
#if OPTIMIZED && HAS_OPT_BLOCK_MOVE && OPTIMIZED_THIRD_SNIPPET
			block_move_snippet3(&p, pe);
#endif
		} while (!done);
	}
}

/*
 * Test memory for bit fade, fill memory with pattern.
 */
void bit_fade_fill(uint32_t p1, int me)
{
	int j, done;
	uint32_t *p, *pe;
	uint32_t *start,*end;

	/* Display the current pattern */
	hprint(LINE_PAT, COL_PAT, p1);

	/* Initialize memory with the initial pattern.  */
	for (j=0; j<segs; j++) {
		start = v->map[j].start;
		end = v->map[j].end;
		pe = (uint32_t *)start;
		p = start;
		done = 0;
		do {
			do_tick(me);
			BAILR

			/* Check for overflow */
			if (pe + SPINSZ > pe && pe != 0) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
 			for (; p < pe;) {
				*p = p1;
				p++;
			}
			p = pe + 1;
		} while (!done);
	}
}

void bit_fade_chk(uint32_t p1, int me)
{
	int j, done;
	uint32_t *p, *pe, bad;
	uint32_t *start,*end;

	/* Make sure that nothing changed while sleeping */
	for (j=0; j<segs; j++) {
		start = v->map[j].start;
		end = v->map[j].end;
		pe = (uint32_t *)start;
		p = start;
		done = 0;
		do {
			do_tick(me);
			BAILR

			/* Check for overflow */
			if (pe + SPINSZ > pe && pe != 0) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
 			for (; p < pe;) {
 				if ((bad=*p) != p1) {
					error((uint32_t*)p, p1, bad);
				}
				p++;
			}
			p = pe + 1;
		} while (!done);
	}
}




/* Sleep for N seconds */
void sleep(long n, int flag, int me, int sms)
{
	uint64_t st;
	ulong t, ip=0;
	/* save the starting time */
	st = RDTSC();

	/* loop for n seconds */
	ulong deadline;
	if (sms) {
		deadline = st + n * v->clks_msec;
	} else {
		deadline = st + n * v->clks_msec * 1000;
	}
	while (1) {
		t = RDTSC();
		
		/* Is the time up? */
		if (t >= deadline) {
			break;
		}

		/* Only display elapsed time if flag is set */
		if (flag == 0) {
			continue;
		}

		if (t != ip) {
			do_tick(me);
			BAILR
			ip = t;
		}
	}
}
