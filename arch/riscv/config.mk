CROSS_PREFIX=riscv64-linux-gnu-

# For some reason, some headers don't work with `lp64`, so `lp64f`
# Don't worry, I'm not going to really use floating point
ARCH_ID  = rv64imf
ABI_ID   = lp64f 
LD_FLAGS = -m elf64lriscv

AS=$(CROSS_PREFIX)as -march=$(ARCH_ID)
CC=$(CROSS_PREFIX)gcc
LD=$(CROSS_PREFIX)ld
OBJCOPY=$(CROSS_PREFIX)objcopy

ARCH_FLAGS= -march=$(ARCH_ID) -mabi=$(ABI_ID)

ARCH_OBJS   = stubs.o arch.o
ASM_SOURCES = head.S

ARTIFACTS = memtest.uboot

memtest.uboot: memtest_shared.bin
	mkimage -A riscv -O linux -T kernel -C none -a 0x80000000 -e 0x80000000 -n memtest -d $< $@
