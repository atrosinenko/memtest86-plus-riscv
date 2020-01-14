#ifndef GLOBALS_H
#define GLOBALS_H

#include "arch.h"
#include "test.h"

extern int l1_cache, l2_cache, l3_cache;
extern short serial_tty;
extern short serial_cons;
extern int serial_baud_rate;
extern unsigned char serial_parity;
extern unsigned char serial_bits;

struct barrier_s;

extern int dmi_err_cnts[MAX_DMI_MEMDEVS];
extern int beepmode;
extern short dmi_initialized;
extern struct cpu_ident cpu_id;
extern struct barrier_s *barr;
extern int test_ticks, nticks;
extern struct tseq tseq[];
extern volatile int test;
extern unsigned num_cpus;
extern unsigned act_cpus;
extern unsigned found_cpus;

extern short memsz_mode;
extern int maxcpus;
extern char cpu_mask[];

extern volatile ulong win0_start;   /* Start test address for window 0 */
extern volatile ulong win1_end;     /* End address for relocation */
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
ulong	rand(int cpu);
void	rand_seed(unsigned int seed1, unsigned int seed2, int cpu);

ulong memspeed(ulong src, ulong len, int iter);
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
void main(int my_cpu_num, int my_cpu_ord);

#endif
