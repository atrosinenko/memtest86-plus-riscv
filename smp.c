#include "arch.h"
#include "globals.h"
#include "test.h"
#include "smp.h"

unsigned act_cpus;
unsigned found_cpus = 0;
unsigned num_cpus = 1; // There is at least one cpu, the BSP
struct barrier_s *barr;

void barrier_init(int max)
{
	/* Set the adddress of the barrier structure */
	barr = (struct barrier_s *)BARRIER_ADDR;
        barr->lck.slock = 1;
        barr->mutex.slock = 1;
        barr->maxproc = max;
        barr->count = max;
        barr->st1.slock = 1;
        barr->st2.slock = 0;
}

void s_barrier_init(int max)
{
        barr->s_lck.slock = 1;
        barr->s_maxproc = max;
        barr->s_count = max;
        barr->s_st1.slock = 1;
        barr->s_st2.slock = 0;
}

void barrier()
{
	if (num_cpus == 1 || v->fail_safe & 3) {
		return;
	}
	spin_wait(&barr->st1);     /* Wait if the barrier is active */
        spin_lock(&barr->lck);	   /* Get lock for barr struct */
        if (--barr->count == 0) {  /* Last process? */
                barr->st1.slock = 0;   /* Hold up any processes re-entering */
                barr->st2.slock = 1;   /* Release the other processes */
                barr->count++;
                spin_unlock(&barr->lck);
        } else {
                spin_unlock(&barr->lck);
                spin_wait(&barr->st2);	/* wait for peers to arrive */
                spin_lock(&barr->lck);
                if (++barr->count == barr->maxproc) {
                        barr->st1.slock = 1;
                        barr->st2.slock = 0;
                }
                spin_unlock(&barr->lck);
        }
}

void s_barrier()
{
	if (run_cpus == 1 || v->fail_safe & 3) {
		return;
	}
	spin_wait(&barr->s_st1);     /* Wait if the barrier is active */
        spin_lock(&barr->s_lck);     /* Get lock for barr struct */
        if (--barr->s_count == 0) {  /* Last process? */
                barr->s_st1.slock = 0;   /* Hold up any processes re-entering */
                barr->s_st2.slock = 1;   /* Release the other processes */
                barr->s_count++;
                spin_unlock(&barr->s_lck);
        } else {
                spin_unlock(&barr->s_lck);
                spin_wait(&barr->s_st2);	/* wait for peers to arrive */
                spin_lock(&barr->s_lck);
                if (++barr->s_count == barr->s_maxproc) {
                        barr->s_st1.slock = 1;
                        barr->s_st2.slock = 0;
                }
                spin_unlock(&barr->s_lck);
        }
}
