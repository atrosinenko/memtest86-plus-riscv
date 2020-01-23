/* Most of the original contents of smp.h moved to arch/i386/arch-smp.h */

#ifndef _SMP_H_
#define _SMP_H_
#include "stdint.h"
#include "defs.h"
#include "arch-smp.h"
#define MAX_CPUS 32

unsigned smp_my_cpu_num();

void smp_init_bsp(void);
void smp_init_aps(void);

void smp_boot_ap(unsigned cpu_num);
void smp_ap_booted(unsigned cpu_num);

struct barrier_s
{
        spinlock_t mutex;
        spinlock_t lck;
        int maxproc;
        volatile int count;
        spinlock_t st1;
        spinlock_t st2;
        spinlock_t s_lck;
        int s_maxproc;
        volatile int s_count;
        spinlock_t s_st1;
        spinlock_t s_st2;
};

void barrier();
void s_barrier();
void barrier_init(int max);
void s_barrier_init(int max);

#endif /* _SMP_H_ */
