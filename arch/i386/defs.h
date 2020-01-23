/* defs.h - MemTest-86 Version 3.3
 * assembler/compiler definitions
 *
 * Released under version 2 of the Gnu Public License.
 * By Chris Brady
 */ 

#define HAS_SMP 1
#define HAS_FLAT_MEM 0

#define SETUPSECS	4		/* Number of setup sectors */

/*
 * Caution!! There is magic in the build process.  Read
 * README.build-process before you change anything.  
 * Unlike earlier versions all of the settings are in defs.h
 * so the build process should be more robust.
 */
#define LOW_TEST_ADR	0x00010000		/* Final adrs for test code */

#define BOOTSEG		0x07c0			/* Segment adrs for inital boot */
#define INITSEG		0x9000			/* Segment adrs for relocated boot */
#define SETUPSEG	(INITSEG+0x20)		/* Segment adrs for relocated setup */
#define TSTLOAD		0x1000			/* Segment adrs for load of test */

#define KERNEL_CS	0x10			/* 32 bit segment adrs for code */
#define KERNEL_DS	0x18			/* 32 bit segment adrs for data */
#define REAL_CS		0x20			/* 16 bit segment adrs for code */
#define REAL_DS		0x28			/* 16 bit segment adrs for data */


#define E88     0x00
#define E801    0x04
#define E820NR  0x08           /* # entries in E820MAP */
#define E820MAP 0x0c           /* our map */
#define E820MAX 127            /* number of entries in E820MAP */
#define E820ENTRY_SIZE 20
#define MEMINFO_SIZE (E820MAP + E820MAX * E820ENTRY_SIZE)

#define E820_RAM        1
#define E820_RESERVED   2
#define E820_ACPI       3 /* usable as RAM once ACPI tables have been read */
#define E820_NVS        4


#define BARRIER_ADDR (0x9ff00)

#define RES_START	0xa0000
#define RES_END		0x100000

#define DMI_SEARCH_START  0x0000F000
#define DMI_SEARCH_LENGTH 0x000F0FFF

#define MAX_DMI_MEMDEVS 16


#define X86_FEATURE_PAE		(0*32+ 6) /* Physical Address Extensions */
#define MAX_MEM_SEGMENTS E820MAX
