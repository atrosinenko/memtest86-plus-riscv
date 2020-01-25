// These snippets has quite huge register pressure, but...
// * I have lots of them :)
// * I would prefer to start with something straightforward
// * I expect these snippets to be entered and exited quite rarely
//   and instead take quite long time to execute

#define HAS_OPT_ADDR_TST2  1
#define HAS_OPT_MOVINVR    1
#define HAS_OPT_MOVINV1    1
#define HAS_OPT_MOVINV32   1
#define HAS_OPT_MODTST     1
#define HAS_OPT_BLOCK_MOVE 0

OPTIMIZED_SNIPPET void addr_tst2_snippet1(uint32_t *p, uint32_t *pe)
{
	asm __volatile__ (
		".tst2_1_store:\n"
		"sw %1, 0(%1)\n"
		"addi %1, %1, 4\n"
		"bleu %1, %2, .tst2_1_store\n"
		: "=r"(p) // `p` is changed
		: "0" (p), "r" (pe)
	);
}

OPTIMIZED_SNIPPET void addr_tst2_snippet2(uint32_t *p, uint32_t *pe)
{
	asm __volatile__ (
		"mv s0, %0\n" // p
		"mv s1, %1\n" // pe
		"li s2, 0xffffffffu\n"

		".tst2_2_load:\n"
		"lwu a1, 0(s0)\n"
		"and a0, s0, s2\n"
		"beq a0, a1, .tst2_2_ok\n"
		"call ad_err2\n"
		".tst2_2_ok:\n"
		"addi s0, s0, 4\n"
		"bleu s0, s1, .tst2_2_load\n"
		: : "r" (p), "r" (pe) : "a0", "a1", "s0", "s1", "s2"
	);
}


OPTIMIZED_SNIPPET void movinvr_snippet1(uint32_t *p, uint32_t *pe, unsigned me)
{
	asm __volatile__ (
		"mv s0, %0\n" // p
		"mv s1, %1\n" // pe
		"mv s2, %2\n" // me

		".movinvr_1_store:\n"
		"mv a0, s2\n"
		"call memtest_rand\n"
		"sw a0, 0(s0)\n"
		"addi s0, s0, 4\n"
		"bleu s0, s1, .movinvr_1_store\n"
		: : "r" (p), "r" (pe), "r"(me) : "a0", "a1" /* may contain return value */, "s0", "s1", "s2"
	);
}

OPTIMIZED_SNIPPET void movinvr_snippet2(uint32_t *p, uint32_t *pe, uint32_t xorVal, unsigned me)
{
	asm __volatile__ (
		"mv s0, %0\n" // p
		"mv s1, %1\n" // pe
		"mv s2, %2\n" // me
		"mv s3, %3\n" // xorVal
		"li s4, 0xffffffffu\n"

		".movinvr_2_loop:\n"
		"mv a0, s2\n"
		"call memtest_rand\n"
		"and a0, a0, s4\n"
		"xor a1, a0, s3\n"
		"lwu a2, 0(s0)\n"
		// a0 == num
		"xor a0, a0, s4\n"
		"sw  a0, 0(s0)\n"
		"beq a1, a2, .movinvr_2_ok\n"
		"mv a0, s0\n"
		"call error\n"
		".movinvr_2_ok:\n"
		"addi s0, s0, 4\n"
		"bleu s0, s1, .movinvr_2_loop\n"
		: : "r" (p), "r" (pe), "r"(me), "r"(xorVal & 0xffffffffLLu) : "a0", "a1", "a2", "s0", "s1", "s2", "s3", "s4"
	);
}

OPTIMIZED_SNIPPET void movinv1_snippet1(uint32_t *p, uint32_t *pe, uint32_t p1)
{
	asm __volatile__(
		".movinv1_1_store:\n"
		"sw %3, 0(%1)\n"
		"addi %1, %1, 4\n"
		"bleu %1, %2, .movinv1_1_store\n"
		: "=r"(p) // `p` is changed
		: "0"(p), "r"(pe), "r"(p1)
	);
}


OPTIMIZED_SNIPPET void movinv1_snippet2(uint32_t *p, uint32_t *pe, uint32_t p1, uint32_t p2)
{
	asm __volatile__(
		"mv s0, %0\n" // p
		"mv s1, %1\n" // pe
		"mv s2, %2\n" // p1
		"mv s3, %3\n" // p2

		".movinv1_2_load:\n"
		"lwu a2, 0(s0)\n"
		"beq a2, s2, .movinv1_2_ok\n"
		"mv a0, s0\n"
		"mv a1, s2\n"
		"call error\n"
		".movinv1_2_ok:\n"
		"sw s3, 0(s0)\n"
		"addi s0, s0, 4\n"
		"bleu s0, s1, .movinv1_2_load\n"
		: : "r"(p), "r"(pe), "r"(p1 & 0xffffffffLLu), "r"(p2 & 0xffffffffLLu) : "a0", "a1", "a2", "s0", "s1", "s2", "s3"
	);
}

OPTIMIZED_SNIPPET void movinv1_snippet3(uint32_t *p, uint32_t *pe, uint32_t p1, uint32_t p2)
{
	asm __volatile__(
		"mv s0, %0\n" // p
		"mv s1, %1\n" // pe
		"mv s2, %2\n" // p1
		"mv s3, %3\n" // p2

		".movinv1_3_load:\n"
		"lwu a2, 0(s0)\n"
		"beq a2, s3, .movinv1_3_ok\n"
		"mv a0, s0\n"
		"mv a1, s3\n"
		"call error\n"
		".movinv1_3_ok:\n"
		"sw s2, 0(s0)\n"
		"addi s0, s0, -4\n"
		"bgeu s0, s1, .movinv1_3_load\n"
		: : "r"(p), "r"(pe), "r"(p1 & 0xffffffffLLu), "r"(p2 & 0xffffffffLLu) : "a0", "a1", "a2", "s0", "s1", "s2", "s3"
	);
}

OPTIMIZED_SNIPPET void movinv32_snippet1(
	int *p_k, uint32_t *p_pat, // inputs-outputs
	uint32_t *p, uint32_t *pe, uint32_t sval, uint32_t lb // inputs only
)
{
	uint32_t k = *p_k;
	uint32_t pat = *p_pat;

	asm __volatile__(
		"li t0, 32\n"

		".movinv32_1_store:\n"
		"sw %1, 0(%2)\n"
		"addi %0, %0, 1\n"
		"bge %0, t0, .movinv32_1_reset_pat\n"
		"slli %1, %1, 1\n"
		"or   %1, %1, %7\n"
		"j .movinv32_1_continue\n"
		".movinv32_1_reset_pat:\n"
		"mv %1, %8\n"
		"li %0, 0\n"
		".movinv32_1_continue:\n"
		"addi %2, %2, 4\n"
		"bleu %2, %6, .movinv32_1_store\n"
		: "=r"(k), "=r"(pat), "=r"(p) // `p` is changed
		: "0"(k), "1"(pat), "2"(p),
		  /* %6 */ "r"(pe), "r"(sval), "r"(lb)
		: "t0"
	);

	*p_k = k;
	*p_pat = pat;
}

OPTIMIZED_SNIPPET void movinv32_snippet2(
	int *p_k, uint32_t *p_pat, // inputs-outputs
	uint32_t *p, uint32_t *pe, uint32_t sval, uint32_t lb // inputs only
)
{
	uint32_t k = *p_k;
	uint32_t pat = *p_pat;

	asm __volatile__(
		"mv s0, %2\n" // k
		"mv s1, %3\n" // pat
		"mv s2, %4\n" // p
		"mv s3, %5\n" // pe
		"mv s4, %6\n" // sval
		"mv s5, %7\n" // lb
		"li t0, 31\n"
		"li t1, 0xffffffffu\n"

		".movinv32_2_load:\n"
		"and s1, s1, t1\n"
		"lwu a2, 0(s2)\n"
		"beq a2, s1, .movinv32_2_ok\n"
		"mv a0, s2\n"
		"mv a1, s1\n"
		"call error\n"
		".movinv32_2_ok:\n"
		"xor a0, s1, t1\n" // a0 contains ~pat as u32
		"sw a0, 0(s2)\n"
		"bge s0, t0, .movinv32_2_reset_pat\n"
		"slli s1, s1, 1\n"
		"or   s1, s1, s4\n"
		"addi s0, s0, 1\n"
		"j .movinv32_2_continue\n"
		".movinv32_2_reset_pat:\n"
		"mv s1, s5\n"
		"li s0, 0\n"
		".movinv32_2_continue:\n"
		"addi s2, s2, 4\n"
		"bleu s2, s3, .movinv32_2_load\n"

		"mv %0, s0\n"
		"mv %1, s1\n"

		: "=r"(k), "=r"(pat)
		: /* %2 */ "r"(k), "r"(pat), "r"(p),
		  /* %5 */ "r"(pe), "r"(sval), "r"(lb)
		: "a0", "a1", "a2", "s0", "s1", "s2", "s3", "s4", "s5", "t0", "t1"
	);

	*p_k = k;
	*p_pat = pat;
}

OPTIMIZED_SNIPPET void movinv32_snippet3(
	int *p_k, uint32_t *p_pat, // inputs-outputs
	uint32_t *p, uint32_t *pe, uint32_t p3, uint32_t hb // inputs only
)
{
	uint32_t k = *p_k;
	uint32_t pat = *p_pat;

	asm __volatile__(
		"mv s0, %2\n" // k
		"mv s1, %3\n" // pat
		"mv s2, %4\n" // p
		"mv s3, %5\n" // pe
		"mv s4, %6\n" // p3
		"mv s5, %7\n" // hb
		"li t0, 1\n"
		"li t1, 0xffffffffu\n"

		".movinv32_3_load:\n"
		"and s1, s1, t1\n"
		"xor a1, s1, t1\n"
		"lwu a2, 0(s2)\n"
		"beq a1, a2, .movinv32_3_ok\n"
		"mv a0, s2\n"
		"call error\n"
		".movinv32_3_ok:\n"
		"sw s1, 0(s2)\n"
		"ble s0, t0, .movinv32_3_reset_pat\n"
		"srli s1, s1, 1\n"
		"or   s1, s1, s4\n"
		"addi s0, s0, -1\n"
		"j .movinv32_3_continue\n"
		".movinv32_3_reset_pat:\n"
		"mv s1, s5\n"
		"li s0, 32\n"
		".movinv32_3_continue:\n"
		"beq s2, zero, .movinv32_3_break\n"
		"addi s2, s2, -4\n"
		"bgeu s2, s3, .movinv32_3_load\n"
		".movinv32_3_break:"

		"mv %0, s0\n"
		"mv %1, s1\n"

		: "=r"(k), "=r"(pat)
		: /* %2 */ "r"(k), "r"(pat), "r"(p),
		  /* %5 */ "r"(pe), "r"(p3), "r"(hb)
		: "a0", "a1", "a2", "s0", "s1", "s2", "s3", "s4", "s5", "t0", "t1"
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

	asm __volatile__(
		".modtst_1_store:\n"
		"sw %3, 0(%1)\n"
		"addi %1, %1, %4\n"
		"bleu %1, %2, .modtst_1_store\n"
		: "=r"(p)  // `p` is changed
		: "0"(p), "r"(pe), "r"(p1), "I"(MOD_SZ)
	);

	*p_p = p;
}

OPTIMIZED_SNIPPET void modtst_snippet2(
	int *p_k, // input-output
	uint32_t *p, uint32_t *pe, uint32_t p2, int offset // inputs only
)
{
	int k = *p_k;

	asm __volatile__(
		".modtst_2_loop:\n"
		"beq %2, %6, .modtst_2_nostore\n"
		"sw %5, 0(%3)\n"
		".modtst_2_nostore:\n"
		"addi %0, %0, 1\n"
		"ble %0, %7, .modtst_2_continue\n"
		"mv %2, zero\n"
		".modtst_2_continue:\n"
		"addi %3, %3, 4\n"
		"bleu %3, %4, .modtst_2_loop\n"
		: "=r"(k), "=r"(p) // `p` is changed
		: /* %2 */ "0"(k),  "1"(p), /* %4 */ "r"(pe), "r"(p2), /* %6 */ "r"(offset), "r"(MOD_SZ - 1)
	);
	*p_k = k;
}


OPTIMIZED_SNIPPET void modtst_snippet3(
	uint32_t **p_p, // input-output
	uint32_t *pe, uint32_t p1 // inputs only
)
{
	uint32_t *p = *p_p;
	asm __volatile__(
		"mv s0, %1\n" // p
		"mv s1, %2\n" // pe
		"mv s2, %3\n" // p1

		".modtst_3_load:\n"
		"lwu a2, 0(s0)\n"
		"beq a2, s2, .modtst_3_ok\n"
		"mv a0, s0\n"
		"mv a1, s2\n"
		"call error\n"
		".modtst_3_ok:\n"
		"addi s0, s0, %4\n"
		"bltu s0, s1, .modtst_3_load\n"

		"mv %0, s0\n"
		: "=r"(p)
		: "0"(p), "r"(pe), "r"(p1 & 0xffffffffLLu), "I"(MOD_SZ * 4)
		: "a0", "a1", "a2", "s0", "s1", "s2"
	);
	*p_p = p;
}
