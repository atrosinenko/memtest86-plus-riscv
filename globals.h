#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>

#include "defs.h"

typedef char bool;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

extern bool rdtsc_is_available;
#define RDTSC_AVAILABLE() (rdtsc_is_available)

extern volatile short   start_seq;
extern unsigned long high_test_adr;

extern int slock, lsr;

extern int l1_cache, l2_cache, l3_cache;
extern short serial_tty;
extern short serial_cons;
extern int serial_baud_rate;
extern unsigned char serial_parity;
extern unsigned char serial_bits;

struct barrier_s;

extern int c_iter;
extern int dmi_err_cnts[MAX_DMI_MEMDEVS];
extern int beepmode;
extern short dmi_initialized;
extern struct cpu_ident cpu_id;
extern int test_ticks, nticks;
extern volatile int test;
extern int pass_flag;
extern unsigned num_cpus;
extern volatile int run_cpus;
extern unsigned act_cpus;
extern unsigned found_cpus;
extern volatile short btflag;
extern volatile short cpu_mode;
extern volatile int bail;
extern volatile int window;
extern volatile unsigned long win_next;
extern short restart_flag;
extern int bitf_seq;
extern volatile short cpu_sel;
extern volatile int mstr_cpu;
extern int ltest;
extern short onepass;
extern volatile int segs;

extern short memsz_mode;
extern int maxcpus;
extern char cpu_mask[];

extern volatile unsigned long win0_start;   /* Start test address for window 0 */
extern volatile unsigned long win1_end;     /* End address for relocation */
extern struct    barrier_s *barr;

char	toupper(char c);
int	isxdigit(char c);
void	reboot();
void	bzero();
void	smp_set_ordinal(int me, int ord);
int	smp_my_ord_num(int me);
int	smp_ord_to_cpu(int me);
void	get_cpuid();
void	initialise_cpus();
unsigned long	memtest_rand(int cpu);
void	rand_seed(unsigned int seed1, unsigned int seed2, int cpu);

unsigned long memspeed(unsigned long src, unsigned long len, int iter);
void cpu_type(void);
int cpuspeed(void);
void get_cache_size();
void cpu_cache_speed();
void get_cpuid();
void set_defaults(void);
void arch_init_early(void);
void arch_init(void);

unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base);

void btrace(int me, int line, char *msg, int wait, long v1, long v2);
void memtest_main(int my_cpu_num, int my_cpu_ord);

#endif
