# MemTest86+ multi-arch version

This is a [MemTest86+](http://www.memtest.org/) v5.1 refactored to support multiple hardware architectures.

**DISCLAIMER: USE AT YOUR OWN RISK. DO NOT USE AT ALL ON REAL HARDWARE UNLESS YOU KNOW WHAT YOU ARE DOING.**

This repository includes the RISC-V port for my build of RocketChip for an FPGA board from AliExpress.
This board has a regular DDR2-SODIMM slot, so testing memory can be not only a "model example".

While I was trying to parse FDT when possible instead of hardcoding memory map, there may still exist some "magic numbers".
Especially, it **does not handle reserved memory areas**. I have none on my soft-processor but running such code on real-life
CPU may probably cause some damage.

## Usage

Build it with

    make -f Makefile.arch ARCH=riscv memtest.uboot

In case some tools cannot be found (and this would probably be the case), edit `arch/riscv/config.mk`.

You will get 
* `memtest.bin` -- a raw image to be loaded into memory
* `memtest_shared` -- an ELF shared object for use with GDB
* `memtest.uboot` -- an U-Boot image pretending to be Linux kernel

For my board, boot command looks like this

    run mmcsetup; run fdtsetup; fdt set /chosen bootargs "console=ttyS0"; fatload mmc 0:1 82000000 memtest.uboot; bootm fdt; bootm 82000000 - ${fdtaddr}

For manual startup, place `memtest.bin` anywhere into your board RAM. Set registers as follows:
* `$pc` -- the load address of `memtest.bin`
* `$a0` -- hart id. Not used for now (SMP is NOT supported) but just for compatibility with other boot protocols
* `$a1` -- load address of FDT, ask your bootloader for it :)
  * please note that the command line will be taken from `/chosen/bootargs` variable. Place `console=ttyS0` there, for example

## Other info

See [original README](README) for more details.

License: GPL 2.
