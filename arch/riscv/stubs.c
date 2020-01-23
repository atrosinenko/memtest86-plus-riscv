#include "arch.h"
#include "globals.h"
#include "test.h"
#include "dmi.h"

void cache_on(void)
{
}

void cache_off(void)
{
}

void paging_off(void)
{
}

ulong memspeed(ulong src, ulong len, int iter)
{
	return 1;
}

void get_spd_spec(void)
{
}

void show_spd(void)
{
}

void coretemp(void)
{
}
void find_controller(void)
{
}
void poll_errors(void)
{
}

unsigned smp_my_cpu_num()
{
	return 0;
}

int smp_my_ord_num(int me)
{
	return 0;
}

void smp_set_ordinal(int me, int ord)
{
}

void initialise_cpus(void)
{
	act_cpus = found_cpus = num_cpus = 1;

	/* Initialize the barrier before starting AP's */
	barrier_init(act_cpus);
}

short dmi_initialized=0;
int dmi_err_cnts[MAX_DMI_MEMDEVS];
void print_dmi_info(void)
{
}

void beep(unsigned int frequency)
{
}

