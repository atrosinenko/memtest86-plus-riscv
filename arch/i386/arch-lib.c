#include "arch.h"
#include "serial.h"

static const short serial_base_ports[] = {0x3f8, 0x2f8};

void reboot(void)
{
	/* tell the BIOS to do a cold start */
	*((unsigned short *)0x472) = 0x0;
	while(1)
	{
		outb(0xFE, 0x64);
		outb(0x02, 0xcf9); /* reset that doesn't rely on the keyboard controller */
		outb(0x04, 0xcf9);
		outb(0x0E, 0xcf9);
	}
}

int get_key() {
	int c;

	c = inb(0x64);
	if ((c & 1) == 0) {
		if (serial_cons) {
			int comstat;
			comstat = serial_echo_inb(UART_LSR);
			if (comstat & UART_LSR_DR) {
				c = serial_echo_inb(UART_RX);
				/* Pressing '.' has same effect as 'c'
				   on a keyboard.
				   Oct 056   Dec 46   Hex 2E   Ascii .
				*/
				return (ascii_to_keycode(c));
			}
		}
		return(0);
	}
	c = inb(0x60);
	return((c));
}

void serial_echo_init(void)
{
	int comstat, hi, lo, serial_div;
	unsigned char lcr;

	/* read the Divisor Latch */
	comstat = serial_echo_inb(UART_LCR);
	serial_echo_outb(comstat | UART_LCR_DLAB, UART_LCR);
	hi = serial_echo_inb(UART_DLM);
	lo = serial_echo_inb(UART_DLL);
	serial_echo_outb(comstat, UART_LCR);

	/* now do hardwired init */
	lcr = serial_parity | (serial_bits - 5);
	serial_echo_outb(lcr, UART_LCR); /* No parity, 8 data bits, 1 stop */
	serial_div = 115200 / serial_baud_rate;
	serial_echo_outb(0x80|lcr, UART_LCR); /* Access divisor latch */
	serial_echo_outb(serial_div & 0xff, UART_DLL);  /* baud rate divisor */
	serial_echo_outb((serial_div >> 8) & 0xff, UART_DLM);
	serial_echo_outb(lcr, UART_LCR); /* Done with divisor */

	/* Prior to disabling interrupts, read the LSR and RBR
	 * registers */
	comstat = serial_echo_inb(UART_LSR); /* COM? LSR */
	comstat = serial_echo_inb(UART_RX);	/* COM? RBR */
	serial_echo_outb(0x00, UART_IER); /* Disable all interrupts */

        clear_screen_buf();

	return;
}

void serial_echo_print(const char *p)
{
	if (!serial_cons) {
		return;
	}
	/* Now, do each character */
	while (*p) {
		WAIT_FOR_XMITR;

		/* Send the character out. */
		serial_echo_outb(*p, UART_TX);
		if(*p==10) {
			WAIT_FOR_XMITR;
			serial_echo_outb(13, UART_TX);
		}
		p++;
	}
}
