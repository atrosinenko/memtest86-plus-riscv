One of the goals when making MemTest86+ multi-arch (not achieved yet) is to have all platform-specific code inside the `arch/<arch-name>/` subdirectory.
Ideally, there should not be any `#if defined(__i386__)` or so in the cross-platform part of the code (i.e., outside of `arch/`).

## Source code layout

Due to MemTest86+ sources being mix of C and Assembly and some headers are included into both, some care should be taken.

Generally, the layout of subdirectories of `arch/` is arbitrary with the exception of some files that should always exist:
* `config.mk` is a file being included inside the main `Makefile.arch`. It has to contain some variable definitions, see `Makefile.arch` and neighbor `config.mk`s
* `arch.h` is a header that is included into almost any C source
* `defs.h` contains Assembly-safe **definitions**. They are expected to be `#include`d as early as possible.
* `reloc.inc.c` is a snippet being included into `reloc.c`
* `test.inc.c` is included inside the `test.c` and should contain support for test optimizations (usually, just Assembly listings), if any

Apart from these, there should usually exist one or more Assembly (*.S) files for setting up CPU context properly and one or more C source files for high-level platform-specific support. They are usually listed in `ASM_SOURCES` and `ARCH_OBJS` variables in `config.mk` (you would probably want to specify Assembly files in a proper order in the `ASM_SOURCES` variable).
