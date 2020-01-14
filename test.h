/* test.h - MemTest-86  Version 3.4
 *
 * Released under version 2 of the Gnu Public License.
 * By Chris Brady
 */

#ifndef _TEST_H_
#define _TEST_H_

#include "arch.h"

typedef unsigned long ulong;
#ifndef __ASSEMBLY__

#define STACKSIZE       (8*1024)
#define MAX_MEM         0x7FF00000      /* 8 TB */
#define WIN_SZ          0x80000         /* 2 GB */
#define UNMAP_SZ        (0x100000-WIN_SZ)  /* Size of umappped first segment */

#define SPINSZ		0x4000000	/* 256 MB */
#define MOD_SZ		20
#define BAILOUT		if (bail) return(1);
#define BAILR		if (bail) return;

#define TITLE_WIDTH	 28
#define LINE_TITLE 		0
#define LINE_TST			3
#define LINE_RANGE		4
#define LINE_PAT      5
#define LINE_TIME			5
#define LINE_STATUS		8
#define LINE_INFO			9
#define LINE_HEADER	 12
#define LINE_SCROLL	 14
#define LINE_SPD 		 14
#define LINE_MSG		 22
#define LINE_CPU			7
#define LINE_RAM			8
#define LINE_DMI		 23
#define COL_INF1        15
#define COL_INF2        32
#define COL_INF3        51
#define COL_INF4        70
#define COL_MODE	15
#define COL_MID		30
#define COL_PAT		41
#define BAR_SIZE	(78-COL_MID-9)
#define COL_MSG		23
#define COL_TIME	67
#define COL_SPEC 41

#define POP_W	34
#define POP_H	15
#define POP_X	11
#define POP_Y	8
#define POP2_W  74
#define POP2_H  21
#define POP2_X  3
#define POP2_Y  2

/* CPU mode types */
#define CPM_ALL    1
#define CPM_RROBIN 2
#define CPM_SEQ    3

/* memspeed operations */
#define MS_COPY		1
#define MS_WRITE	2
#define MS_READ		3

#define SZ_MODE_BIOS		1
#define SZ_MODE_PROBE		2

extern int beepmode;
void parse_command_line(const char *cp);
void switch_to_main_stack(unsigned cpu_num);
void run_at(unsigned long addr, int cpu);
void get_mem_speed(int cpu, int ncpus);
int memcmp(const void *s1, const void *s2, ulong count);
void *memmove(void *dest, const void *src, ulong n);
int strncmp(const char *s1, const char *s2, ulong n);
int strstr(char *str1, char *str2);
int strlen(char *string);
int query_linuxbios(void);
int query_pcbios(void);
int insertaddress(ulong);
void printpatn(void);
void printpatn(void);
void itoa(char s[], int n); 
void reverse(char *p);
void serial_console_setup(const char *param);
void serial_echo_init(void);
void serial_echo_print(const char *s);
void ttyprint(int y, int x, const char *s);
void ttyprintc(int y, int x, char c);
void cprint(int y,int x, const char *s);
void cplace(int y,int x, const char s);
void hprint(int y,int x, ulong val);
void hprint2(int y,int x, ulong val, int len);
void hprint3(int y,int x, ulong val, int len);
void xprint(int y,int x,ulong val);
void aprint(int y,int x,ulong page);
void dprint(int y,int x,ulong val,int len, int right);
void movinv1(int iter, uint32_t p1, uint32_t p2, int cpu);
void movinvr(int cpu);
void movinv32(int iter, uint32_t p1, uint32_t lb, uint32_t mb, int sval, int off,
	int cpu);
void modtst(int off, int iter, uint32_t p1, uint32_t p2, int cpu);
void error(ulong* adr, ulong good, ulong bad);
void ad_err1(ulong *adr1, ulong *adr2, ulong good, ulong bad);
void ad_err2(ulong *adr, ulong bad);
void do_tick();
void init(void);
struct eregs;
void inter(struct eregs *trap_regs);
void set_cache(int val);
void check_input(void);
void footer(void);
void scroll(void);
void clear_scroll(void);
void popup(void);
void popdown(void);
void popclear(void);
void pop2up(void);
void pop2down(void);
void pop2clear(void);
void get_config(void);
void get_menu(void);
void get_printmode(void);
void addr_tst1(int cpu);
void addr_tst2(int cpu);
int getnum(ulong val);
void sleep(long sec, int flag, int cpu, int sms);
void block_move(int iter, int cpu);
void find_ticks(void);
void print_err(ulong *adr, ulong good, ulong bad, ulong xor);
void print_ecc_err(ulong page, ulong offset, int corrected, 
	unsigned short syndrome, int channel);
void mem_size(void);
void adj_mem(void);
ulong getval(int x, int y, int result_shift);
int get_key(void);
int ascii_to_keycode(int in);
void wait_keyup(void);
void print_hdr(void);
void restart(void);
void parity_err(ulong edi, ulong esi);
void start_config(void);
void clear_screen(void);
void paging_off(void);
void show_spd(void);
int map_page(unsigned long page);
void *mapping(unsigned long page_address);
void *emapping(unsigned long page_address);
int isdigit(char c);
ulong memspeed(ulong src, ulong len, int iter);
unsigned long page_of(void *ptr);
ulong correct_tsc(ulong el_org);
void bit_fade_fill(uint32_t n, int cpu);
void bit_fade_chk(uint32_t n, int cpu);
void find_ticks_for_pass(void);
void beep(unsigned int frequency);

#define PRINTMODE_SUMMARY   0
#define PRINTMODE_ADDRESSES 1
#define PRINTMODE_PATTERNS  2
#define PRINTMODE_NONE      3

#define BADRAM_MAXPATNS 10

struct pair {
       ulong adr;
       ulong mask;
};

struct mmap {
	ulong pbase_addr;
	ulong *start;
	ulong *end;
};

struct pmap {
	ulong start;
	ulong end;
};

struct tseq {
	short sel;
	short cpu_sel;
	short pat;
	short iter;
	short errors;
	char *msg;
};

struct xadr {
	ulong page;
	ulong offset;
};

struct err_info {
	struct xadr   low_addr;
	struct xadr   high_addr;
	unsigned long ebits;
	long	      tbits;
	short         min_bits;
	short         max_bits;
	unsigned long maxl;
	unsigned long eadr;
        unsigned long exor;
        unsigned long cor_err;
	short         hdr_flag;
};


/* Define common variables accross relocations of memtest86 */
struct vars {
	int pass;
	int msg_line;
	int ecount;
	int ecc_ecount;
	int msegs;
	int testsel;
	int scroll_start;
	int pass_ticks;
	int total_ticks;
	int pptr;
	int tptr;
	struct err_info erri;
	struct pmap pmap[MAX_MEM_SEGMENTS];
	volatile struct mmap map[MAX_MEM_SEGMENTS];
	ulong plim_lower;
	ulong plim_upper;
	ulong clks_msec;
	uint64_t startt;
	uint64_t snapt;
	int printmode;
	int numpatn;
	struct pair patn [BADRAM_MAXPATNS];
	ulong test_pages;
	ulong selected_pages;
	ulong reserved_pages;
	int check_temp;
	int fail_safe;
	int each_sec;
	int beepmode;
};

#define FIRMWARE_UNKNOWN   0
#define FIRMWARE_PCBIOS    1
#define FIRMWARE_LINUXBIOS 2

extern struct vars * const v;
extern unsigned char _start[], _end[], startup_32[];
extern unsigned char _size, _pages;

#include "globals.h"

#endif /* __ASSEMBLY__ */
#endif /* _TEST_H_ */
