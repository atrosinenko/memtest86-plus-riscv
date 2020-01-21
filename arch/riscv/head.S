	.globl startup_32
startup_32:
# In case we are relocating...
	fence.i
	lla sp, boot_stack_top

	ld t0, initial_load_addr
	bne zero, t0, .skip_first_time_setup # TODO
	lla t0, startup_32
	sd t0, initial_load_addr, t1

# TODO Looks like U-Boot passes uint as a second argument
	li t0, 0xffffffffL
	and a1, a1, t0

	mv s0, a0
	mv s1, a1

# Zero .bss
	lla t1, _bss
	lla t2, _end
.zero_bss:
	sw zero, 0(t1)
	add t1, t1, 4
	blt t1, t2, .zero_bss
.skip_first_time_setup:

# Set trap handler
	lla t1, _trap_entry
	csrw mtvec, t1

# Relocate
	jal _dl_start
# Start via main entry point
	mv a0, s0
	mv a1, s1
	j riscv_entry

.balign 4
_trap_entry:
	csrr a0, mcause
	csrr a1, mepc
	csrr a2, mtval
	jal riscv_trap_entry

.globl halt
halt:
	j halt

.data
.balign 8
.globl initial_load_addr
initial_load_addr:
	. = . + 8
zerobss:    .long   1
.previous
.data
.previous
.bss
.balign 16
boot_stack:
	.globl boot_stack
	. = . + 4096
boot_stack_top:
	.globl boot_stack_top
.previous
