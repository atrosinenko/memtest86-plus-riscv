#include "defs.h"
#include "globals.h"
#include "test.h"
#include "dmi.h"

void cache_on(void)
{
}

void cache_off(void)
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

short dmi_initialized=0;
int dmi_err_cnts[MAX_DMI_MEMDEVS];
void print_dmi_info(void)
{
}

void beep(unsigned int frequency)
{
}

