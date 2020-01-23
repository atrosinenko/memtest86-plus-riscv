#include <stddef.h>
#include <stdint.h>

#include "arch.h"
#include "test.h"
#include "io.h"
#include "extra.h"
#include "smp.h"
#include "globals.h"

typedef uint32_t fdt32_t;
typedef uint64_t fdt64_t;
#include "fdt.h"

static char cur_path[256];

struct barrier_s barrier_struct;

uint64_t uart_base;
char dummy_con[80*25*2];

uint64_t low_test_addr;
uint64_t hz_clock = DEFAULT_HZ_CLOCK;
extern uint64_t initial_load_addr;

static void parse_fdt(uint8_t *dtb)
{
	const char *compatible = NULL;
	const char *device_type = NULL;
	uint64_t reg_start, reg_size;

	int path_len = 0;

	const struct fdt_header *hdr = (struct fdt_header *)dtb;
	const char *strtab = (const char *)(dtb + be32(hdr->off_dt_strings));

	const uint8_t *cur_ptr = dtb + be32(hdr->off_dt_struct);
	int cont = 1;
	while (cont) {
		// align at 4-byte boundary
		while ((uintptr_t)cur_ptr & 3)
			++cur_ptr;

		switch (be32(*(uint32_t *)cur_ptr)) {
		case FDT_BEGIN_NODE:
			cur_path[path_len++] = '/';
			for (cur_ptr += 4; *cur_ptr; ++cur_ptr, ++path_len) {
				cur_path[path_len] = *cur_ptr;
			}
			cur_path[path_len] = '\0';
			++cur_ptr;

			compatible = NULL;
			device_type = NULL;

			break;
		case FDT_END_NODE:
			cur_ptr += 4;
			for (; cur_path[path_len] != '/'; --path_len);
			cur_path[path_len] = '\0';
			if (device_type && memcmp(device_type, "memory", 7) == 0) {
				v->pmap[v->msegs].start = reg_start >> 12;
				v->pmap[v->msegs].end   = (reg_start + reg_size) >> 12;
				v->test_pages += v->pmap[v->msegs].end - v->pmap[v->msegs].start;
				v->msegs += 1;
			}
			if (compatible && memcmp(compatible, "sifive,uart0", 13) == 0) {
				uart_base = reg_start;
			}

			compatible = NULL;
			device_type = NULL;

			break;
		case FDT_PROP:
			{
				cur_ptr += 4;
				uint32_t len    = be32(*(uint32_t *)(cur_ptr + 0));
				uint32_t stroff = be32(*(uint32_t *)(cur_ptr + 4));
				const char *param_name = strtab + stroff;
				const uint8_t *payload = cur_ptr + 8;

				if (memcmp(cur_path, "//chosen", 9) == 0 && memcmp(param_name, "bootargs", 9) == 0) {
					parse_command_line((const char *) payload);
				}
				if (memcmp(param_name, "compatible", 11) == 0) {
					compatible = (const char *) payload;
				}
				if (memcmp(param_name, "device_type", 12) == 0) {
					device_type = (const char *) payload;
				}
				if (memcmp(param_name, "reg", 4) == 0) {
					if (len == 4) {
						reg_start = be32(*(uint32_t*)(payload + 0));
						reg_size = 0;
					} else if (len == 8) {
						reg_start = be32(*(uint32_t*)(payload + 0));
						reg_size  = be32(*(uint32_t*)(payload + 4));
					} else {
						reg_start  = be32(*(uint32_t*)(payload + 0)); reg_start <<= 32;
						reg_start |= be32(*(uint32_t*)(payload + 4));
						reg_size   = be32(*(uint32_t*)(payload + 8)); reg_size <<= 32;
						reg_size  |= be32(*(uint32_t*)(payload + 12));
					}
				}

				cur_ptr += len + 8;
			}
			break;
		case FDT_NOP:
			cur_ptr += 4;
			break;
		case FDT_END:
		default:
			cont = 0;
			break;
		}
	}

	// TODO
	// sort_pmap();

	v->plim_lower = v->pmap[0].start;
	v->plim_upper = v->pmap[v->msegs].end;
	low_test_addr = v->plim_lower << 12;
}

#define MSECS 20
#define TICKS (MSECS * HZ_CLOCK / 1000)
int cpuspeed(void)
{
	uint32_t tsc_start, tsc_end;
	int32_t rtc_lo_start, rtc_lo_end;

	tsc_start = RDTSC();
	rtc_lo_start = IN(int32_t, RTC_LO, 0);

	while (1) {
		rtc_lo_end = IN(int32_t, RTC_LO, 0);
		if (rtc_lo_end - rtc_lo_start > TICKS) {
			break;
		}
	}
	tsc_end = RDTSC();

	v->clks_msec = (tsc_end - tsc_start) / MSECS;
	return v->clks_msec;
}

void reboot(void)
{
}

int get_key() {
	struct rxdata {
		uint32_t data:8;
		uint32_t reserved:23;
		uint32_t empty:1;
	};
	struct rxdata rxd;

	rxd = IN(struct rxdata, UART_BASE, 4);
	if (!rxd.empty) {
		return (ascii_to_keycode(rxd.data));
	}
	return(0);
}

void serial_echo_init(void)
{
	struct txctrl {
		uint32_t txen:1;
		uint32_t nstop:1;
		uint32_t reserved1:14;
		uint32_t txcnt:3;
		uint32_t reserved2:13;
	};
	struct rxctrl {
		uint32_t rxen:1;
		uint32_t reserved1:15;
		uint32_t rxcnt:3;
		uint32_t reserved2:13;
	};

	struct txctrl txc;
	struct rxctrl rxc;

	txc = IN(struct txctrl, UART_BASE, 8);
	txc.txen = 1;
	txc.nstop = 0;
	OUT(struct txctrl, UART_BASE, 8, txc);

	rxc = IN(struct rxctrl, UART_BASE, 8);
	rxc.rxen = 1;
	OUT(struct rxctrl, UART_BASE, 8, rxc);

	// Not changind baudrate for now, using default 115200
}

void serial_echo_print(const char *p)
{
	struct txdata {
		uint32_t data:8;
		uint32_t reserved1:23;
		uint32_t full:1;
	};
	struct txdata txd;

	if (!serial_cons) {
		return;
	}
	/* Now, do each character */
	while (*p) {
		// wait
		while ((txd = IN(struct txdata, UART_BASE, 0)).full);

		txd.data = *p;
		OUT(struct txdata, UART_BASE, 0, txd);

		if(*p==10) {
			// wait
			while ((txd = IN(struct txdata, UART_BASE, 0)).full);
			txd.data = 13;
			OUT(struct txdata, UART_BASE, 0, txd);
		}
		p++;
	}
}

void arch_init_early(void)
{
}

void arch_init(void)
{
	cpu_cache_speed();
}

static void move_to_correct_addr(void)
{
	uintptr_t cur_start = (uintptr_t)&_start;
	uintptr_t cur_end   = (uintptr_t)&_end;
	if (cur_start == low_test_addr || cur_start == high_test_adr) {
		return;
	}
	if (cur_start == initial_load_addr &&
		(cur_start - low_test_addr) < (cur_end - cur_start)
	) {
		// Optional first relocation, move slightly after initial position
		// **Optional**, because we cannot perform it if loaded at the highest addresses
// 		serial_echo_print("FIRST STARTUP RELOCATION...\n");
		uintptr_t temp_addr = (((uintptr_t)&_end >> 12) + 1) << 12;
		run_at(temp_addr, 0);
	} else {
		// final startup relocation
		// now we **definitely** have room at the beginning
// 		serial_echo_print("FINAL STARTUP RELOCATION...\n");
		run_at(low_test_addr, 0);
	}
}

void riscv_entry(uint64_t hart_id, uint64_t fdt_address)
{
	int my_cpu_num = 0, my_cpu_ord = 0;
	if (start_seq == 0) {
		parse_fdt((uint8_t *)fdt_address);
		serial_echo_init();
		serial_echo_print("PARSED DTB\n");
	}
	switch_to_main_stack(0);

	if (start_seq < 2) {
		smp_set_ordinal(my_cpu_num, my_cpu_ord);
		clear_screen();
		barrier_init(1);
		btrace(my_cpu_num, __LINE__, "Begin     ", 1, 0, 0);
		init();
		set_defaults();
		win0_start = 0x100;

		/* Set relocation address to 32Mb if there is enough
			* memory. Otherwise set it to 3Mb */
		/* Large reloc addr allows for more testing overlap */
		if ((ulong)v->test_pages > 0x2f00) {
			high_test_adr = (v->plim_lower << 12) + 0x2000000;
		} else {
			high_test_adr = (v->plim_lower << 12) + 0x300000;
		}
		win1_end = (high_test_adr >> 12);

		find_ticks_for_pass();
	} else {
		spin_unlock(&barr->mutex);
		/* Get the CPU ordinal since it is lost during relocation */
		my_cpu_ord = smp_my_ord_num(my_cpu_num);
		btrace(my_cpu_num, __LINE__, "Reloc_Done",0,my_cpu_num,my_cpu_ord);
	}

	/* A barrier to insure that all of the CPUs are done with startup */
	barrier();
	btrace(my_cpu_num, __LINE__, "1st Barr  ", 1, my_cpu_num, my_cpu_ord);


	/* Setup Memory Management and measure memory speed, we do it here
	 * because we need all of the available CPUs */
	if (start_seq < 2) {

	    btrace(my_cpu_num, __LINE__, "Mem Mgmnt ", 1, 0, 0);
	    /* Get the memory Speed with all CPUs */
	    get_mem_speed(my_cpu_num, 1);
	}

	/* Set the initialized flag only after all of the CPU's have
	 * Reached the barrier. This insures that relocation has
	 * been completed for each CPU. */
	btrace(my_cpu_num, __LINE__, "Start Done", 1, 0, 0);
	start_seq = 2;

	move_to_correct_addr();

	main(my_cpu_num, my_cpu_ord);
}

// From The RISC-V Instruction Set Manual, Volume II: Privileged Architecture, Document Version 20190608-Priv-MSU-Ratified”
static const char *errors[] = {
	"Instruction address misaligned",
	"Instruction access fault",
	"Illegal instruction",
	"Breakpoint",
	"Load address misaligned",
	"Load access fault",
	"Store/AMO address misaligned",
	"Store/AMO access fault",
	"8", "9", "10", "11",
	"Instruction page fault",
	"Load page fault",
	"14",
	"Store/AMO page fault",
};

void riscv_trap_entry(ulong cause, ulong epc, ulong tval)
{
	char buf[32];

	cprint(12, 0, "EXCP:                                  ");
	if (cause < sizeof(errors) / sizeof(errors[0])) {
		cprint(12, 8, errors[cause]);
	} else {
		itoa(buf, cause);
		cprint(12, 8, buf);
	}

	cprint(13, 0, "PC:                                    ");
	hprint3(13, 8, epc, 8);

	cprint(14, 0, "Addr:                                  ");
	hprint3(14, 8, tval, 8);

	HALT();
}

