	.text
	.intel_syntax noprefix
	.file	"weighted-wavelet-assembly.cpp"
	.globl	weighted_add                    # -- Begin function weighted_add
	.p2align	4, 0x90
	.type	weighted_add,@function
weighted_add:                           # @weighted_add
	.cfi_startproc
# %bb.0:
	push	rbp
	.cfi_def_cfa_offset 16
	push	r15
	.cfi_def_cfa_offset 24
	push	r14
	.cfi_def_cfa_offset 32
	push	r13
	.cfi_def_cfa_offset 40
	push	r12
	.cfi_def_cfa_offset 48
	push	rbx
	.cfi_def_cfa_offset 56
	.cfi_offset rbx, -56
	.cfi_offset r12, -48
	.cfi_offset r13, -40
	.cfi_offset r14, -32
	.cfi_offset r15, -24
	.cfi_offset rbp, -16
                                        # kill: def $esi killed $esi def $rsi
	mov	eax, dword ptr [rdi + 56]
	cmp	eax, esi
	jle	.LBB0_3
# %bb.1:
	lea	r8d, [rsi + 1]
	mov	rcx, qword ptr [rdi + 64]
	.p2align	4, 0x90
.LBB0_2:                                # =>This Inner Loop Header: Depth=1
	movsxd	r9, r8d
	add	qword ptr [rcx + 8*r9 - 8], rdx
	mov	r8d, r9d
	neg	r8d
	and	r8d, r9d
	add	r8d, r9d
	cmp	r8d, eax
	jle	.LBB0_2
.LBB0_3:
	mov	rax, qword ptr [rdi + 32]
	mov	rdi, qword ptr [rdi + 40]
	cmp	rax, rdi
	je	.LBB0_9
# %bb.4:
	movabs	r8, 1537228672809129301
	movabs	r9, 3689348814741910323
	movabs	r10, 1085102592571150095
	movabs	r11, 72340172838076673
	jmp	.LBB0_5
	.p2align	4, 0x90
.LBB0_8:                                #   in Loop: Header=BB0_5 Depth=1
	add	rax, 88
	cmp	rax, rdi
	je	.LBB0_9
.LBB0_5:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_7 Depth 2
	lea	ebp, [rsi + 63]
	test	esi, esi
	cmovns	ebp, esi
	mov	ecx, ebp
	sar	ecx, 6
	movsxd	rbx, ecx
	mov	r15, qword ptr [rax]
	mov	r14, qword ptr [rax + 24]
	and	ebp, -64
	mov	ecx, esi
	sub	ecx, ebp
	mov	r12, -1
	shl	r12, cl
	mov	r15, qword ptr [r15 + 8*rbx]
	not	r12
	and	r12, r15
	mov	r13, r12
	shr	r13
	and	r13, r8
	sub	r12, r13
	mov	r13, r12
	and	r13, r9
	shr	r12, 2
	and	r12, r9
	add	r12, r13
	mov	r13, r12
	shr	r13, 4
	add	r13, r12
	and	r13, r10
	imul	r13, r11
	shr	r13, 56
	add	r13d, dword ptr [r14 + 4*rbx]
	mov	ebx, dword ptr [rax + 48]
	add	ebx, r13d
	sub	esi, r13d
	bt	r15, rcx
	cmovb	esi, ebx
	mov	ecx, dword ptr [rax + 56]
	cmp	ecx, esi
	jle	.LBB0_8
# %bb.6:                                #   in Loop: Header=BB0_5 Depth=1
	lea	ebp, [rsi + 1]
	mov	rbx, qword ptr [rax + 64]
	.p2align	4, 0x90
.LBB0_7:                                #   Parent Loop BB0_5 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	movsxd	r14, ebp
	add	qword ptr [rbx + 8*r14 - 8], rdx
	mov	ebp, r14d
	neg	ebp
	and	ebp, r14d
	add	ebp, r14d
	cmp	ebp, ecx
	jle	.LBB0_7
	jmp	.LBB0_8
.LBB0_9:
	pop	rbx
	.cfi_def_cfa_offset 48
	pop	r12
	.cfi_def_cfa_offset 40
	pop	r13
	.cfi_def_cfa_offset 32
	pop	r14
	.cfi_def_cfa_offset 24
	pop	r15
	.cfi_def_cfa_offset 16
	pop	rbp
	.cfi_def_cfa_offset 8
	ret
.Lfunc_end0:
	.size	weighted_add, .Lfunc_end0-weighted_add
	.cfi_endproc
                                        # -- End function
	.globl	weighted_sum                    # -- Begin function weighted_sum
	.p2align	4, 0x90
	.type	weighted_sum,@function
weighted_sum:                           # @weighted_sum
	.cfi_startproc
# %bb.0:
	sub	rsp, 24
	.cfi_def_cfa_offset 32
	mov	qword ptr [rsp + 16], rcx
	mov	qword ptr [rsp + 8], r8
	lea	rcx, [rsp + 16]
	lea	r8, [rsp + 8]
	call	_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_
	add	rsp, 24
	.cfi_def_cfa_offset 8
	ret
.Lfunc_end1:
	.size	weighted_sum, .Lfunc_end1-weighted_sum
	.cfi_endproc
                                        # -- End function
	.section	.text._ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_,"axG",@progbits,_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_,comdat
	.weak	_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_ # -- Begin function _ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_
	.p2align	4, 0x90
	.type	_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_,@function
_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_: # @_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_
	.cfi_startproc
# %bb.0:
	push	rbp
	.cfi_def_cfa_offset 16
	push	r15
	.cfi_def_cfa_offset 24
	push	r14
	.cfi_def_cfa_offset 32
	push	r13
	.cfi_def_cfa_offset 40
	push	r12
	.cfi_def_cfa_offset 48
	push	rbx
	.cfi_def_cfa_offset 56
	sub	rsp, 40
	.cfi_def_cfa_offset 96
	.cfi_offset rbx, -56
	.cfi_offset r12, -48
	.cfi_offset r13, -40
	.cfi_offset r14, -32
	.cfi_offset r15, -24
	.cfi_offset rbp, -16
                                        # kill: def $edx killed $edx def $rdx
	mov	ebp, esi
	mov	r10, qword ptr [rdi + 8]
	mov	rsi, qword ptr [rdi + 16]
	sub	rsi, r10
	mov	r9, rsi
	sar	r9, 3
	test	r9, r9
	jle	.LBB2_5
# %bb.1:
	mov	rcx, qword ptr [rcx]
	mov	rbx, r9
	mov	rax, r10
	.p2align	4, 0x90
.LBB2_2:                                # =>This Inner Loop Header: Depth=1
	mov	r14, rax
	mov	r15, rbx
	shr	r15
	mov	r12, r15
	not	r12
	add	r12, rbx
	cmp	qword ptr [rax + 8*r15], rcx
	lea	rax, [rax + 8*r15 + 8]
	cmovge	rax, r14
	cmovge	r12, r15
	mov	rbx, r12
	test	r12, r12
	jg	.LBB2_2
# %bb.3:
	sub	rax, r10
	shr	rax, 3
	mov	rcx, qword ptr [r8]
	mov	r13, r10
	.p2align	4, 0x90
.LBB2_4:                                # =>This Inner Loop Header: Depth=1
	mov	r8, r13
	mov	rbx, r9
	shr	rbx
	mov	r14, rbx
	not	r14
	add	r14, r9
	cmp	qword ptr [r13 + 8*rbx], rcx
	lea	r13, [r13 + 8*rbx + 8]
	cmovge	r13, r8
	cmovge	r14, rbx
	mov	r9, r14
	test	r14, r14
	jg	.LBB2_4
	jmp	.LBB2_6
.LBB2_5:
	xor	eax, eax
	mov	r13, r10
.LBB2_6:
	sub	r13, r10
	shr	r13, 3
	cmp	eax, r13d
	jne	.LBB2_8
# %bb.7:
	xor	ebx, ebx
	jmp	.LBB2_54
.LBB2_8:
	shr	rsi, 3
	cmp	r13d, esi
	jne	.LBB2_15
# %bb.9:
	test	edx, edx
	jle	.LBB2_22
# %bb.10:
	mov	rcx, qword ptr [rdi + 64]
	xor	ebx, ebx
	mov	esi, edx
	.p2align	4, 0x90
.LBB2_11:                               # =>This Inner Loop Header: Depth=1
	lea	r8d, [rsi - 1]
	add	rbx, qword ptr [rcx + 8*r8]
	and	r8d, esi
	mov	esi, r8d
	jne	.LBB2_11
# %bb.12:
	test	ebp, ebp
	jle	.LBB2_23
.LBB2_13:
	mov	rcx, qword ptr [rdi + 64]
	xor	r14d, r14d
	mov	esi, ebp
	.p2align	4, 0x90
.LBB2_14:                               # =>This Inner Loop Header: Depth=1
	lea	r8d, [rsi - 1]
	add	r14, qword ptr [rcx + 8*r8]
	and	r8d, esi
	mov	esi, r8d
	jne	.LBB2_14
	jmp	.LBB2_24
.LBB2_15:
	mov	r15, qword ptr [rdi + 32]
	mov	rcx, qword ptr [rdi + 40]
	sub	rcx, r15
	sar	rcx, 3
	movabs	rdi, 3353953467947191203
	imul	rdi, rcx
	test	edi, edi
	jle	.LBB2_25
# %bb.16:
	mov	ecx, eax
	xor	ecx, r13d
	mov	dword ptr [rsp + 24], ecx       # 4-byte Spill
	mov	ecx, edi
	mov	qword ptr [rsp + 32], rdi       # 8-byte Spill
	lea	esi, [rdi - 1]
	mov	qword ptr [rsp + 16], rsi       # 8-byte Spill
	neg	rcx
	mov	qword ptr [rsp + 8], rcx        # 8-byte Spill
	xor	ebx, ebx
	mov	r8, r15
	xor	r12d, r12d
	mov	qword ptr [rsp], r15            # 8-byte Spill
	jmp	.LBB2_19
	.p2align	4, 0x90
.LBB2_17:                               #   in Loop: Header=BB2_19 Depth=1
	mov	edx, dword ptr [r8 + 48]
	add	r11d, edx
	add	ecx, edx
	mov	ebp, r11d
	mov	edx, ecx
.LBB2_18:                               #   in Loop: Header=BB2_19 Depth=1
	mov	r15, qword ptr [rsp]            # 8-byte Reload
	dec	r12
	add	r8, 88
	cmp	qword ptr [rsp + 8], r12        # 8-byte Folded Reload
	je	.LBB2_52
.LBB2_19:                               # =>This Inner Loop Header: Depth=1
	mov	rcx, qword ptr [rsp + 16]       # 8-byte Reload
	lea	rdi, [rcx + r12]
	mov	r9d, dword ptr [rsp + 24]       # 4-byte Reload
	mov	ecx, edi
	sar	r9d, cl
	test	r9d, r9d
	jne	.LBB2_26
# %bb.20:                               #   in Loop: Header=BB2_19 Depth=1
	lea	r9d, [rbp + 63]
	test	ebp, ebp
	cmovns	r9d, ebp
	mov	ecx, r9d
	sar	ecx, 6
	movsxd	r14, ecx
	mov	r10, qword ptr [r8]
	mov	r15, qword ptr [r8 + 24]
	and	r9d, -64
	mov	ecx, ebp
	sub	ecx, r9d
	mov	r9, -1
                                        # kill: def $cl killed $cl killed $ecx
	shl	r9, cl
	not	r9
	and	r9, qword ptr [r10 + 8*r14]
	mov	rcx, r9
	shr	rcx
	movabs	rsi, 1537228672809129301
	and	rcx, rsi
	sub	r9, rcx
	mov	rcx, r9
	movabs	rsi, 3689348814741910323
	and	rcx, rsi
	shr	r9, 2
	and	r9, rsi
	add	r9, rcx
	mov	r11, r9
	shr	r11, 4
	add	r11, r9
	movabs	rcx, 1085102592571150095
	and	r11, rcx
	movabs	rcx, 72340172838076673
	imul	r11, rcx
	shr	r11, 56
	add	r11d, dword ptr [r15 + 4*r14]
	lea	r9d, [rdx + 63]
	test	edx, edx
	cmovns	r9d, edx
	mov	ecx, r9d
	sar	ecx, 6
	movsxd	r14, ecx
	and	r9d, -64
	mov	ecx, edx
	sub	ecx, r9d
	mov	r9, -1
                                        # kill: def $cl killed $cl killed $ecx
	shl	r9, cl
	not	r9
	and	r9, qword ptr [r10 + 8*r14]
	mov	rcx, r9
	shr	rcx
	movabs	r10, 1537228672809129301
	and	rcx, r10
	sub	r9, rcx
	mov	rcx, r9
	and	rcx, rsi
	shr	r9, 2
	and	r9, rsi
	add	r9, rcx
	mov	rcx, r9
	shr	rcx, 4
	add	rcx, r9
	movabs	rsi, 1085102592571150095
	and	rcx, rsi
	movabs	rsi, 72340172838076673
	imul	rcx, rsi
	shr	rcx, 56
	add	ecx, dword ptr [r15 + 4*r14]
	bt	eax, edi
	jb	.LBB2_17
# %bb.21:                               #   in Loop: Header=BB2_19 Depth=1
	sub	ebp, r11d
	sub	edx, ecx
	jmp	.LBB2_18
.LBB2_22:
	xor	ebx, ebx
	test	ebp, ebp
	jg	.LBB2_13
.LBB2_23:
	xor	r14d, r14d
.LBB2_24:
	mov	esi, ebp
                                        # kill: def $edx killed $edx killed $rdx
	mov	ecx, eax
	call	_ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii
	add	rax, r14
	jmp	.LBB2_53
.LBB2_25:
	xor	ebx, ebx
.LBB2_52:
	xor	eax, eax
	jmp	.LBB2_53
.LBB2_26:
	neg	r12
	movsxd	rcx, dword ptr [rsp + 32]       # 4-byte Folded Reload
	mov	qword ptr [rsp + 24], rcx       # 8-byte Spill
	xor	ecx, ecx
	mov	qword ptr [rsp + 8], rcx        # 8-byte Spill
	mov	r11, r12
	mov	esi, ebp
	mov	r10d, edx
	jmp	.LBB2_29
	.p2align	4, 0x90
.LBB2_27:                               #   in Loop: Header=BB2_29 Depth=1
	mov	r15, qword ptr [rsp]            # 8-byte Reload
.LBB2_28:                               #   in Loop: Header=BB2_29 Depth=1
	inc	r11
	cmp	r11, qword ptr [rsp + 24]       # 8-byte Folded Reload
	je	.LBB2_39
.LBB2_29:                               # =>This Loop Header: Depth=1
                                        #     Child Loop BB2_32 Depth 2
                                        #     Child Loop BB2_35 Depth 2
	imul	rdi, r11, 88
	lea	r8d, [rsi + 63]
	test	esi, esi
	cmovns	r8d, esi
	mov	ecx, r8d
	sar	ecx, 6
	movsxd	r14, ecx
	mov	r9, qword ptr [r15 + rdi]
	mov	qword ptr [rsp + 16], rdi       # 8-byte Spill
	mov	r15, qword ptr [r15 + rdi + 24]
	and	r8d, -64
	mov	ecx, esi
	sub	ecx, r8d
	mov	r8, -1
                                        # kill: def $cl killed $cl killed $ecx
	shl	r8, cl
	not	r8
	and	r8, qword ptr [r9 + 8*r14]
	mov	rcx, r8
	shr	rcx
	movabs	rbx, 1537228672809129301
	and	rcx, rbx
	sub	r8, rcx
	mov	rcx, r8
	movabs	rdi, 3689348814741910323
	and	rcx, rdi
	shr	r8, 2
	and	r8, rdi
	add	r8, rcx
	mov	rbx, r8
	shr	rbx, 4
	add	rbx, r8
	movabs	rcx, 1085102592571150095
	and	rbx, rcx
	movabs	rcx, 72340172838076673
	imul	rbx, rcx
	shr	rbx, 56
	add	ebx, dword ptr [r15 + 4*r14]
	sub	esi, ebx
	lea	r8d, [r10 + 63]
	test	r10d, r10d
	cmovns	r8d, r10d
	mov	ecx, r8d
	sar	ecx, 6
	movsxd	r14, ecx
	and	r8d, -64
	mov	ecx, r10d
	sub	ecx, r8d
	mov	r8, -1
                                        # kill: def $cl killed $cl killed $ecx
	shl	r8, cl
	not	r8
	and	r8, qword ptr [r9 + 8*r14]
	mov	rcx, r8
	shr	rcx
	movabs	r9, 1537228672809129301
	and	rcx, r9
	sub	r8, rcx
	mov	rcx, r8
	and	rcx, rdi
	shr	r8, 2
	and	r8, rdi
	add	r8, rcx
	mov	rcx, r8
	shr	rcx, 4
	add	rcx, r8
	movabs	rdi, 1085102592571150095
	and	rcx, rdi
	movabs	rdi, 72340172838076673
	imul	rcx, rdi
	shr	rcx, 56
	add	ecx, dword ptr [r15 + 4*r14]
	sub	r10d, ecx
	mov	r8d, r11d
	not	r8d
	add	r8d, dword ptr [rsp + 32]       # 4-byte Folded Reload
	bt	r13d, r8d
	jae	.LBB2_27
# %bb.30:                               #   in Loop: Header=BB2_29 Depth=1
	test	r10d, r10d
	jle	.LBB2_36
# %bb.31:                               #   in Loop: Header=BB2_29 Depth=1
	mov	r15, qword ptr [rsp]            # 8-byte Reload
	mov	rdi, qword ptr [rsp + 16]       # 8-byte Reload
	mov	r14, qword ptr [r15 + rdi + 64]
	xor	r9d, r9d
	.p2align	4, 0x90
.LBB2_32:                               #   Parent Loop BB2_29 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lea	r8d, [r10 - 1]
	add	r9, qword ptr [r14 + 8*r8]
	and	r8d, r10d
	mov	r10d, r8d
	jne	.LBB2_32
# %bb.33:                               #   in Loop: Header=BB2_29 Depth=1
	test	esi, esi
	jle	.LBB2_37
.LBB2_34:                               #   in Loop: Header=BB2_29 Depth=1
	mov	r14, qword ptr [r15 + rdi + 64]
	xor	r10d, r10d
	.p2align	4, 0x90
.LBB2_35:                               #   Parent Loop BB2_29 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lea	r8d, [rsi - 1]
	add	r10, qword ptr [r14 + 8*r8]
	and	r8d, esi
	mov	esi, r8d
	jne	.LBB2_35
	jmp	.LBB2_38
	.p2align	4, 0x90
.LBB2_36:                               #   in Loop: Header=BB2_29 Depth=1
	xor	r9d, r9d
	mov	r15, qword ptr [rsp]            # 8-byte Reload
	mov	rdi, qword ptr [rsp + 16]       # 8-byte Reload
	test	esi, esi
	jg	.LBB2_34
.LBB2_37:                               #   in Loop: Header=BB2_29 Depth=1
	xor	r10d, r10d
.LBB2_38:                               #   in Loop: Header=BB2_29 Depth=1
	add	r9, qword ptr [rsp + 8]         # 8-byte Folded Reload
	sub	r9, r10
	mov	esi, dword ptr [r15 + rdi + 48]
	add	ebx, esi
	add	ecx, esi
	mov	qword ptr [rsp + 8], r9         # 8-byte Spill
	mov	r10d, ecx
	mov	esi, ebx
	jmp	.LBB2_28
.LBB2_39:
	xor	ecx, ecx
	mov	qword ptr [rsp], rcx            # 8-byte Spill
	mov	rbx, qword ptr [rsp + 8]        # 8-byte Reload
	movabs	r14, 1537228672809129301
	movabs	r13, 3689348814741910323
	jmp	.LBB2_43
	.p2align	4, 0x90
.LBB2_40:                               #   in Loop: Header=BB2_43 Depth=1
	xor	edx, edx
.LBB2_41:                               #   in Loop: Header=BB2_43 Depth=1
	add	r8, qword ptr [rsp]             # 8-byte Folded Reload
	sub	r8, rdx
	mov	edx, dword ptr [r15 + rsi + 48]
	add	edi, edx
	add	ecx, edx
	mov	qword ptr [rsp], r8             # 8-byte Spill
	mov	edx, ecx
	mov	ebp, edi
.LBB2_42:                               #   in Loop: Header=BB2_43 Depth=1
	inc	r12
	cmp	r12, qword ptr [rsp + 24]       # 8-byte Folded Reload
	je	.LBB2_51
.LBB2_43:                               # =>This Loop Header: Depth=1
                                        #     Child Loop BB2_46 Depth 2
                                        #     Child Loop BB2_50 Depth 2
	imul	rsi, r12, 88
	lea	edi, [rbp + 63]
	test	ebp, ebp
	cmovns	edi, ebp
	mov	ecx, edi
	sar	ecx, 6
	movsxd	r10, ecx
	mov	r9, qword ptr [r15 + rsi]
	mov	r8, qword ptr [r15 + rsi + 24]
	and	edi, -64
	mov	ecx, ebp
	sub	ecx, edi
	mov	r11, -1
                                        # kill: def $cl killed $cl killed $ecx
	shl	r11, cl
	not	r11
	and	r11, qword ptr [r9 + 8*r10]
	mov	rcx, r11
	shr	rcx
	and	rcx, r14
	sub	r11, rcx
	mov	rcx, r11
	and	rcx, r13
	shr	r11, 2
	and	r11, r13
	add	r11, rcx
	mov	rdi, r11
	shr	rdi, 4
	add	rdi, r11
	movabs	rcx, 1085102592571150095
	and	rdi, rcx
	movabs	rcx, 72340172838076673
	imul	rdi, rcx
	shr	rdi, 56
	add	edi, dword ptr [r8 + 4*r10]
	sub	ebp, edi
	lea	r11d, [rdx + 63]
	test	edx, edx
	cmovns	r11d, edx
	mov	ecx, r11d
	sar	ecx, 6
	movsxd	r10, ecx
	and	r11d, -64
	mov	ecx, edx
	sub	ecx, r11d
	mov	r11, -1
                                        # kill: def $cl killed $cl killed $ecx
	shl	r11, cl
	not	r11
	and	r11, qword ptr [r9 + 8*r10]
	mov	rcx, r11
	shr	rcx
	and	rcx, r14
	sub	r11, rcx
	mov	rcx, r11
	and	rcx, r13
	shr	r11, 2
	and	r11, r13
	add	r11, rcx
	mov	rcx, r11
	shr	rcx, 4
	add	rcx, r11
	movabs	r9, 1085102592571150095
	and	rcx, r9
	movabs	r9, 72340172838076673
	imul	rcx, r9
	shr	rcx, 56
	add	ecx, dword ptr [r8 + 4*r10]
	sub	edx, ecx
	mov	r8d, r12d
	not	r8d
	add	r8d, dword ptr [rsp + 32]       # 4-byte Folded Reload
	bt	eax, r8d
	jae	.LBB2_42
# %bb.44:                               #   in Loop: Header=BB2_43 Depth=1
	test	edx, edx
	jle	.LBB2_48
# %bb.45:                               #   in Loop: Header=BB2_43 Depth=1
	mov	r9, qword ptr [r15 + rsi + 64]
	xor	r8d, r8d
	.p2align	4, 0x90
.LBB2_46:                               #   Parent Loop BB2_43 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lea	r10d, [rdx - 1]
	add	r8, qword ptr [r9 + 8*r10]
	and	r10d, edx
	mov	edx, r10d
	jne	.LBB2_46
# %bb.47:                               #   in Loop: Header=BB2_43 Depth=1
	test	ebp, ebp
	jg	.LBB2_49
	jmp	.LBB2_40
	.p2align	4, 0x90
.LBB2_48:                               #   in Loop: Header=BB2_43 Depth=1
	xor	r8d, r8d
	test	ebp, ebp
	jle	.LBB2_40
.LBB2_49:                               #   in Loop: Header=BB2_43 Depth=1
	mov	r9, qword ptr [r15 + rsi + 64]
	xor	edx, edx
	.p2align	4, 0x90
.LBB2_50:                               #   Parent Loop BB2_43 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lea	r10d, [rbp - 1]
	add	rdx, qword ptr [r9 + 8*r10]
	and	r10d, ebp
	mov	ebp, r10d
	jne	.LBB2_50
	jmp	.LBB2_41
.LBB2_51:
	mov	rax, qword ptr [rsp]            # 8-byte Reload
.LBB2_53:
	sub	rbx, rax
.LBB2_54:
	mov	rax, rbx
	add	rsp, 40
	.cfi_def_cfa_offset 56
	pop	rbx
	.cfi_def_cfa_offset 48
	pop	r12
	.cfi_def_cfa_offset 40
	pop	r13
	.cfi_def_cfa_offset 32
	pop	r14
	.cfi_def_cfa_offset 24
	pop	r15
	.cfi_def_cfa_offset 16
	pop	rbp
	.cfi_def_cfa_offset 8
	ret
.Lfunc_end2:
	.size	_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_, .Lfunc_end2-_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_
	.cfi_endproc
                                        # -- End function
	.section	.text._ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii,"axG",@progbits,_ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii,comdat
	.weak	_ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii # -- Begin function _ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii
	.p2align	4, 0x90
	.type	_ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii,@function
_ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii: # @_ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii
	.cfi_startproc
# %bb.0:
	push	rbp
	.cfi_def_cfa_offset 16
	push	r15
	.cfi_def_cfa_offset 24
	push	r14
	.cfi_def_cfa_offset 32
	push	r13
	.cfi_def_cfa_offset 40
	push	r12
	.cfi_def_cfa_offset 48
	push	rbx
	.cfi_def_cfa_offset 56
	.cfi_offset rbx, -56
	.cfi_offset r12, -48
	.cfi_offset r13, -40
	.cfi_offset r14, -32
	.cfi_offset r15, -24
	.cfi_offset rbp, -16
                                        # kill: def $edx killed $edx def $rdx
                                        # kill: def $esi killed $esi def $rsi
	mov	rax, qword ptr [rdi + 16]
	sub	rax, qword ptr [rdi + 8]
	shr	rax, 3
	mov	dword ptr [rsp - 12], ecx       # 4-byte Spill
	cmp	eax, ecx
	jne	.LBB3_10
# %bb.1:
	test	edx, edx
	jle	.LBB3_2
# %bb.3:
	mov	rcx, qword ptr [rdi + 64]
	xor	eax, eax
	.p2align	4, 0x90
.LBB3_4:                                # =>This Inner Loop Header: Depth=1
	lea	r8d, [rdx - 1]
	add	rax, qword ptr [rcx + 8*r8]
	and	r8d, edx
	mov	edx, r8d
	jne	.LBB3_4
# %bb.5:
	test	esi, esi
	jle	.LBB3_6
.LBB3_7:
	mov	rdx, qword ptr [rdi + 64]
	xor	ecx, ecx
	.p2align	4, 0x90
.LBB3_8:                                # =>This Inner Loop Header: Depth=1
	lea	edi, [rsi - 1]
	add	rcx, qword ptr [rdx + 8*rdi]
	and	edi, esi
	mov	esi, edi
	jne	.LBB3_8
# %bb.9:
	sub	rax, rcx
	jmp	.LBB3_24
.LBB3_10:
	mov	r9, qword ptr [rdi + 32]
	mov	rcx, qword ptr [rdi + 40]
	cmp	r9, rcx
	je	.LBB3_11
# %bb.12:
	mov	rax, rcx
	sub	rax, r9
	shr	rax, 3
	imul	r8d, eax, -1171354717
	xor	eax, eax
	mov	qword ptr [rsp - 8], rcx        # 8-byte Spill
	movabs	r10, 3689348814741910323
	movabs	rbx, 1085102592571150095
	jmp	.LBB3_13
	.p2align	4, 0x90
.LBB3_19:                               #   in Loop: Header=BB3_13 Depth=1
	xor	edx, edx
.LBB3_22:                               #   in Loop: Header=BB3_13 Depth=1
	add	r13, rax
	sub	r13, rdx
	mov	eax, dword ptr [r9 + 48]
	add	r14d, eax
	add	ecx, eax
	mov	rax, r13
	mov	edx, ecx
	mov	esi, r14d
.LBB3_23:                               #   in Loop: Header=BB3_13 Depth=1
	add	r9, 88
	mov	rcx, qword ptr [rsp - 8]        # 8-byte Reload
	cmp	r9, rcx
	je	.LBB3_24
.LBB3_13:                               # =>This Loop Header: Depth=1
                                        #     Child Loop BB3_17 Depth 2
                                        #     Child Loop BB3_21 Depth 2
	dec	r8d
	lea	r12d, [rsi + 63]
	test	esi, esi
	cmovns	r12d, esi
	mov	ecx, r12d
	sar	ecx, 6
	movsxd	r11, ecx
	mov	rbp, qword ptr [r9]
	mov	r13, qword ptr [r9 + 24]
	and	r12d, -64
	mov	ecx, esi
	sub	ecx, r12d
	mov	r12, -1
                                        # kill: def $cl killed $cl killed $ecx
	shl	r12, cl
	not	r12
	and	r12, qword ptr [rbp + 8*r11]
	mov	rcx, r12
	shr	rcx
	movabs	rdi, 1537228672809129301
	and	rcx, rdi
	sub	r12, rcx
	mov	rcx, r12
	and	rcx, r10
	shr	r12, 2
	and	r12, r10
	add	r12, rcx
	mov	r14, r12
	shr	r14, 4
	add	r14, r12
	and	r14, rbx
	movabs	rcx, 72340172838076673
	imul	r14, rcx
	shr	r14, 56
	add	r14d, dword ptr [r13 + 4*r11]
	sub	esi, r14d
	lea	r11d, [rdx + 63]
	test	edx, edx
	cmovns	r11d, edx
	mov	r12d, r11d
	sar	r12d, 6
	and	r11d, -64
	mov	ecx, edx
	sub	ecx, r11d
	mov	r15, -1
                                        # kill: def $cl killed $cl killed $ecx
	shl	r15, cl
	movsxd	r11, r12d
	not	r15
	and	r15, qword ptr [rbp + 8*r11]
	mov	rcx, r15
	shr	rcx
	and	rcx, rdi
	sub	r15, rcx
	mov	rcx, r15
	and	rcx, r10
	shr	r15, 2
	and	r15, r10
	add	r15, rcx
	mov	rcx, r15
	shr	rcx, 4
	add	rcx, r15
	and	rcx, rbx
	movabs	rdi, 72340172838076673
	imul	rcx, rdi
	shr	rcx, 56
	add	ecx, dword ptr [r13 + 4*r11]
	sub	edx, ecx
	mov	edi, dword ptr [rsp - 12]       # 4-byte Reload
	bt	edi, r8d
	jae	.LBB3_23
# %bb.14:                               #   in Loop: Header=BB3_13 Depth=1
	test	edx, edx
	jle	.LBB3_15
# %bb.16:                               #   in Loop: Header=BB3_13 Depth=1
	mov	r11, qword ptr [r9 + 64]
	xor	r13d, r13d
	.p2align	4, 0x90
.LBB3_17:                               #   Parent Loop BB3_13 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lea	r15d, [rdx - 1]
	add	r13, qword ptr [r11 + 8*r15]
	and	r15d, edx
	mov	edx, r15d
	jne	.LBB3_17
# %bb.18:                               #   in Loop: Header=BB3_13 Depth=1
	test	esi, esi
	jg	.LBB3_20
	jmp	.LBB3_19
	.p2align	4, 0x90
.LBB3_15:                               #   in Loop: Header=BB3_13 Depth=1
	xor	r13d, r13d
	test	esi, esi
	jle	.LBB3_19
.LBB3_20:                               #   in Loop: Header=BB3_13 Depth=1
	mov	r11, qword ptr [r9 + 64]
	xor	edx, edx
	.p2align	4, 0x90
.LBB3_21:                               #   Parent Loop BB3_13 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lea	r15d, [rsi - 1]
	add	rdx, qword ptr [r11 + 8*r15]
	and	r15d, esi
	mov	esi, r15d
	jne	.LBB3_21
	jmp	.LBB3_22
.LBB3_2:
	xor	eax, eax
	test	esi, esi
	jg	.LBB3_7
.LBB3_6:
	xor	ecx, ecx
	sub	rax, rcx
	jmp	.LBB3_24
.LBB3_11:
	xor	eax, eax
.LBB3_24:
	pop	rbx
	.cfi_def_cfa_offset 48
	pop	r12
	.cfi_def_cfa_offset 40
	pop	r13
	.cfi_def_cfa_offset 32
	pop	r14
	.cfi_def_cfa_offset 24
	pop	r15
	.cfi_def_cfa_offset 16
	pop	rbp
	.cfi_def_cfa_offset 8
	ret
.Lfunc_end3:
	.size	_ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii, .Lfunc_end3-_ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii
	.cfi_endproc
                                        # -- End function
	.section	".linker-options","e",@llvm_linker_options
	.ident	"Ubuntu clang version 18.1.3 (1ubuntu1)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
