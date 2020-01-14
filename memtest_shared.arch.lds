/* For historycal reasons... */
ENTRY(startup_32); 

SECTIONS {
	.text : {
		_start = .;
		*(.text)
		*(.text.*)
		*(.plt)
		_etext = . ;
	}
	.rodata : {
		*(.rodata)
		*(.rodata.*)
	}
	.dynsym     : { *(.dynsym) }
	.dynstr     : { *(.dynstr) }
	.hash       : { *(.hash) }
	.gnu.hash   : { *(.gnu.hash) }

	. = ALIGN(16);
	_dynamic = .;
	.dynamic    : { *(.dynamic) }

	.rela.text    : { *(.rela.text   .rela.text.*) }
	.rela.rodata  : { *(.rela.rodata .rela.rodata.*) }
	.rela.data    : { *(.rela.data   .rela.data.*) }
	.rela.got     : { *(.rela.got    .rela.got.*) }
	.rela.plt     : { *(.rela.plt    .rela.plt.*) }
	
	.rel.text    : { *(.rel.text   .rel.text.*) }
	.rel.rodata  : { *(.rel.rodata .rel.rodata.*) }
	.rel.data    : { *(.rel.data   .rel.data.*) }
	.rel.got     : { *(.rel.got    .rel.got.*) }
	.rel.plt     : { *(.rel.plt    .rel.plt.*) }

	. = ALIGN(4);
	.data : {
		 _data = .; 
		*(.data) 
		*(.data.*) 
	}
	.got : {
		*(.got.plt)
		*(.got)
		_edata = . ;
	}
	. = ALIGN(4);
	.bss : { 
		_bss = .;
		*(.dynbss)
		*(.bss) 
		*(.bss.*) 
		*(COMMON) 
		/* _end must be at least 256 byte aligned */
		. = ALIGN(256); 
		_end = .;
	}
/* Comment to pass through debug sections, etc. */
/*	/DISCARD/ : { *(*) }	*/
}
