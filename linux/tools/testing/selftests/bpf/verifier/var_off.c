{
	"variable-offset ctx access",
	.insns = {
	/* Get an unknown value */
	BPF_LDX_MEM(BPF_W, BPF_REG_2, BPF_REG_1, 0),
	/* Make it small and 4-byte aligned */
	BPF_ALU64_IMM(BPF_AND, BPF_REG_2, 4),
	/* add it to skb.  We now have either &skb->len or
	 * &skb->pkt_type, but we don't know which
	 */
	BPF_ALU64_REG(BPF_ADD, BPF_REG_1, BPF_REG_2),
	/* dereference it */
	BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_1, 0),
	BPF_EXIT_INSN(),
	},
	.errstr = "variable ctx access var_off=(0x0; 0x4)",
	.result = REJECT,
	.prog_type = BPF_PROG_TYPE_LWT_IN,
},
{
	"variable-offset stack access",
	.insns = {
	/* Fill the top 8 bytes of the stack */
	BPF_ST_MEM(BPF_DW, BPF_REG_10, -8, 0),
	/* Get an unknown value */
	BPF_LDX_MEM(BPF_W, BPF_REG_2, BPF_REG_1, 0),
	/* Make it small and 4-byte aligned */
	BPF_ALU64_IMM(BPF_AND, BPF_REG_2, 4),
	BPF_ALU64_IMM(BPF_SUB, BPF_REG_2, 8),
	/* add it to fp.  We now have either fp-4 or fp-8, but
	 * we don't know which
	 */
	BPF_ALU64_REG(BPF_ADD, BPF_REG_2, BPF_REG_10),
	/* dereference it */
	BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_2, 0),
	BPF_EXIT_INSN(),
	},
	.errstr = "variable stack access var_off=(0xfffffffffffffff8; 0x4)",
	.result = REJECT,
	.prog_type = BPF_PROG_TYPE_LWT_IN,
},
{
	"indirect variable-offset stack access",
	.insns = {
	/* Fill the top 8 bytes of the stack */
	BPF_ST_MEM(BPF_DW, BPF_REG_10, -8, 0),
	/* Get an unknown value */
	BPF_LDX_MEM(BPF_W, BPF_REG_2, BPF_REG_1, 0),
	/* Make it small and 4-byte aligned */
	BPF_ALU64_IMM(BPF_AND, BPF_REG_2, 4),
	BPF_ALU64_IMM(BPF_SUB, BPF_REG_2, 8),
	/* add it to fp.  We now have either fp-4 or fp-8, but
	 * we don't know which
	 */
	BPF_ALU64_REG(BPF_ADD, BPF_REG_2, BPF_REG_10),
	/* dereference it indirectly */
	BPF_LD_MAP_FD(BPF_REG_1, 0),
	BPF_RAW_INSN(BPF_JMP | BPF_CALL, 0, 0, 0, BPF_FUNC_map_lookup_elem),
	BPF_MOV64_IMM(BPF_REG_0, 0),
	BPF_EXIT_INSN(),
	},
	.fixup_map_hash_8b = { 5 },
	.errstr = "variable stack read R2",
	.result = REJECT,
	.prog_type = BPF_PROG_TYPE_LWT_IN,
},
