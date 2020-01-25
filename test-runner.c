/*
 * This is a userspace launcher for various memory tests.
 * You can use it to simplify debugging of optimized
 * test implementations.
 */

#include <stdio.h>
#include <sys/mman.h>

// Not including stdlib.h due to clashes for now
void abort(void);
unsigned long int strtoul(const char *nptr, char **endptr, int base);
char *getenv(const char *name);

#include "globals.h"
#include "test.h"

static bool mute_patterns;
#define EXIT_IF_PATTERN_LINE_AND_MUTED if (y == LINE_PAT && mute_patterns) return;

void do_tick(void)
{
	// do nothing
}

void s_barrier(void)
{
	// TODO
	// do nothing FOR NOW
}

void set_cache(int x)
{
	(void)x;
	// do nothing
}

void serial_console_setup(const char *param)
{
	(void)param;
	// do nothing
}

unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base) {
	return strtoul(cp, endp, base);
}

#if !HAS_FLAT_MEM // otherwise they are already defined
int map_page(unsigned long page)
{
    return 0;
}

void *mapping(unsigned long page_addr)
{
    return (void *)(page_addr << 12);
}

unsigned long page_of(void *addr)
{
    return ((uintptr_t)addr) >> 12;
}
#endif

void cprint(int y,int x, const char *s)
{
	EXIT_IF_PATTERN_LINE_AND_MUTED

	if (y == LINE_PAT && x == COL_PAT) {
		fprintf(stderr, "Pattern: %s\n", s);
	} else {
		fprintf(stderr, "cprint [%02d, %02d] %s\n", y, x, s);
	}
}

void hprint(int y,int x, ulong val)
{
	EXIT_IF_PATTERN_LINE_AND_MUTED

	if (y == LINE_PAT && x == COL_PAT) {
		fprintf(stderr, "Pattern: %08lx\n", val);
	} else {
		fprintf(stderr, "hprint [%02d, %02d] %lx\n", y, x, val);
	}
}

void dprint(int y,int x,ulong val,int len, int right)
{
	EXIT_IF_PATTERN_LINE_AND_MUTED

	fprintf(stderr, "dprint [%02d, %02d] %lu\n", y, x, val);
}

void aprint(int y,int x,ulong page)
{
	EXIT_IF_PATTERN_LINE_AND_MUTED

	fprintf(stderr, "aprint [%02d, %02d] page = %08lx\n", y, x, page);
}

ulong virt_addr(void *addr)
{
	uintptr_t i_addr = (uintptr_t) addr;
	return i_addr - (uintptr_t)v->map[0].start;
}

void ad_err1(uint32_t *addr, uint32_t mask, uint32_t bad, uint32_t good)
{
	fprintf(stderr, "ad_err1: addr = 0x%08lx    mask = 0x%08x    bad = 0x%08x    good = 0x%08x\n",
		virt_addr(addr), mask, bad, good);
}

void ad_err2(uint32_t *addr, uint32_t bad)
{
	fprintf(stderr, "ad_err2: addr = 0x%08lx    bad = 0x%08x\n",
		virt_addr(addr), bad);
}

void error(ulong *addr, ulong good, ulong bad)
{
	fprintf(stderr, "error:   addr = 0x%08lx    good = 0x%08lx    bad = 0x%08lx\n",
		virt_addr(addr), good, bad);
	if (!getenv("KEEP_GOING")) {
		abort();
	}
}

// allocate in low half of address space
#define VMEM_SZ (1 << 20)
static uint8_t vmem[VMEM_SZ + (1 << 12)];

static void setup(void)
{
	// align at the next page boundary
	uint8_t *vmem_start = mapping(page_of(&vmem) + 1);
	uint8_t *vmem_end   = vmem_start + VMEM_SZ;
	fprintf(stderr, "Memory allocated: %p - %p\n", vmem_start, vmem_end);
	v->pmap[0].start = page_of(vmem_start);
	v->pmap[0].end   = page_of(vmem_end);
	segs = v->msegs = 1;
	rdtsc_is_available = 1;
	run_cpus = 1;
}

int main(int argc, const char *argv[])
{
	setup();
	set_defaults();

	mute_patterns = getenv("MUTE_PATTERNS") != NULL;

	v->plim_lower = v->pmap[0].start;
	v->plim_upper = v->pmap[0].end;
	v->map[0].start =  mapping(v->plim_lower);
	v->map[0].end   = emapping(v->plim_upper);

	// Use at least two iterations, so snippet3() is checked against snippet2()
	// in cases like this:
	//
	// snippet1();
	// for (int i =0; i < iter; ++i) {
	//     snippet2();
	//     snippet3();
	// }
	c_iter = 2;

	fprintf(stderr, "Usage: %s [\"boot command line\"]\n", argv[0]);
	const char *cmdline = "";
	if (argc > 2) {
		fprintf(stderr, "Please pass all command line as a single parameter.\n");
		return 1;
	} else if (argc > 1) {
		cmdline = argv[1];
	}

 	parse_command_line(cmdline);

	while (pass_flag < 2) {
		fprintf(stderr, "Pass #%d Test: #%2d %s\n", pass_flag, tseq[test].pat, tseq[test].msg);
		invoke_test(0);
		next_test();
	}
}
