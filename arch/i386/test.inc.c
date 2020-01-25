#define HAS_OPT_ADDR_TST2  1
#define HAS_OPT_MOVINVR    1
#define HAS_OPT_MOVINV1    1
#define HAS_OPT_MOVINV32   1
#define HAS_OPT_MODTST     1
#define HAS_OPT_BLOCK_MOVE 1

OPTIMIZED_SNIPPET void addr_tst2_snippet1(uint32_t *p, uint32_t *pe)
{
	asm __volatile__ (
		"jmp L91\n\t"
		".p2align 4,,7\n\t"
		"L90:\n\t"
		"addl $4,%%edi\n\t"
		"L91:\n\t"
		"movl %%edi,(%%edi)\n\t"
		"cmpl %%edx,%%edi\n\t"
		"jb L90\n\t"
		: : "D" (p), "d" (pe)
	);
}

OPTIMIZED_SNIPPET void addr_tst2_snippet2(uint32_t *p, uint32_t *pe)
{
	asm __volatile__ (
		"jmp L95\n\t"
		".p2align 4,,7\n\t"
		"L99:\n\t"
		"addl $4,%%edi\n\t"
		"L95:\n\t"
		"movl (%%edi),%%ecx\n\t"
		"cmpl %%edi,%%ecx\n\t"
		"jne L97\n\t"
		"L96:\n\t"
		"cmpl %%edx,%%edi\n\t"
		"jb L99\n\t"
		"jmp L98\n\t"

		"L97:\n\t"
		"pushl %%edx\n\t"
		"pushl %%ecx\n\t"
		"pushl %%edi\n\t"
		"call ad_err2\n\t"
		"popl %%edi\n\t"
		"popl %%ecx\n\t"
		"popl %%edx\n\t"
		"jmp L96\n\t"

		"L98:\n\t"
		: : "D" (p), "d" (pe)
		: "ecx"
	);
}

OPTIMIZED_SNIPPET void movinvr_snippet1(uint32_t *p, uint32_t *pe, int me)
{
	asm __volatile__ (
		"jmp L200\n\t"
		".p2align 4,,7\n\t"
		"L201:\n\t"
		"addl $4,%%edi\n\t"
		"L200:\n\t"
		"pushl %%ecx\n\t" \
		"call memtest_rand\n\t"
		"popl %%ecx\n\t" \
		"movl %%eax,(%%edi)\n\t"
		"cmpl %%ebx,%%edi\n\t"
		"jb L201\n\t"
		: : "D" (p), "b" (pe), "c" (me)
		: "eax"
	);
}

OPTIMIZED_SNIPPET void movinvr_snippet2(uint32_t *p, uint32_t *pe, int i, int me)
{
	uint32_t xorVal;
	if (i) {
		xorVal = 0xffffffff;
	} else {
		xorVal = 0;
	}
	asm __volatile__ (
		"pushl %%ebp\n\t"

		// Skip first increment
		"jmp L26\n\t"
		".p2align 4,,7\n\t"

		// increment 4 bytes (32-bits)
		"L27:\n\t"
		"addl $4,%%edi\n\t"

		// Check this byte
		"L26:\n\t"

		// Get next random number, pass in me(edx), random value returned in num(eax)
		// num = memtest_rand(me);
		// cdecl call maintains all registers except eax, ecx, and edx
		// We maintain edx with a push and pop here using it also as an input
		// we don't need the current eax value and want it to change to the return value
		// we overwrite ecx shortly after this discarding its current value
		"pushl %%edx\n\t" // Push function inputs onto stack
		"call memtest_rand\n\t"
		"popl %%edx\n\t" // Remove function inputs from stack

		// XOR the random number with xorVal(ebx), which is either 0xffffffff or 0 depending on the outer loop
		// if (i) { num = ~num; }
		"xorl %%ebx,%%eax\n\t"

		// Move the current value of the current position p(edi) into bad(ecx)
		// (bad=*p)
		"movl (%%edi),%%ecx\n\t"

		// Compare bad(ecx) to num(eax)
		"cmpl %%eax,%%ecx\n\t"

		// If not equal jump the error case
		"jne L23\n\t"

		// Set a new value or not num(eax) at the current position p(edi)
		// *p = ~num;
		"L25:\n\t"
		"movl $0xffffffff,%%ebp\n\t"
		"xorl %%ebp,%%eax\n\t"
		"movl %%eax,(%%edi)\n\t"

		// Loop until current position p(edi) equals the end position pe(esi)
		"cmpl %%esi,%%edi\n\t"
		"jb L27\n\t"
		"jmp L24\n"

		// Error case
		"L23:\n\t"
		// Must manually maintain eax, ecx, and edx as part of cdecl call convention
		"pushl %%edx\n\t"
		"pushl %%ecx\n\t" // Next three pushes are functions input
		"pushl %%eax\n\t"
		"pushl %%edi\n\t"
		"call error\n\t"
		"popl %%edi\n\t" // Remove function inputs from stack and restore register values
		"popl %%eax\n\t"
		"popl %%ecx\n\t"
		"popl %%edx\n\t"
		"jmp L25\n"

		"L24:\n\t"
		"popl %%ebp\n\t"
		:: "D" (p), "S" (pe), "b" (xorVal),
				"d" (me)
		: "eax", "ecx"
	);
}

OPTIMIZED_SNIPPET void movinv1_snippet1(ulong len, uint32_t *p, uint32_t *pe, uint32_t p1)
{
	(void)pe;

	asm __volatile__ (
		"rep\n\t" \
		"stosl\n\t"
		: : "c" (len), "D" (p), "a" (p1)
	);
}

OPTIMIZED_SNIPPET void movinv1_snippet2(ulong len, uint32_t *p, uint32_t *pe, uint32_t p1, uint32_t p2)
{
	(void)len;

	asm __volatile__ (
		"jmp L2\n\t" \
		".p2align 4,,7\n\t" \
		"L0:\n\t" \
		"addl $4,%%edi\n\t" \
		"L2:\n\t" \
		"movl (%%edi),%%ecx\n\t" \
		"cmpl %%eax,%%ecx\n\t" \
		"jne L3\n\t" \
		"L5:\n\t" \
		"movl %%ebx,(%%edi)\n\t" \
		"cmpl %%edx,%%edi\n\t" \
		"jb L0\n\t" \
		"jmp L4\n" \

		"L3:\n\t" \
		"pushl %%edx\n\t" \
		"pushl %%ebx\n\t" \
		"pushl %%ecx\n\t" \
		"pushl %%eax\n\t" \
		"pushl %%edi\n\t" \
		"call error\n\t" \
		"popl %%edi\n\t" \
		"popl %%eax\n\t" \
		"popl %%ecx\n\t" \
		"popl %%ebx\n\t" \
		"popl %%edx\n\t" \
		"jmp L5\n" \

		"L4:\n\t" \
		:: "a" (p1), "D" (p), "d" (pe), "b" (p2)
		: "ecx"
	);
}

OPTIMIZED_SNIPPET void movinv1_snippet3(ulong len, uint32_t *p, uint32_t *pe, uint32_t p1, uint32_t p2)
{
	(void)len;

	asm __volatile__ (
		"jmp L9\n\t"
		".p2align 4,,7\n\t"
		"L11:\n\t"
		"subl $4, %%edi\n\t"
		"L9:\n\t"
		"movl (%%edi),%%ecx\n\t"
		"cmpl %%ebx,%%ecx\n\t"
		"jne L6\n\t"
		"L10:\n\t"
		"movl %%eax,(%%edi)\n\t"
		"cmpl %%edi, %%edx\n\t"
		"jne L11\n\t"
		"jmp L7\n\t"

		"L6:\n\t"
		"pushl %%edx\n\t"
		"pushl %%eax\n\t"
		"pushl %%ecx\n\t"
		"pushl %%ebx\n\t"
		"pushl %%edi\n\t"
		"call error\n\t"
		"popl %%edi\n\t"
		"popl %%ebx\n\t"
		"popl %%ecx\n\t"
		"popl %%eax\n\t"
		"popl %%edx\n\t"
		"jmp L10\n"

		"L7:\n\t"
		:: "a" (p1), "D" (p), "d" (pe), "b" (p2)
		: "ecx"
	);
}

OPTIMIZED_SNIPPET void movinv32_snippet1(
	int *p_k, uint32_t *p_pat, // inputs-outputs
	uint32_t *p, uint32_t *pe, int sval, uint32_t lb // inputs only
)
{
	int k = *p_k;
	uint32_t pat = *p_pat;
	asm __volatile__ (
		"jmp L20\n\t"
		".p2align 4,,7\n\t"
		"L923:\n\t"
		"addl $4,%%edi\n\t"
		"L20:\n\t"
		"movl %%ecx,(%%edi)\n\t"
		"addl $1,%%ebx\n\t"
		"cmpl $32,%%ebx\n\t"
		"jne L21\n\t"
		"movl %%esi,%%ecx\n\t"
		"xorl %%ebx,%%ebx\n\t"
		"jmp L22\n"
		"L21:\n\t"
		"shll $1,%%ecx\n\t"
		"orl %%eax,%%ecx\n\t"
		"L22:\n\t"
		"cmpl %%edx,%%edi\n\t"
		"jb L923\n\t"
		: "=b" (k), "=c" (pat)
		: "D" (p),"d" (pe),"b" (k),"c" (pat),
			"a" (sval), "S" (lb)
	);
	*p_k = k;
	*p_pat = pat;
}

OPTIMIZED_SNIPPET void movinv32_snippet2(
	int *p_k, uint32_t *p_pat, // inputs-outputs
	uint32_t *p, uint32_t *pe, int sval, uint32_t lb // inputs only
)
{
	int k = *p_k;
	uint32_t pat = *p_pat;
	asm __volatile__ (
		"pushl %%ebp\n\t"
		"jmp L30\n\t"
		".p2align 4,,7\n\t"
		"L930:\n\t"
		"addl $4,%%edi\n\t"
		"L30:\n\t"
		"movl (%%edi),%%ebp\n\t"
		"cmpl %%ecx,%%ebp\n\t"
		"jne L34\n\t"

		"L35:\n\t"
		"notl %%ecx\n\t"
		"movl %%ecx,(%%edi)\n\t"
		"notl %%ecx\n\t"
		"incl %%ebx\n\t"
		"cmpl $32,%%ebx\n\t"
		"jne L31\n\t"
		"movl %%esi,%%ecx\n\t"
		"xorl %%ebx,%%ebx\n\t"
		"jmp L32\n"
		"L31:\n\t"
		"shll $1,%%ecx\n\t"
		"orl %%eax,%%ecx\n\t"
		"L32:\n\t"
		"cmpl %%edx,%%edi\n\t"
		"jb L930\n\t"
		"jmp L33\n\t"

		"L34:\n\t" \
		"pushl %%esi\n\t"
		"pushl %%eax\n\t"
		"pushl %%ebx\n\t"
		"pushl %%edx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%ecx\n\t"
		"pushl %%edi\n\t"
		"call error\n\t"
		"popl %%edi\n\t"
		"popl %%ecx\n\t"
		"popl %%ebp\n\t"
		"popl %%edx\n\t"
		"popl %%ebx\n\t"
		"popl %%eax\n\t"
		"popl %%esi\n\t"
		"jmp L35\n"

		"L33:\n\t"
		"popl %%ebp\n\t"
		: "=b" (k),"=c" (pat)
		: "D" (p),"d" (pe),"b" (k),"c" (pat),
			"a" (sval), "S" (lb)
	);
	*p_k = k;
	*p_pat = pat;
}

OPTIMIZED_SNIPPET void movinv32_snippet3(
	int *p_k, uint32_t *p_pat, // inputs-outputs
	uint32_t *p, uint32_t *pe, uint32_t p3, uint32_t hb // inputs only
)
{
	int k = *p_k;
	uint32_t pat = *p_pat;

	asm __volatile__ (
		"pushl %%ebp\n\t"
		"jmp L40\n\t"
		".p2align 4,,7\n\t"
		"L49:\n\t"
		"subl $4,%%edi\n\t"
		"L40:\n\t"
		"movl (%%edi),%%ebp\n\t"
		"notl %%ecx\n\t"
		"cmpl %%ecx,%%ebp\n\t"
		"jne L44\n\t"

		"L45:\n\t"
		"notl %%ecx\n\t"
		"movl %%ecx,(%%edi)\n\t"
		"decl %%ebx\n\t"
		"cmpl $0,%%ebx\n\t"
		"jg L41\n\t"
		"movl %%esi,%%ecx\n\t"
		"movl $32,%%ebx\n\t"
		"jmp L42\n"
		"L41:\n\t"
		"shrl $1,%%ecx\n\t"
		"orl %%eax,%%ecx\n\t"
		"L42:\n\t"
		"cmpl %%edx,%%edi\n\t"
		"ja L49\n\t"
		"jmp L43\n\t"

		"L44:\n\t" \
		"pushl %%esi\n\t"
		"pushl %%eax\n\t"
		"pushl %%ebx\n\t"
		"pushl %%edx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%ecx\n\t"
		"pushl %%edi\n\t"
		"call error\n\t"
		"popl %%edi\n\t"
		"popl %%ecx\n\t"
		"popl %%ebp\n\t"
		"popl %%edx\n\t"
		"popl %%ebx\n\t"
		"popl %%eax\n\t"
		"popl %%esi\n\t"
		"jmp L45\n"

		"L43:\n\t"
		"popl %%ebp\n\t"
		: "=b" (k), "=c" (pat)
		: "D" (p),"d" (pe),"b" (k),"c" (pat),
			"a" (p3), "S" (hb)
	);

	*p_k = k;
	*p_pat = pat;
}

OPTIMIZED_SNIPPET void modtst_snippet1(
	uint32_t **p_p, // input-output
	uint32_t *pe, uint32_t p1 // inputs only
)
{
	uint32_t *p = *p_p;
	asm __volatile__ (
		"jmp L60\n\t" \
		".p2align 4,,7\n\t" \

		"L60:\n\t" \
		"movl %%eax,(%%edi)\n\t" \
		"addl $80,%%edi\n\t" \
		"cmpl %%edx,%%edi\n\t" \
		"jb L60\n\t" \
		: "=D" (p)
		: "D" (p), "d" (pe), "a" (p1)
	);
	*p_p = p;
}


OPTIMIZED_SNIPPET void modtst_snippet2(
	int *p_k, // input-output
	uint32_t *p, uint32_t *pe, uint32_t p2, int offset // inputs only
)
{
	int k = *p_k;
	asm __volatile__ (
		"jmp L50\n\t" \
		".p2align 4,,7\n\t" \

		"L54:\n\t" \
		"addl $4,%%edi\n\t" \
		"L50:\n\t" \
		"cmpl %%ebx,%%ecx\n\t" \
		"je L52\n\t" \
			"movl %%eax,(%%edi)\n\t" \
		"L52:\n\t" \
		"incl %%ebx\n\t" \
		"cmpl $19,%%ebx\n\t" \
		"jle L53\n\t" \
			"xorl %%ebx,%%ebx\n\t" \
		"L53:\n\t" \
		"cmpl %%edx,%%edi\n\t" \
		"jb L54\n\t" \
		: "=b" (k)
		: "D" (p), "d" (pe), "a" (p2),
			"b" (k), "c" (offset)
	);
	*p_k = k;
}

OPTIMIZED_SNIPPET void modtst_snippet3(
	uint32_t **p_p, // input-output
	uint32_t *pe, uint32_t p1 // inputs only
)
{
	uint32_t *p = *p_p;

	asm __volatile__ (
		"jmp L70\n\t" \
		".p2align 4,,7\n\t" \

		"L70:\n\t" \
		"movl (%%edi),%%ecx\n\t" \
		"cmpl %%eax,%%ecx\n\t" \
		"jne L71\n\t" \
		"L72:\n\t" \
		"addl $80,%%edi\n\t" \
		"cmpl %%edx,%%edi\n\t" \
		"jb L70\n\t" \
		"jmp L73\n\t" \

		"L71:\n\t" \
		"pushl %%edx\n\t"
		"pushl %%ecx\n\t"
		"pushl %%eax\n\t"
		"pushl %%edi\n\t"
		"call error\n\t"
		"popl %%edi\n\t"
		"popl %%eax\n\t"
		"popl %%ecx\n\t"
		"popl %%edx\n\t"
		"jmp L72\n"

		"L73:\n\t" \
		: "=D" (p)
		: "D" (p), "d" (pe), "a" (p1)
		: "ecx"
	);

	*p_p = p;
}

OPTIMIZED_SNIPPET void block_move_snippet1(uint32_t **p_p, ulong len)
{
	uint32_t *p = *p_p;
	asm __volatile__ (
		"jmp L100\n\t"

		".p2align 4,,7\n\t"
		"L100:\n\t"

		// First loop eax is 0x00000001, edx is 0xfffffffe
		"movl %%eax, %%edx\n\t"
		"notl %%edx\n\t"

		// Set a block of 64-bytes	// First loop DWORDS are
		"movl %%eax,0(%%edi)\n\t"	// 0x00000001
		"movl %%eax,4(%%edi)\n\t"	// 0x00000001
		"movl %%eax,8(%%edi)\n\t"	// 0x00000001
		"movl %%eax,12(%%edi)\n\t"	// 0x00000001
		"movl %%edx,16(%%edi)\n\t"	// 0xfffffffe
		"movl %%edx,20(%%edi)\n\t"	// 0xfffffffe
		"movl %%eax,24(%%edi)\n\t"	// 0x00000001
		"movl %%eax,28(%%edi)\n\t"	// 0x00000001
		"movl %%eax,32(%%edi)\n\t"	// 0x00000001
		"movl %%eax,36(%%edi)\n\t"	// 0x00000001
		"movl %%edx,40(%%edi)\n\t"	// 0xfffffffe
		"movl %%edx,44(%%edi)\n\t"	// 0xfffffffe
		"movl %%eax,48(%%edi)\n\t"	// 0x00000001
		"movl %%eax,52(%%edi)\n\t"	// 0x00000001
		"movl %%edx,56(%%edi)\n\t"	// 0xfffffffe
		"movl %%edx,60(%%edi)\n\t"	// 0xfffffffe

		// rotate left with carry,
		// second loop eax is		 0x00000002
		// second loop edx is (~eax) 0xfffffffd
		"rcll $1, %%eax\n\t"

		// Move current position forward 64-bytes (to start of next block)
		"leal 64(%%edi), %%edi\n\t"

		// Loop until end
		"decl %%ecx\n\t"
		"jnz  L100\n\t"

		: "=D" (p)
		: "D" (p), "c" (len), "a" (1)
		: "edx"
	);
	*p_p = p;
}

OPTIMIZED_SNIPPET void block_move_snippet2(uint32_t *p, ulong pp, ulong len)
{
	asm __volatile__ (
		"cld\n"
		"jmp L110\n\t"

		".p2align 4,,7\n\t"
		"L110:\n\t"

		//
		// At the end of all this
		// - the second half equals the inital value of the first half
		// - the first half is right shifted 32-bytes (with wrapping)
		//

		// Move first half to second half
		"movl %1,%%edi\n\t" // Destionation, pp (mid point)
		"movl %0,%%esi\n\t" // Source, p (start point)
		"movl %2,%%ecx\n\t" // Length, len (size of a half in DWORDS)
		"rep\n\t"
		"movsl\n\t"

		// Move the second half, less the last 32-bytes. To the first half, offset plus 32-bytes
		"movl %0,%%edi\n\t"
		"addl $32,%%edi\n\t"	// Destination, p(start-point) plus 32 bytes
		"movl %1,%%esi\n\t"		// Source, pp(mid-point)
		"movl %2,%%ecx\n\t"
		"subl $8,%%ecx\n\t"		// Length, len(size of a half in DWORDS) minus 8 DWORDS (32 bytes)
		"rep\n\t"
		"movsl\n\t"

		// Move last 8 DWORDS (32-bytes) of the second half to the start of the first half
		"movl %0,%%edi\n\t"		// Destination, p(start-point)
								// Source, 8 DWORDS from the end of the second half, left over by the last rep/movsl
		"movl $8,%%ecx\n\t"		// Length, 8 DWORDS (32-bytes)
		"rep\n\t"
		"movsl\n\t"

		:: "g" (p), "g" (pp), "g" (len)
		: "edi", "esi", "ecx"
	);
}


OPTIMIZED_SNIPPET void block_move_snippet3(uint32_t **p_p, uint32_t *pe)
{
	uint32_t *p = *p_p;

	asm __volatile__ (
		"jmp L120\n\t"

		".p2align 4,,7\n\t"
		"L124:\n\t"
		"addl $8,%%edi\n\t" // Next QWORD
		"L120:\n\t"

		// Compare adjacent DWORDS
		"movl (%%edi),%%ecx\n\t"
		"cmpl 4(%%edi),%%ecx\n\t"
		"jnz L121\n\t" // Print error if they don't match

		// Loop until end of block
		"L122:\n\t"
		"cmpl %%edx,%%edi\n\t"
		"jb L124\n"
		"jmp L123\n\t"

		"L121:\n\t"
		// eax not used so we don't need to save it as per cdecl
		// ecx is used but not restored, however we don't need it's value anymore after this point
		"pushl %%edx\n\t"
		"pushl 4(%%edi)\n\t"
		"pushl %%ecx\n\t"
		"pushl %%edi\n\t"
		"call error\n\t"
		"popl %%edi\n\t"
		"addl $8,%%esp\n\t"
		"popl %%edx\n\t"
		"jmp L122\n"
		"L123:\n\t"
		: "=D" (p)
		: "D" (p), "d" (pe)
		: "ecx"
	);

	*p_p = p;
}
