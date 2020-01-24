#define OPTIMIZED 1

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

OPTIMIZED_SNIPPET void movinvr_snippet1(uint32_t *p, uint32_t *pe, int me)
{
	asm __volatile__ (
		"jmp L200\n\t"
		".p2align 4,,7\n\t"
		"L201:\n\t"
		"addl $4,%%edi\n\t"
		"L200:\n\t"
		"pushl %%ecx\n\t" \
		"call rand\n\t"
		"popl %%ecx\n\t" \
		"movl %%eax,(%%edi)\n\t"
		"cmpl %%ebx,%%edi\n\t"
		"jb L201\n\t"
		: : "D" (p), "b" (pe), "c" (me)
		: "eax"
	);
}

OPTIMIZED_SNIPPET void movinv1_snippet1(ulong len, uint32_t *p, uint32_t p1)
{
	asm __volatile__ (
		"rep\n\t" \
		"stosl\n\t"
		: : "c" (len), "D" (p), "a" (p1)
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
