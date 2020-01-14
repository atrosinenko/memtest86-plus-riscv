ARCH_CFLAGS = -march=i486 -m32

ASM_SOURCES = bootsect.S setup.S head.S
ARCH_OBJS = arch.o controller.o cpuid.o dmi.o linuxbios.o memsize.o pci.o spd.o
