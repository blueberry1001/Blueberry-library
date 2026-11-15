	.text
	.intel_syntax noprefix
	.file	"weighted-wavelet-assembly.cpp"
	.globl	weighted_add                    # -- Begin function weighted_add
	.p2align	4, 0x90
	.type	weighted_add,@function
weighted_add:                           # @weighted_add
	.cfi_startproc
# %bb.0:
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
	blsi	r8d, r9d
	add	r8d, r9d
	cmp	r8d, eax
	jle	.LBB0_2
.LBB0_3:
	mov	rax, qword ptr [rdi + 32]
	mov	rcx, qword ptr [rdi + 40]
	jmp	.LBB0_4
	.p2align	4, 0x90
.LBB0_8:                                #   in Loop: Header=BB0_4 Depth=1
	add	rax, 88
.LBB0_4:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_7 Depth 2
	cmp	rax, rcx
	je	.LBB0_9
# %bb.5:                                #   in Loop: Header=BB0_4 Depth=1
	lea	edi, [rsi + 63]
	test	esi, esi
	cmovns	edi, esi
	mov	r8d, edi
	sar	r8d, 6
	movsxd	r8, r8d
	mov	r9, qword ptr [rax]
	mov	r10, qword ptr [rax + 24]
	mov	r9, qword ptr [r9 + 8*r8]
	and	edi, -64
	mov	r11d, esi
	sub	r11d, edi
	bzhi	rdi, r9, r11
	popcnt	rdi, rdi
	add	edi, dword ptr [r10 + 4*r8]
	mov	r8d, dword ptr [rax + 48]
	add	r8d, edi
	sub	esi, edi
	bt	r9, r11
	cmovb	esi, r8d
	mov	edi, dword ptr [rax + 56]
	cmp	edi, esi
	jle	.LBB0_8
# %bb.6:                                #   in Loop: Header=BB0_4 Depth=1
	lea	r9d, [rsi + 1]
	mov	r8, qword ptr [rax + 64]
	.p2align	4, 0x90
.LBB0_7:                                #   Parent Loop BB0_4 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	movsxd	r10, r9d
	add	qword ptr [r8 + 8*r10 - 8], rdx
	blsi	r9d, r10d
	add	r9d, r10d
	cmp	r9d, edi
	jle	.LBB0_7
	jmp	.LBB0_8
.LBB0_9:
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
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0                          # -- Begin function _ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_
.LCPI2_0:
	.long	4294967232                      # 0xffffffc0
	.section	.rodata.cst16,"aM",@progbits,16
	.p2align	4, 0x0
.LCPI2_1:
	.long	4294967295                      # 0xffffffff
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	0                               # 0x0
.LCPI2_2:
	.zero	16,15
.LCPI2_3:
	.byte	0                               # 0x0
	.byte	1                               # 0x1
	.byte	1                               # 0x1
	.byte	2                               # 0x2
	.byte	1                               # 0x1
	.byte	2                               # 0x2
	.byte	2                               # 0x2
	.byte	3                               # 0x3
	.byte	1                               # 0x1
	.byte	2                               # 0x2
	.byte	2                               # 0x2
	.byte	3                               # 0x3
	.byte	2                               # 0x2
	.byte	3                               # 0x3
	.byte	3                               # 0x3
	.byte	4                               # 0x4
	.section	.rodata,"a",@progbits
.LCPI2_4:
	.byte	15                              # 0xf
	.section	.text._ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_,"axG",@progbits,_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_,comdat
	.weak	_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_
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
	push	rax
	.cfi_def_cfa_offset 64
	.cfi_offset rbx, -56
	.cfi_offset r12, -48
	.cfi_offset r13, -40
	.cfi_offset r14, -32
	.cfi_offset r15, -24
	.cfi_offset rbp, -16
	mov	r10, qword ptr [rdi + 8]
	mov	rax, qword ptr [rdi + 16]
	sub	rax, r10
	mov	r11, rax
	sar	r11, 3
	test	r11, r11
	jle	.LBB2_9
# %bb.1:
	mov	r9, qword ptr [rcx]
	mov	r14, r11
	mov	rcx, r10
	mov	rbx, r11
	jmp	.LBB2_3
	.p2align	4, 0x90
.LBB2_2:                                #   in Loop: Header=BB2_3 Depth=1
	mov	r14, rbx
	test	rbx, rbx
	jle	.LBB2_5
.LBB2_3:                                # =>This Inner Loop Header: Depth=1
	shr	rbx
	cmp	qword ptr [rcx + 8*rbx], r9
	jge	.LBB2_2
# %bb.4:                                #   in Loop: Header=BB2_3 Depth=1
	lea	rcx, [rcx + 8*rbx]
	add	rcx, 8
	not	rbx
	add	rbx, r14
	jmp	.LBB2_2
.LBB2_5:
	sub	rcx, r10
	shr	rcx, 3
	mov	r8, qword ptr [r8]
	mov	r9, r10
	mov	rbx, r11
	jmp	.LBB2_7
	.p2align	4, 0x90
.LBB2_6:                                #   in Loop: Header=BB2_7 Depth=1
	mov	r11, rbx
	test	rbx, rbx
	jle	.LBB2_10
.LBB2_7:                                # =>This Inner Loop Header: Depth=1
	shr	rbx
	cmp	qword ptr [r9 + 8*rbx], r8
	jge	.LBB2_6
# %bb.8:                                #   in Loop: Header=BB2_7 Depth=1
	lea	r9, [r9 + 8*rbx]
	add	r9, 8
	not	rbx
	add	rbx, r11
	jmp	.LBB2_6
.LBB2_9:
	xor	ecx, ecx
	mov	r9, r10
.LBB2_10:
	sub	r9, r10
	shr	r9, 3
	cmp	ecx, r9d
	jne	.LBB2_12
# %bb.11:
	xor	eax, eax
	jmp	.LBB2_56
.LBB2_12:
	shr	rax, 3
	cmp	r9d, eax
	jne	.LBB2_19
# %bb.13:
	test	edx, edx
	jle	.LBB2_26
# %bb.14:
	mov	rax, qword ptr [rdi + 64]
	xor	ebx, ebx
	mov	r8d, edx
	.p2align	4, 0x90
.LBB2_15:                               # =>This Inner Loop Header: Depth=1
	lea	r9d, [r8 - 1]
	add	rbx, qword ptr [rax + 8*r9]
	and	r9d, r8d
	mov	r8d, r9d
	jne	.LBB2_15
# %bb.16:
	test	esi, esi
	jle	.LBB2_27
.LBB2_17:
	mov	rax, qword ptr [rdi + 64]
	xor	r14d, r14d
	mov	r8d, esi
	.p2align	4, 0x90
.LBB2_18:                               # =>This Inner Loop Header: Depth=1
	lea	r9d, [r8 - 1]
	add	r14, qword ptr [rax + 8*r9]
	and	r9d, r8d
	mov	r8d, r9d
	jne	.LBB2_18
	jmp	.LBB2_28
.LBB2_19:
	mov	r8, qword ptr [rdi + 32]
	mov	rax, qword ptr [rdi + 40]
	sub	rax, r8
	sar	rax, 3
	movabs	rdi, 3353953467947191203
	imul	rdi, rax
	test	edi, edi
	jle	.LBB2_29
# %bb.20:
	mov	r10d, ecx
	xor	r10d, r9d
	mov	r11d, edi
	vmovd	xmm0, esi
	vpinsrd	xmm4, xmm0, edx, 1
	lea	esi, [rdi - 1]
	neg	r11
	xor	eax, eax
	vpbroadcastd	xmm0, dword ptr [rip + .LCPI2_0] # xmm0 = [4294967232,4294967232,4294967232,4294967232]
	vmovdqa	xmm1, xmmword ptr [rip + .LCPI2_1] # xmm1 = [4294967295,4294967295,0,0]
	vpxor	xmm5, xmm5, xmm5
	vpcmpeqd	xmm6, xmm6, xmm6
	vpbroadcastb	xmm2, byte ptr [rip + .LCPI2_4] # xmm2 = [15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15]
	vmovdqa	xmm3, xmmword ptr [rip + .LCPI2_3] # xmm3 = [0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4]
	mov	rbx, r8
	xor	edx, edx
	jmp	.LBB2_23
	.p2align	4, 0x90
.LBB2_21:                               #   in Loop: Header=BB2_23 Depth=1
	vpbroadcastd	xmm4, dword ptr [rbx + 48]
	vpaddd	xmm4, xmm7, xmm4
.LBB2_22:                               #   in Loop: Header=BB2_23 Depth=1
	dec	rdx
	add	rbx, 88
	cmp	r11, rdx
	je	.LBB2_54
.LBB2_23:                               # =>This Inner Loop Header: Depth=1
	lea	r14, [rsi + rdx]
	sarx	ebp, r10d, r14d
	test	ebp, ebp
	jne	.LBB2_30
# %bb.24:                               #   in Loop: Header=BB2_23 Depth=1
	vpsrad	xmm7, xmm4, 31
	vpsrld	xmm7, xmm7, 26
	vpaddd	xmm7, xmm4, xmm7
	vpsrad	xmm8, xmm7, 6
	mov	r15, qword ptr [rbx + 24]
	vmovdqa	xmm9, xmm1
	vpxor	xmm10, xmm10, xmm10
	vpgatherdd	xmm10, xmmword ptr [r15 + 4*xmm8], xmm9
	mov	r15, qword ptr [rbx]
	vpand	xmm7, xmm7, xmm0
	vpsubd	xmm7, xmm4, xmm7
	vpcmpeqd	xmm9, xmm9, xmm9
	vpxor	xmm11, xmm11, xmm11
	vpgatherdq	xmm11, xmmword ptr [r15 + 8*xmm8], xmm9
	vpmovzxdq	xmm7, xmm7              # xmm7 = xmm7[0],zero,xmm7[1],zero
	vpsllvq	xmm7, xmm6, xmm7
	vpandn	xmm7, xmm7, xmm11
	vpand	xmm8, xmm7, xmm2
	vpshufb	xmm8, xmm3, xmm8
	vpsrlw	xmm7, xmm7, 4
	vpand	xmm7, xmm7, xmm2
	vpshufb	xmm7, xmm3, xmm7
	vpaddb	xmm7, xmm8, xmm7
	vpsadbw	xmm7, xmm7, xmm5
	vpshufd	xmm7, xmm7, 232                 # xmm7 = xmm7[0,2,2,3]
	vpaddd	xmm7, xmm10, xmm7
	bt	ecx, r14d
	jb	.LBB2_21
# %bb.25:                               #   in Loop: Header=BB2_23 Depth=1
	vpsubd	xmm4, xmm4, xmm7
	jmp	.LBB2_22
.LBB2_26:
	xor	ebx, ebx
	test	esi, esi
	jg	.LBB2_17
.LBB2_27:
	xor	r14d, r14d
.LBB2_28:
                                        # kill: def $ecx killed $ecx killed $rcx
	call	_ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii
	mov	rcx, rax
	add	rcx, r14
	mov	rax, rbx
	sub	rax, rcx
	jmp	.LBB2_56
.LBB2_29:
	xor	eax, eax
.LBB2_54:
	xor	r9d, r9d
.LBB2_55:
	sub	rax, r9
.LBB2_56:
	add	rsp, 8
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
.LBB2_30:
	.cfi_def_cfa_offset 64
	neg	rdx
	movsxd	rsi, edi
	xor	eax, eax
	vpxor	xmm5, xmm5, xmm5
	vpcmpeqd	xmm6, xmm6, xmm6
	mov	r10, rdx
	vmovdqa	xmm7, xmm4
	jmp	.LBB2_34
	.p2align	4, 0x90
.LBB2_31:                               #   in Loop: Header=BB2_34 Depth=1
	xor	r14d, r14d
.LBB2_32:                               #   in Loop: Header=BB2_34 Depth=1
	add	rbx, rax
	sub	rbx, r14
	vpbroadcastd	xmm7, dword ptr [r8 + r11 + 48]
	vpaddd	xmm7, xmm8, xmm7
	mov	rax, rbx
.LBB2_33:                               #   in Loop: Header=BB2_34 Depth=1
	inc	r10
	cmp	r10, rsi
	je	.LBB2_42
.LBB2_34:                               # =>This Loop Header: Depth=1
                                        #     Child Loop BB2_37 Depth 2
                                        #     Child Loop BB2_41 Depth 2
	imul	r11, r10, 88
	vpsrad	xmm8, xmm7, 31
	vpsrld	xmm8, xmm8, 26
	vpaddd	xmm8, xmm8, xmm7
	vpsrad	xmm9, xmm8, 6
	mov	rbx, qword ptr [r8 + r11 + 24]
	vpxor	xmm10, xmm10, xmm10
	vmovdqa	xmm11, xmm1
	vpgatherdd	xmm10, xmmword ptr [rbx + 4*xmm9], xmm11
	mov	rbx, qword ptr [r8 + r11]
	vpxor	xmm11, xmm11, xmm11
	vpcmpeqd	xmm12, xmm12, xmm12
	vpgatherdq	xmm11, xmmword ptr [rbx + 8*xmm9], xmm12
	vpand	xmm8, xmm8, xmm0
	vpsubd	xmm8, xmm7, xmm8
	vpmovzxdq	xmm8, xmm8              # xmm8 = xmm8[0],zero,xmm8[1],zero
	vpsllvq	xmm8, xmm6, xmm8
	vpandn	xmm8, xmm8, xmm11
	vpand	xmm9, xmm8, xmm2
	vpshufb	xmm9, xmm3, xmm9
	vpsrlw	xmm8, xmm8, 4
	vpand	xmm8, xmm8, xmm2
	vpshufb	xmm8, xmm3, xmm8
	vpaddb	xmm8, xmm8, xmm9
	vpsadbw	xmm8, xmm8, xmm5
	vpshufd	xmm8, xmm8, 232                 # xmm8 = xmm8[0,2,2,3]
	vpaddd	xmm8, xmm10, xmm8
	vpsubd	xmm7, xmm7, xmm8
	mov	ebx, r10d
	not	ebx
	add	ebx, edi
	bt	r9d, ebx
	jae	.LBB2_33
# %bb.35:                               #   in Loop: Header=BB2_34 Depth=1
	vpextrd	r14d, xmm7, 1
	test	r14d, r14d
	jle	.LBB2_38
# %bb.36:                               #   in Loop: Header=BB2_34 Depth=1
	mov	r15, qword ptr [r8 + r11 + 64]
	xor	ebx, ebx
	.p2align	4, 0x90
.LBB2_37:                               #   Parent Loop BB2_34 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lea	r12d, [r14 - 1]
	add	rbx, qword ptr [r15 + 8*r12]
	and	r12d, r14d
	mov	r14d, r12d
	jne	.LBB2_37
	jmp	.LBB2_39
	.p2align	4, 0x90
.LBB2_38:                               #   in Loop: Header=BB2_34 Depth=1
	xor	ebx, ebx
.LBB2_39:                               #   in Loop: Header=BB2_34 Depth=1
	vmovd	r15d, xmm7
	test	r15d, r15d
	jle	.LBB2_31
# %bb.40:                               #   in Loop: Header=BB2_34 Depth=1
	mov	r12, qword ptr [r8 + r11 + 64]
	xor	r14d, r14d
	.p2align	4, 0x90
.LBB2_41:                               #   Parent Loop BB2_34 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lea	r13d, [r15 - 1]
	add	r14, qword ptr [r12 + 8*r13]
	and	r13d, r15d
	mov	r15d, r13d
	jne	.LBB2_41
	jmp	.LBB2_32
.LBB2_42:
	xor	r9d, r9d
	vpxor	xmm5, xmm5, xmm5
	vpcmpeqd	xmm6, xmm6, xmm6
	jmp	.LBB2_46
	.p2align	4, 0x90
.LBB2_43:                               #   in Loop: Header=BB2_46 Depth=1
	xor	ebx, ebx
.LBB2_44:                               #   in Loop: Header=BB2_46 Depth=1
	add	r11, r9
	sub	r11, rbx
	vpbroadcastd	xmm4, dword ptr [r8 + r10 + 48]
	vpaddd	xmm4, xmm4, xmm7
	mov	r9, r11
.LBB2_45:                               #   in Loop: Header=BB2_46 Depth=1
	inc	rdx
	cmp	rdx, rsi
	je	.LBB2_55
.LBB2_46:                               # =>This Loop Header: Depth=1
                                        #     Child Loop BB2_49 Depth 2
                                        #     Child Loop BB2_53 Depth 2
	imul	r10, rdx, 88
	vpsrad	xmm7, xmm4, 31
	vpsrld	xmm7, xmm7, 26
	vpaddd	xmm7, xmm4, xmm7
	vpsrad	xmm8, xmm7, 6
	mov	r11, qword ptr [r8 + r10 + 24]
	vmovdqa	xmm9, xmm1
	vpxor	xmm10, xmm10, xmm10
	vpgatherdd	xmm10, xmmword ptr [r11 + 4*xmm8], xmm9
	mov	r11, qword ptr [r8 + r10]
	vpcmpeqd	xmm9, xmm9, xmm9
	vpxor	xmm11, xmm11, xmm11
	vpgatherdq	xmm11, xmmword ptr [r11 + 8*xmm8], xmm9
	vpand	xmm7, xmm7, xmm0
	vpsubd	xmm7, xmm4, xmm7
	vpmovzxdq	xmm7, xmm7              # xmm7 = xmm7[0],zero,xmm7[1],zero
	vpsllvq	xmm7, xmm6, xmm7
	vpandn	xmm7, xmm7, xmm11
	vpand	xmm8, xmm7, xmm2
	vpshufb	xmm8, xmm3, xmm8
	vpsrlw	xmm7, xmm7, 4
	vpand	xmm7, xmm7, xmm2
	vpshufb	xmm7, xmm3, xmm7
	vpaddb	xmm7, xmm8, xmm7
	vpsadbw	xmm7, xmm7, xmm5
	vpshufd	xmm7, xmm7, 232                 # xmm7 = xmm7[0,2,2,3]
	vpaddd	xmm7, xmm10, xmm7
	vpsubd	xmm4, xmm4, xmm7
	mov	r11d, edx
	not	r11d
	add	r11d, edi
	bt	ecx, r11d
	jae	.LBB2_45
# %bb.47:                               #   in Loop: Header=BB2_46 Depth=1
	vpextrd	ebx, xmm4, 1
	test	ebx, ebx
	jle	.LBB2_50
# %bb.48:                               #   in Loop: Header=BB2_46 Depth=1
	mov	r14, qword ptr [r8 + r10 + 64]
	xor	r11d, r11d
	.p2align	4, 0x90
.LBB2_49:                               #   Parent Loop BB2_46 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lea	r15d, [rbx - 1]
	add	r11, qword ptr [r14 + 8*r15]
	and	r15d, ebx
	mov	ebx, r15d
	jne	.LBB2_49
	jmp	.LBB2_51
	.p2align	4, 0x90
.LBB2_50:                               #   in Loop: Header=BB2_46 Depth=1
	xor	r11d, r11d
.LBB2_51:                               #   in Loop: Header=BB2_46 Depth=1
	vmovd	r14d, xmm4
	test	r14d, r14d
	jle	.LBB2_43
# %bb.52:                               #   in Loop: Header=BB2_46 Depth=1
	mov	r15, qword ptr [r8 + r10 + 64]
	xor	ebx, ebx
	.p2align	4, 0x90
.LBB2_53:                               #   Parent Loop BB2_46 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lea	r12d, [r14 - 1]
	add	rbx, qword ptr [r15 + 8*r12]
	and	r12d, r14d
	mov	r14d, r12d
	jne	.LBB2_53
	jmp	.LBB2_44
.Lfunc_end2:
	.size	_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_, .Lfunc_end2-_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0                          # -- Begin function _ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii
.LCPI3_0:
	.long	4294967232                      # 0xffffffc0
	.section	.rodata.cst16,"aM",@progbits,16
	.p2align	4, 0x0
.LCPI3_1:
	.long	4294967295                      # 0xffffffff
	.long	4294967295                      # 0xffffffff
	.long	0                               # 0x0
	.long	0                               # 0x0
.LCPI3_2:
	.zero	16,15
.LCPI3_3:
	.byte	0                               # 0x0
	.byte	1                               # 0x1
	.byte	1                               # 0x1
	.byte	2                               # 0x2
	.byte	1                               # 0x1
	.byte	2                               # 0x2
	.byte	2                               # 0x2
	.byte	3                               # 0x3
	.byte	1                               # 0x1
	.byte	2                               # 0x2
	.byte	2                               # 0x2
	.byte	3                               # 0x3
	.byte	2                               # 0x2
	.byte	3                               # 0x3
	.byte	3                               # 0x3
	.byte	4                               # 0x4
	.section	.rodata,"a",@progbits
.LCPI3_4:
	.byte	15                              # 0xf
	.section	.text._ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii,"axG",@progbits,_ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii,comdat
	.weak	_ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii
	.p2align	4, 0x90
	.type	_ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii,@function
_ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii: # @_ZNK9blueberry21WeightedWaveletMatrixIxxE4lessEiii
	.cfi_startproc
# %bb.0:
	push	rbx
	.cfi_def_cfa_offset 16
	.cfi_offset rbx, -16
                                        # kill: def $edx killed $edx def $rdx
                                        # kill: def $esi killed $esi def $rsi
	mov	rax, qword ptr [rdi + 16]
	sub	rax, qword ptr [rdi + 8]
	shr	rax, 3
	cmp	eax, ecx
	jne	.LBB3_8
# %bb.1:
	test	edx, edx
	jle	.LBB3_22
# %bb.2:
	mov	rcx, qword ptr [rdi + 64]
	xor	eax, eax
	.p2align	4, 0x90
.LBB3_3:                                # =>This Inner Loop Header: Depth=1
	lea	r8d, [rdx - 1]
	add	rax, qword ptr [rcx + 8*r8]
	and	r8d, edx
	mov	edx, r8d
	jne	.LBB3_3
# %bb.4:
	test	esi, esi
	jle	.LBB3_23
.LBB3_5:
	mov	rdx, qword ptr [rdi + 64]
	xor	ecx, ecx
	.p2align	4, 0x90
.LBB3_6:                                # =>This Inner Loop Header: Depth=1
	lea	edi, [rsi - 1]
	add	rcx, qword ptr [rdx + 8*rdi]
	and	edi, esi
	mov	esi, edi
	jne	.LBB3_6
	jmp	.LBB3_7
.LBB3_8:
	mov	r8, qword ptr [rdi + 32]
	mov	rdi, qword ptr [rdi + 40]
	cmp	r8, rdi
	je	.LBB3_24
# %bb.9:
	mov	rax, rdi
	sub	rax, r8
	shr	rax, 3
	imul	r9d, eax, -1171354717
	vmovd	xmm0, edx
	vpinsrd	xmm6, xmm0, esi, 1
	xor	eax, eax
	vpbroadcastd	xmm0, dword ptr [rip + .LCPI3_0] # xmm0 = [4294967232,4294967232,4294967232,4294967232]
	vmovdqa	xmm1, xmmword ptr [rip + .LCPI3_1] # xmm1 = [4294967295,4294967295,0,0]
	vpxor	xmm2, xmm2, xmm2
	vpcmpeqd	xmm3, xmm3, xmm3
	vpbroadcastb	xmm4, byte ptr [rip + .LCPI3_4] # xmm4 = [15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15]
	vmovdqa	xmm5, xmmword ptr [rip + .LCPI3_3] # xmm5 = [0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4]
	jmp	.LBB3_13
	.p2align	4, 0x90
.LBB3_10:                               #   in Loop: Header=BB3_13 Depth=1
	xor	esi, esi
.LBB3_11:                               #   in Loop: Header=BB3_13 Depth=1
	add	rdx, rax
	sub	rdx, rsi
	vpbroadcastd	xmm6, dword ptr [r8 + 48]
	vpaddd	xmm6, xmm7, xmm6
	mov	rax, rdx
.LBB3_12:                               #   in Loop: Header=BB3_13 Depth=1
	add	r8, 88
	cmp	r8, rdi
	je	.LBB3_21
.LBB3_13:                               # =>This Loop Header: Depth=1
                                        #     Child Loop BB3_16 Depth 2
                                        #     Child Loop BB3_20 Depth 2
	vpsrad	xmm7, xmm6, 31
	vpsrld	xmm7, xmm7, 26
	vpaddd	xmm7, xmm6, xmm7
	vpsrad	xmm8, xmm7, 6
	mov	rdx, qword ptr [r8 + 24]
	vmovdqa	xmm9, xmm1
	vpxor	xmm10, xmm10, xmm10
	vpgatherdd	xmm10, xmmword ptr [rdx + 4*xmm8], xmm9
	mov	rdx, qword ptr [r8]
	vpcmpeqd	xmm9, xmm9, xmm9
	vpxor	xmm11, xmm11, xmm11
	vpgatherdq	xmm11, xmmword ptr [rdx + 8*xmm8], xmm9
	dec	r9d
	vpand	xmm7, xmm7, xmm0
	vpsubd	xmm7, xmm6, xmm7
	vpmovzxdq	xmm7, xmm7              # xmm7 = xmm7[0],zero,xmm7[1],zero
	vpsllvq	xmm7, xmm3, xmm7
	vpandn	xmm7, xmm7, xmm11
	vpand	xmm8, xmm7, xmm4
	vpshufb	xmm8, xmm5, xmm8
	vpsrlw	xmm7, xmm7, 4
	vpand	xmm7, xmm7, xmm4
	vpshufb	xmm7, xmm5, xmm7
	vpaddb	xmm7, xmm8, xmm7
	vpsadbw	xmm7, xmm7, xmm2
	vpshufd	xmm7, xmm7, 232                 # xmm7 = xmm7[0,2,2,3]
	vpaddd	xmm7, xmm10, xmm7
	vpsubd	xmm6, xmm6, xmm7
	bt	ecx, r9d
	jae	.LBB3_12
# %bb.14:                               #   in Loop: Header=BB3_13 Depth=1
	vmovd	esi, xmm6
	test	esi, esi
	jle	.LBB3_17
# %bb.15:                               #   in Loop: Header=BB3_13 Depth=1
	mov	r10, qword ptr [r8 + 64]
	xor	edx, edx
	.p2align	4, 0x90
.LBB3_16:                               #   Parent Loop BB3_13 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lea	r11d, [rsi - 1]
	add	rdx, qword ptr [r10 + 8*r11]
	and	r11d, esi
	mov	esi, r11d
	jne	.LBB3_16
	jmp	.LBB3_18
	.p2align	4, 0x90
.LBB3_17:                               #   in Loop: Header=BB3_13 Depth=1
	xor	edx, edx
.LBB3_18:                               #   in Loop: Header=BB3_13 Depth=1
	vpextrd	r10d, xmm6, 1
	test	r10d, r10d
	jle	.LBB3_10
# %bb.19:                               #   in Loop: Header=BB3_13 Depth=1
	mov	r11, qword ptr [r8 + 64]
	xor	esi, esi
	.p2align	4, 0x90
.LBB3_20:                               #   Parent Loop BB3_13 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lea	ebx, [r10 - 1]
	add	rsi, qword ptr [r11 + 8*rbx]
	and	ebx, r10d
	mov	r10d, ebx
	jne	.LBB3_20
	jmp	.LBB3_11
.LBB3_21:
	pop	rbx
	.cfi_def_cfa_offset 8
	ret
.LBB3_22:
	.cfi_def_cfa_offset 16
	xor	eax, eax
	test	esi, esi
	jg	.LBB3_5
.LBB3_23:
	xor	ecx, ecx
.LBB3_7:
	sub	rax, rcx
	pop	rbx
	.cfi_def_cfa_offset 8
	ret
.LBB3_24:
	.cfi_def_cfa_offset 16
	xor	eax, eax
	pop	rbx
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
