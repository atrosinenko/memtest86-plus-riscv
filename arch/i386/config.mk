ARCH_CFLAGS = -march=i486 -m32
LD_FLAGS = -m elf_i386

OBJCOPY = objcopy

ASM_SOURCES = head.S
ARCH_OBJS = arch.o arch-init.o arch-lib.o arch-smp.o arch-vmem.o controller.o cpuid.o dmi.o linuxbios.o memsize.o pci.o spd.o

ARTIFACTS = memtest memtest.bin

memtest: memtest_shared.bin arch/i386/memtest.lds
	$(LD) -s -T arch/i386/memtest.lds -b binary $< -o $@

memtest.bin: memtest_shared.bin bootsect.o setup.o arch/i386/memtest.bin.lds
	$(LD) -T arch/i386/memtest.bin.lds bootsect.o setup.o -b binary $< -o $@
