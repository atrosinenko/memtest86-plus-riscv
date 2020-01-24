# Makefile for MemTest86+ multi-arch version
#
# Based on Makefile by Chris Brady

all: clean

ROOT_DIR=$(CURDIR)
ARCH_DIR=$(ROOT_DIR)/arch/$(ARCH)

include $(ARCH_DIR)/config.mk

all: $(ARTIFACTS)

VPATH=$(ARCH_DIR):.

CFLAGS= $(ARCH_CFLAGS) -I$(ARCH_DIR) -I$(ROOT_DIR) -ggdb3 -Wall -O0 -fomit-frame-pointer -fno-builtin \
	-ffreestanding $(SMP_FL) -fno-stack-protector -fPIC

ASM_OBJS = $(subst .S,.o,$(ASM_SOURCES))

OBJS= $(ASM_OBJS) reloc.o main.o test.o init.o lib.o patn.o screen_buffer.o \
	config.o error.o smp.o vmem.o random.o $(ARCH_OBJS)

# Link it statically once so I know I don't have undefined
# symbols and then link it dynamically so I have full
# relocation information
memtest_shared: memtest_shared.lds Makefile $(OBJS)
	$(LD) $(LD_FLAGS) --warn-constructors --warn-common -static -T $< -o $@ $(OBJS)
	$(LD) $(LD_FLAGS) -shared -Bsymbolic -T $< -o $@ $(OBJS)

memtest_shared.bin: memtest_shared
	$(OBJCOPY) -O binary $< $@

%.o: %.S config.h defs.h test.h
	$(CC) -I$(ARCH_DIR) -I$(ROOT_DIR) $(CFLAGS) -traditional $< -c -o $@

reloc.o: reloc.c
	$(CC) $(CFLAGS) -fno-strict-aliasing -c $<

test.o: test.c
	$(CC) $(ARCH_CFLAGS) -I$(ARCH_DIR) -I$(ROOT_DIR) -ggdb3 -Wall -O0 -fPIC -fomit-frame-pointer -fno-builtin -ffreestanding -c $<

random.o: random.c
	$(CC) $(ARCH_CFLAGS) -I$(ARCH_DIR) -I$(ROOT_DIR) -ggdb3 -Wall -O3 -fPIC -fomit-frame-pointer -fno-builtin -ffreestanding -c $<
	
clean:
	rm -f *.o *.s *.iso memtest.bin memtest memtest_shared \
		memtest_shared.bin memtest.iso memtest.bin

