#ifndef ARCH_H
#define ARCH_H

#include <stdint.h>

#include "config.h"

extern uint64_t low_test_addr;
#define LOW_TEST_ADR (low_test_addr)

extern uint64_t uart_base;
extern uint64_t hz_clock;
#define DEFAULT_HZ_CLOCK 1000000
#define HZ_CLOCK (hz_clock)
#define UART_BASE (uart_base)
#define RTC_LO    0x0200bff8

#define __ELF_NATIVE_CLASS 64
#define ELF_MACHINE_NO_RELA 0

#define HALT() { while(1){} }

extern struct barrier_s barrier_struct;
#define BARRIER_ADDR (&barrier_struct)

extern char dummy_con[80*25*2];
#define SCREEN_ADR     (dummy_con)
#define SCREEN_END_ADR (dummy_con + 80*25*2)

#define assert(x) { if (!(x)) HALT(); }

#define RDTSC_AVAILABLE() (1)
#if UINTMAX == 0xffffffff
// 32-bit
#define RDTSC_LH(low, high) { asm __volatile__ ("rdcycle %0\n" "rdcycleh %1\n" : "=r"(low), "=r"(high)); }
static inline uint64_t RDTSC(void)
{
   unsigned long l, h;
   RDTSC_LH(l, h);
   return (((uint64_t)h) << 32) | l;
}
#else
// 64-bit
#define RDTSC_LH(low, high) { asm __volatile__ ( \
	"rdcycle %0\n" \
	"srli %1, %0, 32\n" \
	"slli %0, %0, 32\n" \
	"srli %0, %0, 32\n" \
	: "=r"(low), "=r"(high)); }

static inline uint64_t RDTSC()
{
	uint64_t result;
	asm volatile ("rdcycle %0\n" : "=r"(result));
	return result;
}
#endif

#define ADJUST_STACK(offs) { __asm__ __volatile__ ( "sub sp, sp, %0" : : "r" (offs) : "memory" ); }

static inline uint32_t be32(uint32_t x)
{
	return (x << 24) | (x >> 24) | ((x & 0xff0000) >> 8) | ((x & 0xff00) << 8);
}

#endif
