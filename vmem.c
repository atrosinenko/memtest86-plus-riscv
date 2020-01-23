/* vmem.c - MemTest-86 
 *
 * Virtual memory handling (PAE)
 *
 * Released under version 2 of the Gnu Public License.
 * By Chris Brady
 */

#include <stdint.h>

#include "defs.h"

#if HAS_FLAT_MEM
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

void *emapping(unsigned long page_addr)
{
	void *result;
	result = mapping(page_addr -1);
	/* Fill in the low address bits */
	result = ((unsigned char *)result) + 0xffc;
	return result;
}

