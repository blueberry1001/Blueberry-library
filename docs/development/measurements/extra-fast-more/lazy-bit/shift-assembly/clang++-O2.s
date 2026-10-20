	.text
	.file	"probe-affine.cpp"
	.globl	baseline_prod                   # -- Begin function baseline_prod
	.p2align	4, 0x90
	.type	baseline_prod,@function
baseline_prod:                          # @baseline_prod
	.cfi_startproc
# %bb.0:
	cmpl	%edx, %esi
	jne	.LBB0_2
# %bb.1:
	xorl	%eax, %eax
	retq
.LBB0_2:
	movl	80(%rdi), %eax
	decl	%eax
	movslq	%esi, %rcx
	movslq	%edx, %r8
	movl	%eax, %esi
	xorl	%edx, %edx
	jmp	_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm # TAILCALL
.Lfunc_end0:
	.size	baseline_prod, .Lfunc_end0-baseline_prod
	.cfi_endproc
                                        # -- End function
	.globl	shift_prod                      # -- Begin function shift_prod
	.p2align	4, 0x90
	.type	shift_prod,@function
shift_prod:                             # @shift_prod
	.cfi_startproc
# %bb.0:
	cmpl	%edx, %esi
	jne	.LBB1_2
# %bb.1:
	xorl	%eax, %eax
	retq
.LBB1_2:
	movl	80(%rdi), %eax
	decl	%eax
	movslq	%esi, %rcx
	movslq	%edx, %r8
	movl	%eax, %esi
	xorl	%edx, %edx
	jmp	_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm # TAILCALL
.Lfunc_end1:
	.size	shift_prod, .Lfunc_end1-shift_prod
	.cfi_endproc
                                        # -- End function
	.globl	baseline_apply                  # -- Begin function baseline_apply
	.p2align	4, 0x90
	.type	baseline_apply,@function
baseline_apply:                         # @baseline_apply
	.cfi_startproc
# %bb.0:
                                        # kill: def $r8d killed $r8d def $r8
	cmpl	%edx, %esi
	je	.LBB2_4
# %bb.1:
	movl	%ecx, %eax
	movl	%edx, %r10d
	shlq	$32, %rax
	movabsq	$-8525806094425994177, %rcx     # imm = 0x89AE40875DE0CC3F
	mulq	%rcx
	shrq	$29, %rdx
	imull	$998244353, %edx, %r9d          # imm = 0x3B800001
	negl	%r9d
	shlq	$32, %r8
	movq	%r8, %rax
	mulq	%rcx
	shrq	$29, %rdx
	imull	$998244353, %edx, %edx          # imm = 0x3B800001
	negl	%edx
	cmpl	$301989884, %r9d                # imm = 0x11FFFFFC
	jne	.LBB2_3
# %bb.2:
	testl	%edx, %edx
	jne	.LBB2_3
.LBB2_4:
	retq
.LBB2_3:
	pushq	%rax
	.cfi_def_cfa_offset 16
	movl	80(%rdi), %eax
	decl	%eax
	movslq	%esi, %rcx
	movslq	%r10d, %r8
	movl	%edx, (%rsp)
	movl	%eax, %esi
	xorl	%edx, %edx
	callq	_ZN9blueberry10extra_fast22AffineSumSegmentTree166updateEimmmjj
	popq	%rax
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end2:
	.size	baseline_apply, .Lfunc_end2-baseline_apply
	.cfi_endproc
                                        # -- End function
	.globl	shift_apply                     # -- Begin function shift_apply
	.p2align	4, 0x90
	.type	shift_apply,@function
shift_apply:                            # @shift_apply
	.cfi_startproc
# %bb.0:
                                        # kill: def $r8d killed $r8d def $r8
	cmpl	%edx, %esi
	je	.LBB3_4
# %bb.1:
	movl	%ecx, %eax
	movl	%edx, %r10d
	shlq	$32, %rax
	movabsq	$-8525806094425994177, %rcx     # imm = 0x89AE40875DE0CC3F
	mulq	%rcx
	shrq	$29, %rdx
	imull	$998244353, %edx, %r9d          # imm = 0x3B800001
	negl	%r9d
	shlq	$32, %r8
	movq	%r8, %rax
	mulq	%rcx
	shrq	$29, %rdx
	imull	$998244353, %edx, %edx          # imm = 0x3B800001
	negl	%edx
	cmpl	$301989884, %r9d                # imm = 0x11FFFFFC
	jne	.LBB3_3
# %bb.2:
	testl	%edx, %edx
	jne	.LBB3_3
.LBB3_4:
	retq
.LBB3_3:
	pushq	%rax
	.cfi_def_cfa_offset 16
	movl	80(%rdi), %eax
	decl	%eax
	movslq	%esi, %rcx
	movslq	%r10d, %r8
	movl	%edx, (%rsp)
	movl	%eax, %esi
	xorl	%edx, %edx
	callq	_ZN9blueberry10extra_fast18ShiftAffineControl6updateEimmmjj
	popq	%rax
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end3:
	.size	shift_apply, .Lfunc_end3-shift_apply
	.cfi_endproc
                                        # -- End function
	.section	.text._ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm,"axG",@progbits,_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm,comdat
	.weak	_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm # -- Begin function _ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm
	.p2align	4, 0x90
	.type	_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm,@function
_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm: # @_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$56, %rsp
	.cfi_def_cfa_offset 112
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%rdx, %r14
	movslq	%esi, %r11
	movq	184(%rdi,%r11,8), %r13
	leaq	(%rcx,%r13), %rax
	decq	%rax
	movq	%rax, %rdx
	orq	%r13, %rdx
	shrq	$32, %rdx
	je	.LBB4_1
# %bb.2:
	xorl	%edx, %edx
	divq	%r13
	movq	%rax, %rbp
	jmp	.LBB4_3
.LBB4_1:
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%r13d
	movl	%eax, %ebp
.LBB4_3:
	movq	%r8, %rax
	orq	%r13, %rax
	shrq	$32, %rax
	je	.LBB4_4
# %bb.5:
	movq	%r8, %rax
	xorl	%edx, %edx
	divq	%r13
	movq	%rdx, %r9
	movq	%rax, %r10
	jmp	.LBB4_6
.LBB4_4:
	movl	%r8d, %eax
	xorl	%edx, %edx
	divl	%r13d
	movl	%edx, %r9d
	movl	%eax, %r10d
.LBB4_6:
	xorl	%r12d, %r12d
	cmpl	%r10d, %ebp
	jae	.LBB4_20
# %bb.7:
	movq	%r14, 16(%rsp)                  # 8-byte Spill
	shlq	$4, %r14
	addq	88(%rdi,%r11,8), %r14
	cmpb	$0, 76(%rdi)
	je	.LBB4_8
# %bb.15:
	xorl	%r12d, %r12d
	cmpl	$7, %ebp
	movq	%r9, 32(%rsp)                   # 8-byte Spill
	movq	%r10, 24(%rsp)                  # 8-byte Spill
	ja	.LBB4_17
# %bb.16:
	movq	%rdi, %rbx
	leaq	(,%r14,4), %rdi
	addq	(%rbx), %rdi
	cmpl	$8, %r10d
	movl	$8, %edx
	cmovbl	%r10d, %edx
	movl	%esi, 12(%rsp)                  # 4-byte Spill
	movl	%ebp, %esi
	movq	%r8, %r15
	movq	%rcx, %r12
	callq	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
	movq	%r12, %rcx
	movq	24(%rsp), %r10                  # 8-byte Reload
	movq	%r15, %r8
	movq	32(%rsp), %r9                   # 8-byte Reload
	movq	%rbx, %rdi
	movl	12(%rsp), %esi                  # 4-byte Reload
	movl	%eax, %r12d
.LBB4_17:
	cmpl	$9, %r10d
	jb	.LBB4_19
# %bb.18:
	movq	(%rdi), %rax
	movq	%rdi, %rbx
	leaq	(%rax,%r14,4), %rdi
	addq	$32, %rdi
	xorl	%eax, %eax
	subl	$8, %ebp
	cmovbl	%eax, %ebp
	leal	-8(%r10), %edx
	movl	%esi, %eax
	movl	%ebp, %esi
	movl	%eax, %ebp
	movq	%r8, %r15
	movq	%rcx, %r14
	callq	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
	movq	%r14, %rcx
	movq	24(%rsp), %r10                  # 8-byte Reload
	movq	%r15, %r8
	movq	32(%rsp), %r9                   # 8-byte Reload
	movq	%rbx, %rdi
	movl	%ebp, %esi
                                        # kill: def $eax killed $eax def $rax
	leal	(%rax,%r12), %edx
	cmpl	$998244353, %edx                # imm = 0x3B800001
	leal	-998244353(%rax,%r12), %r12d
	cmovbl	%edx, %r12d
	jmp	.LBB4_19
.LBB4_8:
	shlq	$2, %r14
	addq	(%rdi), %r14
	movl	%ebp, %eax
	movl	%r10d, %ebp
	movq	%rbp, %rdx
	subq	%rax, %rdx
	cmpq	$4, %rdx
	jae	.LBB4_11
# %bb.9:
	xorl	%r12d, %r12d
	jmp	.LBB4_10
.LBB4_11:
	movq	%rcx, 40(%rsp)                  # 8-byte Spill
	movq	%r10, %rcx
	movq	%r8, %r15
	movq	%r9, %rbx
	movq	%rdi, %r11
	movl	%esi, %r10d
	movq	%rdx, %rsi
	andq	$-4, %rsi
	leaq	(%r14,%rax,4), %rdi
	addq	%rsi, %rax
	pxor	%xmm0, %xmm0
	xorl	%r8d, %r8d
	movq	%rsi, %r9
	pxor	%xmm2, %xmm2
	pxor	%xmm1, %xmm1
	.p2align	4, 0x90
.LBB4_12:                               # =>This Inner Loop Header: Depth=1
	movq	(%rdi,%r8,4), %xmm3             # xmm3 = mem[0],zero
	movq	8(%rdi,%r8,4), %xmm4            # xmm4 = mem[0],zero
	punpckldq	%xmm0, %xmm3            # xmm3 = xmm3[0],xmm0[0],xmm3[1],xmm0[1]
	paddq	%xmm3, %xmm2
	punpckldq	%xmm0, %xmm4            # xmm4 = xmm4[0],xmm0[0],xmm4[1],xmm0[1]
	paddq	%xmm4, %xmm1
	addq	$4, %r8
	addq	$-4, %r9
	jne	.LBB4_12
# %bb.13:
	paddq	%xmm2, %xmm1
	pshufd	$238, %xmm1, %xmm0              # xmm0 = xmm1[2,3,2,3]
	paddq	%xmm1, %xmm0
	movq	%xmm0, %r12
	cmpq	%rsi, %rdx
	movl	%r10d, %esi
	movq	%r11, %rdi
	movq	%rbx, %r9
	movq	%r15, %r8
	movq	%rcx, %r10
	movq	40(%rsp), %rcx                  # 8-byte Reload
	je	.LBB4_14
	.p2align	4, 0x90
.LBB4_10:                               # =>This Inner Loop Header: Depth=1
	movl	(%r14,%rax,4), %edx
	addq	%rdx, %r12
	incq	%rax
	cmpq	%rax, %rbp
	jne	.LBB4_10
.LBB4_14:
	movabsq	$-8525806094425994177, %rdx     # imm = 0x89AE40875DE0CC3F
	movq	%r12, %rax
	mulq	%rdx
	shrq	$29, %rdx
	imull	$998244353, %edx, %eax          # imm = 0x3B800001
	subl	%eax, %r12d
.LBB4_19:
	movq	16(%rsp), %r14                  # 8-byte Reload
.LBB4_20:
	testl	%esi, %esi
	je	.LBB4_30
# %bb.21:
	movq	%rcx, %rax
	orq	%r13, %rax
	shrq	$32, %rax
	je	.LBB4_22
# %bb.23:
	movq	%rcx, %rax
	xorl	%edx, %edx
	divq	%r13
	movq	%rdx, %rbp
	movq	%rax, %rbx
	jmp	.LBB4_24
.LBB4_22:
	movl	%ecx, %eax
	xorl	%edx, %edx
	divl	%r13d
	movl	%edx, %ebp
	movl	%eax, %ebx
.LBB4_24:
	testq	%rbp, %rbp
	movq	%rdi, 48(%rsp)                  # 8-byte Spill
	movl	%esi, 12(%rsp)                  # 4-byte Spill
	je	.LBB4_26
# %bb.25:
	leal	1(%rbx), %eax
	imulq	%r13, %rax
	movl	%ebx, %ecx
	imulq	%r13, %rcx
	movq	%r9, %r15
	movq	%r8, %r9
	subq	%rcx, %r9
	cmpq	%r8, %rax
	cmovbeq	%r13, %r9
	movq	%r14, %rdx
	movl	%ebx, %ecx
	movq	%rbp, %r8
	movq	%r10, %r13
	callq	_ZNK9blueberry10extra_fast22AffineSumSegmentTree1611query_childEimjmm
	movq	%r13, %r10
	movq	%r15, %r9
                                        # kill: def $eax killed $eax def $rax
	leal	(%rax,%r12), %ecx
	cmpl	$998244353, %ecx                # imm = 0x3B800001
	leal	-998244353(%rax,%r12), %r12d
	cmovbl	%ecx, %r12d
.LBB4_26:
	testq	%r9, %r9
	je	.LBB4_30
# %bb.27:
	testq	%rbp, %rbp
	je	.LBB4_29
# %bb.28:
	cmpl	%r10d, %ebx
	je	.LBB4_30
.LBB4_29:
	movq	48(%rsp), %rdi                  # 8-byte Reload
	movl	12(%rsp), %esi                  # 4-byte Reload
	movq	%r14, %rdx
	movl	%r10d, %ecx
	xorl	%r8d, %r8d
	callq	_ZNK9blueberry10extra_fast22AffineSumSegmentTree1611query_childEimjmm
                                        # kill: def $eax killed $eax def $rax
	leal	(%rax,%r12), %ecx
	cmpl	$998244353, %ecx                # imm = 0x3B800001
	leal	-998244353(%rax,%r12), %r12d
	cmovbl	%ecx, %r12d
.LBB4_30:
	movl	%r12d, %eax
	addq	$56, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end4:
	.size	_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm, .Lfunc_end4-_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm
	.cfi_endproc
                                        # -- End function
	.section	.text._ZNK9blueberry10extra_fast22AffineSumSegmentTree1611query_childEimjmm,"axG",@progbits,_ZNK9blueberry10extra_fast22AffineSumSegmentTree1611query_childEimjmm,comdat
	.weak	_ZNK9blueberry10extra_fast22AffineSumSegmentTree1611query_childEimjmm # -- Begin function _ZNK9blueberry10extra_fast22AffineSumSegmentTree1611query_childEimjmm
	.p2align	4, 0x90
	.type	_ZNK9blueberry10extra_fast22AffineSumSegmentTree1611query_childEimjmm,@function
_ZNK9blueberry10extra_fast22AffineSumSegmentTree1611query_childEimjmm: # @_ZNK9blueberry10extra_fast22AffineSumSegmentTree1611query_childEimjmm
	.cfi_startproc
# %bb.0:
	pushq	%r15
	.cfi_def_cfa_offset 16
	pushq	%r14
	.cfi_def_cfa_offset 24
	pushq	%r13
	.cfi_def_cfa_offset 32
	pushq	%r12
	.cfi_def_cfa_offset 40
	pushq	%rbx
	.cfi_def_cfa_offset 48
	.cfi_offset %rbx, -48
	.cfi_offset %r12, -40
	.cfi_offset %r13, -32
	.cfi_offset %r14, -24
	.cfi_offset %r15, -16
	movq	%r9, %rbx
	movq	%r8, %r14
	movl	%esi, %r15d
	movq	%rdi, %r12
	leal	-1(%r15), %esi
	shlq	$4, %rdx
	movl	%ecx, %r13d
	addq	%rdx, %r13
	movq	%r13, %rdx
	movq	%r8, %rcx
	movq	%r9, %r8
	callq	_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm
	movslq	%r15d, %rcx
	addq	88(%r12,%rcx,8), %r13
	subq	96(%r12), %r13
	movq	24(%r12), %rcx
	movq	48(%r12), %rsi
	movl	(%rcx,%r13,4), %edx
	movl	(%rsi,%r13,4), %ecx
	cmpq	$301989884, %rdx                # imm = 0x11FFFFFC
	jne	.LBB5_2
# %bb.1:
	testl	%ecx, %ecx
	je	.LBB5_3
.LBB5_2:
	movl	%eax, %eax
	imulq	%rax, %rdx
	imull	$998244351, %edx, %eax          # imm = 0x3B7FFFFF
	imulq	$998244353, %rax, %rsi          # imm = 0x3B800001
	addq	%rdx, %rsi
	shrq	$32, %rsi
	leal	-998244353(%rsi), %eax
	cmpl	$998244353, %esi                # imm = 0x3B800001
	cmovbl	%esi, %eax
	subl	%r14d, %ebx
	imulq	$324829455, %rbx, %rdx          # imm = 0x135C810F
	shrq	$32, %rdx
	movl	%ebx, %esi
	subl	%edx, %esi
	shrl	%esi
	addl	%edx, %esi
	shrl	$29, %esi
	imull	$998244353, %esi, %edx          # imm = 0x3B800001
	subl	%edx, %ebx
	imulq	%rcx, %rbx
	imull	$998244351, %ebx, %ecx          # imm = 0x3B7FFFFF
	imulq	$998244353, %rcx, %rcx          # imm = 0x3B800001
	addq	%rbx, %rcx
	shrq	$32, %rcx
	leal	-998244353(%rcx), %edx
	cmpl	$998244353, %ecx                # imm = 0x3B800001
	cmovbl	%ecx, %edx
	leal	(%rdx,%rax), %ecx
	cmpl	$998244353, %ecx                # imm = 0x3B800001
	leal	-998244353(%rdx,%rax), %eax
	cmovbl	%ecx, %eax
.LBB5_3:
	popq	%rbx
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end5:
	.size	_ZNK9blueberry10extra_fast22AffineSumSegmentTree1611query_childEimjmm, .Lfunc_end5-_ZNK9blueberry10extra_fast22AffineSumSegmentTree1611query_childEimjmm
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst32,"aM",@progbits,32
	.p2align	5, 0x0                          # -- Begin function _ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
.LCPI6_0:
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	2                               # 0x2
	.long	3                               # 0x3
	.long	4                               # 0x4
	.long	5                               # 0x5
	.long	6                               # 0x6
	.long	7                               # 0x7
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0
.LCPI6_1:
	.long	3296722943                      # 0xc47fffff
	.section	.text._ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj,"axG",@progbits,_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj,comdat
	.weak	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
	.p2align	4, 0x90
	.type	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj,@function
_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj: # @_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
	.cfi_startproc
# %bb.0:
	vmovd	%esi, %xmm0
	vpbroadcastd	%xmm0, %ymm0
	vmovdqa	.LCPI6_0(%rip), %ymm1           # ymm1 = [0,1,2,3,4,5,6,7]
	vpcmpgtd	%ymm1, %ymm0, %ymm0
	vmovd	%edx, %xmm2
	vpbroadcastd	%xmm2, %ymm2
	vpcmpgtd	%ymm1, %ymm2, %ymm1
	vpandn	%ymm1, %ymm0, %ymm0
	vpand	(%rdi), %ymm0, %ymm0
	vextracti128	$1, %ymm0, %xmm1
	vpaddd	%xmm1, %xmm0, %xmm0
	vpbroadcastd	.LCPI6_1(%rip), %xmm1   # xmm1 = [3296722943,3296722943,3296722943,3296722943]
	vpaddd	%xmm1, %xmm0, %xmm2
	vpminud	%xmm2, %xmm0, %xmm0
	vpshufd	$78, %xmm0, %xmm2               # xmm2 = xmm0[2,3,0,1]
	vpaddd	%xmm0, %xmm2, %xmm0
	vpaddd	%xmm1, %xmm0, %xmm2
	vpminud	%xmm2, %xmm0, %xmm0
	vpshufd	$177, %xmm0, %xmm2              # xmm2 = xmm0[1,0,3,2]
	vpaddd	%xmm0, %xmm2, %xmm0
	vpaddd	%xmm1, %xmm0, %xmm1
	vpminud	%xmm1, %xmm0, %xmm0
	vmovd	%xmm0, %eax
	vzeroupper
	retq
.Lfunc_end6:
	.size	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj, .Lfunc_end6-_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
	.cfi_endproc
                                        # -- End function
	.section	.text._ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm,"axG",@progbits,_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm,comdat
	.weak	_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm # -- Begin function _ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm
	.p2align	4, 0x90
	.type	_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm,@function
_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm: # @_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$40, %rsp
	.cfi_def_cfa_offset 96
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%rcx, %r11
	movl	%esi, %r10d
	leal	(,%r10,4), %r13d
	movl	$1, %r12d
	movl	%r13d, %ecx
	shlq	%cl, %r12
	leaq	(%r12,%r11), %rbx
	decq	%rbx
	shrq	%cl, %rbx
	movq	%r8, %r15
	shrq	%cl, %r15
	movq	%rdx, %r9
	xorl	%ebp, %ebp
	cmpl	%r15d, %ebx
	movq	%rdx, 24(%rsp)                  # 8-byte Spill
	movq	%rdi, (%rsp)                    # 8-byte Spill
	jae	.LBB7_14
# %bb.1:
	movslq	%r10d, %rax
	movq	88(%rdi,%rax,8), %rbp
	movq	%r9, %r14
	shlq	$4, %r14
	addq	%rbp, %r14
	cmpb	$0, 76(%rdi)
	je	.LBB7_2
# %bb.10:
	xorl	%ebp, %ebp
	cmpl	$7, %ebx
	movq	%r15, 32(%rsp)                  # 8-byte Spill
	ja	.LBB7_12
# %bb.11:
	movq	%rdi, %rax
	leaq	(,%r14,4), %rdi
	addq	(%rax), %rdi
	cmpl	$8, %r15d
	movl	$8, %edx
	cmovbl	%r15d, %edx
	movq	%r8, %r15
	movl	%ebx, %esi
	movq	%r10, 16(%rsp)                  # 8-byte Spill
	movq	%r11, %rbp
	callq	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
	movq	%r15, %r8
	movq	32(%rsp), %r15                  # 8-byte Reload
	movq	(%rsp), %rdi                    # 8-byte Reload
	movq	24(%rsp), %r9                   # 8-byte Reload
	movq	%rbp, %r11
	movq	16(%rsp), %r10                  # 8-byte Reload
	movl	%eax, %ebp
.LBB7_12:
	cmpl	$9, %r15d
	jb	.LBB7_14
# %bb.13:
	movq	(%rdi), %rax
	leaq	(%rax,%r14,4), %rdi
	addq	$32, %rdi
	xorl	%eax, %eax
	subl	$8, %ebx
	cmovbl	%eax, %ebx
	leal	-8(%r15), %edx
	movq	%r8, %r15
	movl	%ebx, %esi
	movq	%r10, %rbx
	movq	%r11, %r14
	callq	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
	movq	%r15, %r8
	movq	32(%rsp), %r15                  # 8-byte Reload
	movq	(%rsp), %rdi                    # 8-byte Reload
	movq	24(%rsp), %r9                   # 8-byte Reload
	movq	%r14, %r11
	movq	%rbx, %r10
                                        # kill: def $eax killed $eax def $rax
	leal	(%rax,%rbp), %ecx
	cmpl	$998244353, %ecx                # imm = 0x3B800001
	leal	-998244353(%rax,%rbp), %ebp
	cmovbl	%ecx, %ebp
.LBB7_14:
	testl	%r10d, %r10d
	je	.LBB7_21
.LBB7_15:
	leaq	-1(%r12), %rbx
	movq	%r11, %r14
	movl	%r13d, %ecx
	shrq	%cl, %r14
	andq	%rbx, %r11
	movq	%r10, 16(%rsp)                  # 8-byte Spill
	movq	%r11, 8(%rsp)                   # 8-byte Spill
	je	.LBB7_17
# %bb.16:
	leal	1(%r14), %eax
	movl	%r13d, %ecx
	shlq	%cl, %rax
	movl	%r14d, %edx
	shlq	%cl, %rdx
	movq	%r8, %rcx
	subq	%rdx, %rcx
	cmpq	%r8, %rax
	cmovaq	%rcx, %r12
	movq	%r15, %r13
	movq	%r8, %r15
	movl	%r10d, %esi
	movq	%r9, %rdx
	movl	%r14d, %ecx
	movq	%r11, %r8
	movq	%r12, %r9
	callq	_ZNK9blueberry10extra_fast18ShiftAffineControl11query_childEimjmm
	movq	%r15, %r8
	movq	%r13, %r15
                                        # kill: def $eax killed $eax def $rax
	leal	(%rax,%rbp), %ecx
	cmpl	$998244353, %ecx                # imm = 0x3B800001
	leal	-998244353(%rax,%rbp), %ebp
	cmovbl	%ecx, %ebp
.LBB7_17:
	andq	%r8, %rbx
	je	.LBB7_21
# %bb.18:
	cmpq	$0, 8(%rsp)                     # 8-byte Folded Reload
	je	.LBB7_20
# %bb.19:
	cmpl	%r15d, %r14d
	je	.LBB7_21
.LBB7_20:
	movq	(%rsp), %rdi                    # 8-byte Reload
	movq	16(%rsp), %rsi                  # 8-byte Reload
                                        # kill: def $esi killed $esi killed $rsi
	movq	24(%rsp), %rdx                  # 8-byte Reload
	movl	%r15d, %ecx
	xorl	%r8d, %r8d
	movq	%rbx, %r9
	callq	_ZNK9blueberry10extra_fast18ShiftAffineControl11query_childEimjmm
                                        # kill: def $eax killed $eax def $rax
	leal	(%rax,%rbp), %ecx
	cmpl	$998244353, %ecx                # imm = 0x3B800001
	leal	-998244353(%rax,%rbp), %ebp
	cmovbl	%ecx, %ebp
	jmp	.LBB7_21
.LBB7_2:
	movq	%r8, 8(%rsp)                    # 8-byte Spill
	movq	(%rdi), %rdx
	movl	%ebx, %eax
	movl	%r15d, %ecx
	movq	%rcx, %rsi
	subq	%rax, %rsi
	cmpq	$4, %rsi
	jae	.LBB7_4
# %bb.3:
	xorl	%ebp, %ebp
	jmp	.LBB7_7
.LBB7_4:
	movq	%rsi, %rdi
	andq	$-4, %rdi
	movq	%r9, %rbx
	shlq	$6, %r9
	leaq	(%r9,%rbp,4), %r8
	leaq	(%r8,%rax,4), %r8
	addq	%rdi, %rax
	addq	%rdx, %r8
	addq	$8, %r8
	pxor	%xmm1, %xmm1
	xorl	%r9d, %r9d
	pxor	%xmm2, %xmm2
	pxor	%xmm0, %xmm0
	.p2align	4, 0x90
.LBB7_5:                                # =>This Inner Loop Header: Depth=1
	movq	-8(%r8,%r9,4), %xmm3            # xmm3 = mem[0],zero
	movq	(%r8,%r9,4), %xmm4              # xmm4 = mem[0],zero
	punpckldq	%xmm1, %xmm3            # xmm3 = xmm3[0],xmm1[0],xmm3[1],xmm1[1]
	paddq	%xmm3, %xmm2
	punpckldq	%xmm1, %xmm4            # xmm4 = xmm4[0],xmm1[0],xmm4[1],xmm1[1]
	paddq	%xmm4, %xmm0
	addq	$4, %r9
	cmpq	%r9, %rdi
	jne	.LBB7_5
# %bb.6:
	paddq	%xmm2, %xmm0
	pshufd	$238, %xmm0, %xmm1              # xmm1 = xmm0[2,3,2,3]
	paddq	%xmm0, %xmm1
	movq	%xmm1, %rbp
	cmpq	%rdi, %rsi
	movq	%rbx, %r9
	je	.LBB7_9
.LBB7_7:
	leaq	(%rdx,%r14,4), %rdx
	.p2align	4, 0x90
.LBB7_8:                                # =>This Inner Loop Header: Depth=1
	movl	(%rdx,%rax,4), %esi
	addq	%rsi, %rbp
	incq	%rax
	cmpq	%rax, %rcx
	jne	.LBB7_8
.LBB7_9:
	movabsq	$-8525806094425994177, %rcx     # imm = 0x89AE40875DE0CC3F
	movq	%rbp, %rax
	mulq	%rcx
	shrq	$29, %rdx
	imull	$998244353, %edx, %eax          # imm = 0x3B800001
	subl	%eax, %ebp
	movq	(%rsp), %rdi                    # 8-byte Reload
	movq	8(%rsp), %r8                    # 8-byte Reload
	testl	%r10d, %r10d
	jne	.LBB7_15
.LBB7_21:
	movl	%ebp, %eax
	addq	$40, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end7:
	.size	_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm, .Lfunc_end7-_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm
	.cfi_endproc
                                        # -- End function
	.section	.text._ZNK9blueberry10extra_fast18ShiftAffineControl11query_childEimjmm,"axG",@progbits,_ZNK9blueberry10extra_fast18ShiftAffineControl11query_childEimjmm,comdat
	.weak	_ZNK9blueberry10extra_fast18ShiftAffineControl11query_childEimjmm # -- Begin function _ZNK9blueberry10extra_fast18ShiftAffineControl11query_childEimjmm
	.p2align	4, 0x90
	.type	_ZNK9blueberry10extra_fast18ShiftAffineControl11query_childEimjmm,@function
_ZNK9blueberry10extra_fast18ShiftAffineControl11query_childEimjmm: # @_ZNK9blueberry10extra_fast18ShiftAffineControl11query_childEimjmm
	.cfi_startproc
# %bb.0:
	pushq	%r15
	.cfi_def_cfa_offset 16
	pushq	%r14
	.cfi_def_cfa_offset 24
	pushq	%r13
	.cfi_def_cfa_offset 32
	pushq	%r12
	.cfi_def_cfa_offset 40
	pushq	%rbx
	.cfi_def_cfa_offset 48
	.cfi_offset %rbx, -48
	.cfi_offset %r12, -40
	.cfi_offset %r13, -32
	.cfi_offset %r14, -24
	.cfi_offset %r15, -16
	movq	%r9, %rbx
	movq	%r8, %r14
	movl	%esi, %r15d
	movq	%rdi, %r12
	leal	-1(%r15), %esi
	shlq	$4, %rdx
	movl	%ecx, %r13d
	addq	%rdx, %r13
	movq	%r13, %rdx
	movq	%r8, %rcx
	movq	%r9, %r8
	callq	_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm
	movslq	%r15d, %rcx
	addq	88(%r12,%rcx,8), %r13
	subq	96(%r12), %r13
	movq	24(%r12), %rcx
	movq	48(%r12), %rsi
	movl	(%rcx,%r13,4), %edx
	movl	(%rsi,%r13,4), %ecx
	cmpq	$301989884, %rdx                # imm = 0x11FFFFFC
	jne	.LBB8_2
# %bb.1:
	testl	%ecx, %ecx
	je	.LBB8_3
.LBB8_2:
	movl	%eax, %eax
	imulq	%rax, %rdx
	imull	$998244351, %edx, %eax          # imm = 0x3B7FFFFF
	imulq	$998244353, %rax, %rsi          # imm = 0x3B800001
	addq	%rdx, %rsi
	shrq	$32, %rsi
	leal	-998244353(%rsi), %eax
	cmpl	$998244353, %esi                # imm = 0x3B800001
	cmovbl	%esi, %eax
	subl	%r14d, %ebx
	imulq	$324829455, %rbx, %rdx          # imm = 0x135C810F
	shrq	$32, %rdx
	movl	%ebx, %esi
	subl	%edx, %esi
	shrl	%esi
	addl	%edx, %esi
	shrl	$29, %esi
	imull	$998244353, %esi, %edx          # imm = 0x3B800001
	subl	%edx, %ebx
	imulq	%rcx, %rbx
	imull	$998244351, %ebx, %ecx          # imm = 0x3B7FFFFF
	imulq	$998244353, %rcx, %rcx          # imm = 0x3B800001
	addq	%rbx, %rcx
	shrq	$32, %rcx
	leal	-998244353(%rcx), %edx
	cmpl	$998244353, %ecx                # imm = 0x3B800001
	cmovbl	%ecx, %edx
	leal	(%rdx,%rax), %ecx
	cmpl	$998244353, %ecx                # imm = 0x3B800001
	leal	-998244353(%rdx,%rax), %eax
	cmovbl	%ecx, %eax
.LBB8_3:
	popq	%rbx
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end8:
	.size	_ZNK9blueberry10extra_fast18ShiftAffineControl11query_childEimjmm, .Lfunc_end8-_ZNK9blueberry10extra_fast18ShiftAffineControl11query_childEimjmm
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst32,"aM",@progbits,32
	.p2align	5, 0x0                          # -- Begin function _ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
.LCPI9_0:
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	2                               # 0x2
	.long	3                               # 0x3
	.long	4                               # 0x4
	.long	5                               # 0x5
	.long	6                               # 0x6
	.long	7                               # 0x7
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0
.LCPI9_1:
	.long	3296722943                      # 0xc47fffff
	.section	.text._ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj,"axG",@progbits,_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj,comdat
	.weak	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
	.p2align	4, 0x90
	.type	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj,@function
_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj: # @_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
	.cfi_startproc
# %bb.0:
	vmovd	%esi, %xmm0
	vpbroadcastd	%xmm0, %ymm0
	vmovdqa	.LCPI9_0(%rip), %ymm1           # ymm1 = [0,1,2,3,4,5,6,7]
	vpcmpgtd	%ymm1, %ymm0, %ymm0
	vmovd	%edx, %xmm2
	vpbroadcastd	%xmm2, %ymm2
	vpcmpgtd	%ymm1, %ymm2, %ymm1
	vpandn	%ymm1, %ymm0, %ymm0
	vpand	(%rdi), %ymm0, %ymm0
	vextracti128	$1, %ymm0, %xmm1
	vpaddd	%xmm1, %xmm0, %xmm0
	vpbroadcastd	.LCPI9_1(%rip), %xmm1   # xmm1 = [3296722943,3296722943,3296722943,3296722943]
	vpaddd	%xmm1, %xmm0, %xmm2
	vpminud	%xmm2, %xmm0, %xmm0
	vpshufd	$78, %xmm0, %xmm2               # xmm2 = xmm0[2,3,0,1]
	vpaddd	%xmm0, %xmm2, %xmm0
	vpaddd	%xmm1, %xmm0, %xmm2
	vpminud	%xmm2, %xmm0, %xmm0
	vpshufd	$177, %xmm0, %xmm2              # xmm2 = xmm0[1,0,3,2]
	vpaddd	%xmm0, %xmm2, %xmm0
	vpaddd	%xmm1, %xmm0, %xmm1
	vpminud	%xmm1, %xmm0, %xmm0
	vmovd	%xmm0, %eax
	vzeroupper
	retq
.Lfunc_end9:
	.size	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj, .Lfunc_end9-_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
	.cfi_endproc
                                        # -- End function
	.section	.text._ZN9blueberry10extra_fast22AffineSumSegmentTree166updateEimmmjj,"axG",@progbits,_ZN9blueberry10extra_fast22AffineSumSegmentTree166updateEimmmjj,comdat
	.weak	_ZN9blueberry10extra_fast22AffineSumSegmentTree166updateEimmmjj # -- Begin function _ZN9blueberry10extra_fast22AffineSumSegmentTree166updateEimmmjj
	.p2align	4, 0x90
	.type	_ZN9blueberry10extra_fast22AffineSumSegmentTree166updateEimmmjj,@function
_ZN9blueberry10extra_fast22AffineSumSegmentTree166updateEimmmjj: # @_ZN9blueberry10extra_fast22AffineSumSegmentTree166updateEimmmjj
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$88, %rsp
	.cfi_def_cfa_offset 144
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movl	%r9d, 28(%rsp)                  # 4-byte Spill
	movq	%r8, %r9
	movq	%rcx, %r12
	movq	%rdx, %rbp
                                        # kill: def $esi killed $esi def $rsi
	movq	%rdi, %r14
	movq	%rsi, 16(%rsp)                  # 8-byte Spill
	movslq	%esi, %rax
	movq	%rax, 32(%rsp)                  # 8-byte Spill
	movq	184(%rdi,%rax,8), %r13
	leaq	(%rcx,%r13), %rax
	decq	%rax
	movq	%rax, %rcx
	orq	%r13, %rcx
	shrq	$32, %rcx
	je	.LBB10_1
# %bb.2:
	xorl	%edx, %edx
	divq	%r13
	movq	%rax, %rcx
	jmp	.LBB10_3
.LBB10_1:
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%r13d
	movl	%eax, %ecx
.LBB10_3:
	movq	%r9, %rax
	orq	%r13, %rax
	shrq	$32, %rax
	je	.LBB10_4
# %bb.5:
	movq	%r9, %rax
	xorl	%edx, %edx
	divq	%r13
	movq	%rdx, 40(%rsp)                  # 8-byte Spill
	movq	%rax, %r8
	cmpl	%r8d, %ecx
	jae	.LBB10_8
.LBB10_7:
	movl	144(%rsp), %eax
	movl	%eax, (%rsp)
	movq	%r14, %rdi
	movq	16(%rsp), %rsi                  # 8-byte Reload
                                        # kill: def $esi killed $esi killed $rsi
	movq	%rbp, %rdx
                                        # kill: def $ecx killed $ecx killed $rcx
	movq	%r14, %rbx
	movq	%rbp, %r15
	movq	%r8, %rbp
	movq	%r9, %r14
	movl	28(%rsp), %r9d                  # 4-byte Reload
	callq	_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj
	movq	%r14, %r9
	movq	%rbp, %r8
	movq	%r15, %rbp
	movq	%rbx, %r14
.LBB10_8:
	cmpl	$0, 16(%rsp)                    # 4-byte Folded Reload
	je	.LBB10_30
# %bb.9:
	movq	%r12, %rax
	orq	%r13, %rax
	shrq	$32, %rax
	je	.LBB10_10
# %bb.11:
	movq	%r12, %rax
	xorl	%edx, %edx
	divq	%r13
	movq	%rdx, %r10
	testq	%r10, %r10
	jne	.LBB10_13
	jmp	.LBB10_20
.LBB10_4:
	movl	%r9d, %eax
	xorl	%edx, %edx
	divl	%r13d
                                        # kill: def $edx killed $edx def $rdx
	movq	%rdx, 40(%rsp)                  # 8-byte Spill
	movl	%eax, %r8d
	cmpl	%r8d, %ecx
	jb	.LBB10_7
	jmp	.LBB10_8
.LBB10_10:
	movl	%r12d, %eax
	xorl	%edx, %edx
	divl	%r13d
	movl	%edx, %r10d
                                        # kill: def $eax killed $eax def $rax
	testq	%r10, %r10
	je	.LBB10_20
.LBB10_13:
	movq	%r8, 48(%rsp)                   # 8-byte Spill
	leal	1(%rax), %esi
	imulq	%r13, %rsi
	movq	%rax, 80(%rsp)                  # 8-byte Spill
	movl	%eax, %r15d
	movq	%r15, %rcx
	imulq	%r13, %rcx
	movq	%rbp, %rdx
	movq	%r9, %rbp
	subq	%rcx, %rbp
	cmpq	%r9, %rsi
	cmovbeq	%r13, %rbp
	movq	%rdx, 56(%rsp)                  # 8-byte Spill
	shlq	$4, %rdx
	movq	%rdx, 72(%rsp)                  # 8-byte Spill
	leaq	(%r15,%rdx), %rbx
	movq	32(%rsp), %rax                  # 8-byte Reload
	movq	88(%r14,%rax,8), %r13
	addq	%rbx, %r13
	subq	96(%r14), %r13
	movq	24(%r14), %rax
	movq	48(%r14), %rcx
	movl	(%rax,%r13,4), %r9d
	movl	(%rcx,%r13,4), %eax
	movq	16(%rsp), %rcx                  # 8-byte Reload
	leal	-1(%rcx), %r12d
	cmpl	$301989884, %r9d                # imm = 0x11FFFFFC
	jne	.LBB10_15
# %bb.14:
	testl	%eax, %eax
	je	.LBB10_16
.LBB10_15:
	movl	%eax, (%rsp)
	movq	%r14, %rdi
	movl	%r12d, %esi
	movq	%rbx, %rdx
	xorl	%ecx, %ecx
	movl	$16, %r8d
	movq	%r15, 64(%rsp)                  # 8-byte Spill
	movq	%r10, %r15
	callq	_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj
	movq	%r15, %r10
	movq	64(%rsp), %r15                  # 8-byte Reload
	movq	24(%r14), %rax
	movl	$301989884, (%rax,%r13,4)       # imm = 0x11FFFFFC
	movq	48(%r14), %rax
	movl	$0, (%rax,%r13,4)
.LBB10_16:
	movl	144(%rsp), %eax
	movl	%eax, (%rsp)
	movq	%r14, %rdi
	movl	%r12d, %esi
	movq	%rbx, %rdx
	movq	%r10, %r13
	movq	%r10, %rcx
	movq	%rbp, %r8
	movl	28(%rsp), %r9d                  # 4-byte Reload
	callq	_ZN9blueberry10extra_fast22AffineSumSegmentTree166updateEimmmjj
	movslq	%r12d, %rax
	shlq	$4, %rbx
	addq	88(%r14,%rax,8), %rbx
	cmpb	$0, 76(%r14)
	movq	(%r14), %rsi
	leaq	(%rsi,%rbx,4), %rdi
	je	.LBB10_17
# %bb.18:
	xorl	%esi, %esi
	movl	$8, %edx
	callq	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
	movl	%eax, %r12d
	movq	(%r14), %rax
	leaq	(%rax,%rbx,4), %rdi
	addq	$32, %rdi
	xorl	%esi, %esi
	movl	$8, %edx
	callq	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
                                        # kill: def $eax killed $eax def $rax
	leal	(%rax,%r12), %edx
	cmpl	$998244353, %edx                # imm = 0x3B800001
	leal	-998244353(%rax,%r12), %ecx
	cmovbl	%edx, %ecx
	movq	(%r14), %rsi
	jmp	.LBB10_19
.LBB10_17:
	movdqu	(%rdi), %xmm1
	movdqu	16(%rdi), %xmm0
	movdqu	32(%rdi), %xmm2
	movdqu	48(%rdi), %xmm3
	pxor	%xmm4, %xmm4
	movdqa	%xmm2, %xmm5
	punpckhdq	%xmm4, %xmm5            # xmm5 = xmm5[2],xmm4[2],xmm5[3],xmm4[3]
	movdqa	%xmm1, %xmm6
	punpckhdq	%xmm4, %xmm6            # xmm6 = xmm6[2],xmm4[2],xmm6[3],xmm4[3]
	paddq	%xmm5, %xmm6
	movdqa	%xmm3, %xmm5
	punpckhdq	%xmm4, %xmm5            # xmm5 = xmm5[2],xmm4[2],xmm5[3],xmm4[3]
	movdqa	%xmm0, %xmm7
	punpckhdq	%xmm4, %xmm7            # xmm7 = xmm7[2],xmm4[2],xmm7[3],xmm4[3]
	paddq	%xmm5, %xmm7
	paddq	%xmm6, %xmm7
	punpckldq	%xmm4, %xmm2            # xmm2 = xmm2[0],xmm4[0],xmm2[1],xmm4[1]
	punpckldq	%xmm4, %xmm1            # xmm1 = xmm1[0],xmm4[0],xmm1[1],xmm4[1]
	paddq	%xmm2, %xmm1
	punpckldq	%xmm4, %xmm3            # xmm3 = xmm3[0],xmm4[0],xmm3[1],xmm4[1]
	punpckldq	%xmm4, %xmm0            # xmm0 = xmm0[0],xmm4[0],xmm0[1],xmm4[1]
	paddq	%xmm3, %xmm0
	paddq	%xmm1, %xmm0
	paddq	%xmm7, %xmm0
	pshufd	$238, %xmm0, %xmm1              # xmm1 = xmm0[2,3,2,3]
	paddq	%xmm0, %xmm1
	movq	%xmm1, %rcx
	movabsq	$-8525806094425994177, %rdx     # imm = 0x89AE40875DE0CC3F
	movq	%rcx, %rax
	mulq	%rdx
	shrq	$29, %rdx
	imull	$998244353, %edx, %eax          # imm = 0x3B800001
	subl	%eax, %ecx
.LBB10_19:
	movq	56(%rsp), %rbp                  # 8-byte Reload
	movq	48(%rsp), %r8                   # 8-byte Reload
	movq	32(%rsp), %rax                  # 8-byte Reload
	movq	88(%r14,%rax,8), %rax
	leaq	(%rsi,%rax,4), %rax
	movq	72(%rsp), %rdx                  # 8-byte Reload
	leaq	(%rax,%rdx,4), %rax
	movl	%ecx, (%rax,%r15,4)
	movq	80(%rsp), %rax                  # 8-byte Reload
	movq	%r13, %r10
.LBB10_20:
	cmpq	$0, 40(%rsp)                    # 8-byte Folded Reload
	je	.LBB10_30
# %bb.21:
	testq	%r10, %r10
	je	.LBB10_23
# %bb.22:
	cmpl	%r8d, %eax
	je	.LBB10_30
.LBB10_23:
	shlq	$4, %rbp
	movl	%r8d, %r12d
	leaq	(%r12,%rbp), %rbx
	movq	32(%rsp), %rax                  # 8-byte Reload
	movq	88(%r14,%rax,8), %r13
	addq	%rbx, %r13
	subq	96(%r14), %r13
	movq	24(%r14), %rax
	movq	48(%r14), %rcx
	movl	(%rax,%r13,4), %r9d
	movl	(%rcx,%r13,4), %eax
	movq	16(%rsp), %rcx                  # 8-byte Reload
	decl	%ecx
	movq	%rcx, 16(%rsp)                  # 8-byte Spill
	cmpl	$301989884, %r9d                # imm = 0x11FFFFFC
	jne	.LBB10_25
# %bb.24:
	testl	%eax, %eax
	je	.LBB10_26
.LBB10_25:
	movl	%eax, (%rsp)
	movq	%r14, %rdi
	movq	16(%rsp), %rsi                  # 8-byte Reload
                                        # kill: def $esi killed $esi killed $rsi
	movq	%rbx, %rdx
	xorl	%ecx, %ecx
	movl	$16, %r8d
	callq	_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj
	movq	24(%r14), %rax
	movl	$301989884, (%rax,%r13,4)       # imm = 0x11FFFFFC
	movq	48(%r14), %rax
	movl	$0, (%rax,%r13,4)
.LBB10_26:
	movl	144(%rsp), %eax
	movl	%eax, (%rsp)
	movq	%r14, %rdi
	movq	16(%rsp), %r15                  # 8-byte Reload
	movl	%r15d, %esi
	movq	%rbx, %rdx
	xorl	%ecx, %ecx
	movq	40(%rsp), %r8                   # 8-byte Reload
	movl	28(%rsp), %r9d                  # 4-byte Reload
	callq	_ZN9blueberry10extra_fast22AffineSumSegmentTree166updateEimmmjj
	movslq	%r15d, %rax
	shlq	$4, %rbx
	addq	88(%r14,%rax,8), %rbx
	cmpb	$0, 76(%r14)
	movq	(%r14), %rsi
	leaq	(%rsi,%rbx,4), %rdi
	je	.LBB10_27
# %bb.28:
	xorl	%esi, %esi
	movl	$8, %edx
	callq	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
	movl	%eax, %r15d
	movq	(%r14), %rax
	leaq	(%rax,%rbx,4), %rdi
	addq	$32, %rdi
	xorl	%esi, %esi
	movl	$8, %edx
	callq	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
                                        # kill: def $eax killed $eax def $rax
	leal	(%rax,%r15), %edx
	cmpl	$998244353, %edx                # imm = 0x3B800001
	leal	-998244353(%rax,%r15), %ecx
	cmovbl	%edx, %ecx
	movq	(%r14), %rsi
	jmp	.LBB10_29
.LBB10_27:
	movdqu	(%rdi), %xmm1
	movdqu	16(%rdi), %xmm0
	movdqu	32(%rdi), %xmm2
	movdqu	48(%rdi), %xmm3
	pxor	%xmm4, %xmm4
	movdqa	%xmm2, %xmm5
	punpckhdq	%xmm4, %xmm5            # xmm5 = xmm5[2],xmm4[2],xmm5[3],xmm4[3]
	movdqa	%xmm1, %xmm6
	punpckhdq	%xmm4, %xmm6            # xmm6 = xmm6[2],xmm4[2],xmm6[3],xmm4[3]
	paddq	%xmm5, %xmm6
	movdqa	%xmm3, %xmm5
	punpckhdq	%xmm4, %xmm5            # xmm5 = xmm5[2],xmm4[2],xmm5[3],xmm4[3]
	movdqa	%xmm0, %xmm7
	punpckhdq	%xmm4, %xmm7            # xmm7 = xmm7[2],xmm4[2],xmm7[3],xmm4[3]
	paddq	%xmm5, %xmm7
	paddq	%xmm6, %xmm7
	punpckldq	%xmm4, %xmm2            # xmm2 = xmm2[0],xmm4[0],xmm2[1],xmm4[1]
	punpckldq	%xmm4, %xmm1            # xmm1 = xmm1[0],xmm4[0],xmm1[1],xmm4[1]
	paddq	%xmm2, %xmm1
	punpckldq	%xmm4, %xmm3            # xmm3 = xmm3[0],xmm4[0],xmm3[1],xmm4[1]
	punpckldq	%xmm4, %xmm0            # xmm0 = xmm0[0],xmm4[0],xmm0[1],xmm4[1]
	paddq	%xmm3, %xmm0
	paddq	%xmm1, %xmm0
	paddq	%xmm7, %xmm0
	pshufd	$238, %xmm0, %xmm1              # xmm1 = xmm0[2,3,2,3]
	paddq	%xmm0, %xmm1
	movq	%xmm1, %rcx
	movabsq	$-8525806094425994177, %rdx     # imm = 0x89AE40875DE0CC3F
	movq	%rcx, %rax
	mulq	%rdx
	shrq	$29, %rdx
	imull	$998244353, %edx, %eax          # imm = 0x3B800001
	subl	%eax, %ecx
.LBB10_29:
	movq	32(%rsp), %rax                  # 8-byte Reload
	movq	88(%r14,%rax,8), %rax
	leaq	(%rsi,%rax,4), %rax
	leaq	(%rax,%rbp,4), %rax
	movl	%ecx, (%rax,%r12,4)
.LBB10_30:
	addq	$88, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end10:
	.size	_ZN9blueberry10extra_fast22AffineSumSegmentTree166updateEimmmjj, .Lfunc_end10-_ZN9blueberry10extra_fast22AffineSumSegmentTree166updateEimmmjj
	.cfi_endproc
                                        # -- End function
	.section	.text._ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj,"axG",@progbits,_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj,comdat
	.weak	_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj # -- Begin function _ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj
	.p2align	4, 0x90
	.type	_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj,@function
_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj: # @_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$24, %rsp
	.cfi_def_cfa_offset 80
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%rdi, %r15
	movslq	%esi, %rax
	shlq	$4, %rdx
	addq	88(%rdi,%rax,8), %rdx
	movl	80(%rsp), %r11d
	movl	280(%rdi,%rax,4), %edi
	imulq	%r11, %rdi
	imull	$998244351, %edi, %r10d         # imm = 0x3B7FFFFF
	imulq	$998244353, %r10, %r10          # imm = 0x3B800001
	addq	%rdi, %r10
	shrq	$32, %r10
	leal	-998244353(%r10), %r13d
	cmpl	$998244353, %r10d               # imm = 0x3B800001
	cmovbl	%r10d, %r13d
	leaq	(,%rdx,4), %rdi
	movq	24(%r15), %rbx
	addq	%rdi, %rbx
	movq	96(%r15), %r10
	shlq	$2, %r10
	subq	%r10, %rbx
	movq	48(%r15), %r14
	addq	%rdi, %r14
	subq	%r10, %r14
	xorl	%r10d, %r10d
	testl	%eax, %eax
	cmoveq	%r10, %rbx
	cmoveq	%r10, %r14
	cmpb	$0, 76(%r15)
	je	.LBB11_1
# %bb.7:
	cmpl	$7, %ecx
	ja	.LBB11_9
# %bb.8:
	addq	(%r15), %rdi
	cmpl	$8, %r8d
	movl	$8, %eax
	cmovbl	%r8d, %eax
	movq	%rbx, %rsi
	movq	%rdx, 16(%rsp)                  # 8-byte Spill
	movq	%r14, %rdx
	movl	%ecx, %ebp
	movl	%r8d, 12(%rsp)                  # 4-byte Spill
	movl	%eax, %r8d
	movl	%r9d, %r12d
	pushq	%r13
	.cfi_adjust_cfa_offset 8
	pushq	%r11
	.cfi_adjust_cfa_offset 8
	callq	_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj
	movl	96(%rsp), %r11d
	movq	32(%rsp), %rdx                  # 8-byte Reload
	movl	%ebp, %ecx
	movl	%r12d, %r9d
	movl	28(%rsp), %r8d                  # 4-byte Reload
	addq	$16, %rsp
	.cfi_adjust_cfa_offset -16
.LBB11_9:
	cmpl	$9, %r8d
	jb	.LBB11_6
# %bb.10:
	movq	(%r15), %rax
	leaq	(%rax,%rdx,4), %rdi
	addq	$32, %rdi
	leaq	32(%rbx), %rsi
	testq	%rbx, %rbx
	cmoveq	%rbx, %rsi
	leaq	32(%r14), %rdx
	testq	%r14, %r14
	cmoveq	%r14, %rdx
	xorl	%eax, %eax
	subl	$8, %ecx
	cmovbl	%eax, %ecx
	addl	$-8, %r8d
	pushq	%r13
	.cfi_adjust_cfa_offset 8
	pushq	%r11
	.cfi_adjust_cfa_offset 8
	callq	_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj
	addq	$40, %rsp
	.cfi_adjust_cfa_offset -40
	jmp	.LBB11_11
.LBB11_1:
	.cfi_def_cfa_offset 80
	cmpl	%r8d, %ecx
	jae	.LBB11_6
# %bb.2:
	addq	(%r15), %rdi
	movl	%r9d, %eax
	movl	%ecx, %ecx
	movl	%r8d, %edx
	jmp	.LBB11_3
	.p2align	4, 0x90
.LBB11_5:                               #   in Loop: Header=BB11_3 Depth=1
	incq	%rcx
	cmpq	%rcx, %rdx
	je	.LBB11_6
.LBB11_3:                               # =>This Inner Loop Header: Depth=1
	movl	(%rdi,%rcx,4), %r8d
	imulq	%rax, %r8
	imull	$998244351, %r8d, %r9d          # imm = 0x3B7FFFFF
	imulq	$998244353, %r9, %r9            # imm = 0x3B800001
	addq	%r8, %r9
	shrq	$32, %r9
	leal	-998244353(%r9), %r8d
	cmpl	$998244353, %r9d                # imm = 0x3B800001
	cmovbl	%r9d, %r8d
	leal	(%r8,%r13), %r9d
	cmpl	$998244353, %r9d                # imm = 0x3B800001
	leal	-998244353(%r8,%r13), %r8d
	cmovbl	%r9d, %r8d
	movl	%r8d, (%rdi,%rcx,4)
	testl	%esi, %esi
	je	.LBB11_5
# %bb.4:                                #   in Loop: Header=BB11_3 Depth=1
	movl	(%rbx,%rcx,4), %r8d
	imulq	%rax, %r8
	imull	$998244351, %r8d, %r9d          # imm = 0x3B7FFFFF
	imulq	$998244353, %r9, %r9            # imm = 0x3B800001
	addq	%r8, %r9
	shrq	$32, %r9
	leal	-998244353(%r9), %r8d
	cmpl	$998244353, %r9d                # imm = 0x3B800001
	cmovbl	%r9d, %r8d
	movl	%r8d, (%rbx,%rcx,4)
	movl	(%r14,%rcx,4), %r8d
	imulq	%rax, %r8
	imull	$998244351, %r8d, %r9d          # imm = 0x3B7FFFFF
	imulq	$998244353, %r9, %r9            # imm = 0x3B800001
	addq	%r8, %r9
	shrq	$32, %r9
	leal	-998244353(%r9), %r8d
	cmpl	$998244353, %r9d                # imm = 0x3B800001
	cmovbl	%r9d, %r8d
	leal	(%r8,%r11), %r9d
	cmpl	$998244353, %r9d                # imm = 0x3B800001
	leal	-998244353(%r8,%r11), %r8d
	cmovbl	%r9d, %r8d
	movl	%r8d, (%r14,%rcx,4)
	jmp	.LBB11_5
.LBB11_6:
	addq	$24, %rsp
	.cfi_def_cfa_offset 56
.LBB11_11:
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end11:
	.size	_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj, .Lfunc_end11-_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst32,"aM",@progbits,32
	.p2align	5, 0x0                          # -- Begin function _ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj
.LCPI12_0:
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	2                               # 0x2
	.long	3                               # 0x3
	.long	4                               # 0x4
	.long	5                               # 0x5
	.long	6                               # 0x6
	.long	7                               # 0x7
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0
.LCPI12_1:
	.quad	998244353                       # 0x3b800001
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0
.LCPI12_2:
	.long	3296722943                      # 0xc47fffff
	.section	.text._ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj,"axG",@progbits,_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj,comdat
	.weak	_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj
	.p2align	4, 0x90
	.type	_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj,@function
_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj: # @_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj
	.cfi_startproc
# %bb.0:
	vmovd	%ecx, %xmm0
	vpbroadcastd	%xmm0, %ymm0
	vmovdqa	.LCPI12_0(%rip), %ymm1          # ymm1 = [0,1,2,3,4,5,6,7]
	vpcmpgtd	%ymm1, %ymm0, %ymm0
	vmovd	%r8d, %xmm2
	vpbroadcastd	%xmm2, %ymm2
	vpcmpgtd	%ymm1, %ymm2, %ymm1
	vpandn	%ymm1, %ymm0, %ymm4
	vmovd	%r9d, %xmm0
	vpbroadcastd	%xmm0, %ymm0
	imull	$998244351, %r9d, %eax          # imm = 0x3B7FFFFF
	vmovd	%eax, %xmm1
	vpbroadcastd	%xmm1, %ymm2
	vmovdqa	(%rdi), %ymm5
	vpbroadcastd	16(%rsp), %ymm6
	vpmulld	%ymm5, %ymm2, %ymm3
	vpmuludq	%ymm0, %ymm5, %ymm7
	vpbroadcastq	.LCPI12_1(%rip), %ymm1  # ymm1 = [998244353,998244353,998244353,998244353]
	vpmuludq	%ymm1, %ymm3, %ymm8
	vpaddq	%ymm7, %ymm8, %ymm7
	vpsrlq	$32, %ymm7, %ymm7
	vpsrlq	$32, %ymm5, %ymm8
	vpmuludq	%ymm0, %ymm8, %ymm8
	vpsrlq	$32, %ymm3, %ymm3
	vpmuludq	%ymm1, %ymm3, %ymm3
	vpaddq	%ymm3, %ymm8, %ymm3
	vpblendd	$170, %ymm3, %ymm7, %ymm7       # ymm7 = ymm7[0],ymm3[1],ymm7[2],ymm3[3],ymm7[4],ymm3[5],ymm7[6],ymm3[7]
	vpbroadcastd	.LCPI12_2(%rip), %ymm3  # ymm3 = [3296722943,3296722943,3296722943,3296722943,3296722943,3296722943,3296722943,3296722943]
	vpaddd	%ymm3, %ymm7, %ymm8
	vpminud	%ymm8, %ymm7, %ymm7
	vpaddd	%ymm6, %ymm7, %ymm6
	vpaddd	%ymm3, %ymm6, %ymm7
	vpminud	%ymm7, %ymm6, %ymm6
	vblendvps	%ymm4, %ymm6, %ymm5, %ymm5
	vmovaps	%ymm5, (%rdi)
	testq	%rsi, %rsi
	je	.LBB12_2
# %bb.1:
	movl	8(%rsp), %eax
	vextracti128	$1, %ymm4, %xmm5
	vpackssdw	%xmm5, %xmm4, %xmm5
	vmovdqa	(%rsi), %ymm6
	vmovdqa	(%rdx), %ymm4
	vpmulld	%ymm6, %ymm2, %ymm7
	vpmuludq	%ymm0, %ymm6, %ymm8
	vpmuludq	%ymm1, %ymm7, %ymm9
	vpaddq	%ymm8, %ymm9, %ymm8
	vpsrlq	$32, %ymm8, %ymm8
	vpsrlq	$32, %ymm6, %ymm9
	vpmuludq	%ymm0, %ymm9, %ymm9
	vpsrlq	$32, %ymm7, %ymm7
	vpmuludq	%ymm1, %ymm7, %ymm7
	vpaddq	%ymm7, %ymm9, %ymm7
	vpblendd	$170, %ymm7, %ymm8, %ymm7       # ymm7 = ymm8[0],ymm7[1],ymm8[2],ymm7[3],ymm8[4],ymm7[5],ymm8[6],ymm7[7]
	vpaddd	%ymm3, %ymm7, %ymm8
	vpminud	%ymm8, %ymm7, %ymm7
	vpaddd	%ymm3, %ymm7, %ymm8
	vpminud	%ymm8, %ymm7, %ymm7
	vpmovsxwd	%xmm5, %ymm5
	vblendvps	%ymm5, %ymm7, %ymm6, %ymm6
	vmovaps	%ymm6, (%rsi)
	vmovd	%eax, %xmm6
	vpbroadcastd	%xmm6, %ymm6
	vpmulld	%ymm4, %ymm2, %ymm2
	vpmuludq	%ymm0, %ymm4, %ymm7
	vpmuludq	%ymm1, %ymm2, %ymm8
	vpaddq	%ymm7, %ymm8, %ymm7
	vpsrlq	$32, %ymm7, %ymm7
	vpsrlq	$32, %ymm4, %ymm8
	vpmuludq	%ymm0, %ymm8, %ymm0
	vpsrlq	$32, %ymm2, %ymm2
	vpmuludq	%ymm1, %ymm2, %ymm1
	vpaddq	%ymm0, %ymm1, %ymm0
	vpblendd	$170, %ymm0, %ymm7, %ymm0       # ymm0 = ymm7[0],ymm0[1],ymm7[2],ymm0[3],ymm7[4],ymm0[5],ymm7[6],ymm0[7]
	vpaddd	%ymm3, %ymm0, %ymm1
	vpminud	%ymm1, %ymm0, %ymm0
	vpaddd	%ymm6, %ymm0, %ymm0
	vpaddd	%ymm3, %ymm0, %ymm1
	vpminud	%ymm1, %ymm0, %ymm0
	vblendvps	%ymm5, %ymm0, %ymm4, %ymm0
	vmovaps	%ymm0, (%rdx)
.LBB12_2:
	vzeroupper
	retq
.Lfunc_end12:
	.size	_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj, .Lfunc_end12-_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj
	.cfi_endproc
                                        # -- End function
	.section	.text._ZN9blueberry10extra_fast18ShiftAffineControl6updateEimmmjj,"axG",@progbits,_ZN9blueberry10extra_fast18ShiftAffineControl6updateEimmmjj,comdat
	.weak	_ZN9blueberry10extra_fast18ShiftAffineControl6updateEimmmjj # -- Begin function _ZN9blueberry10extra_fast18ShiftAffineControl6updateEimmmjj
	.p2align	4, 0x90
	.type	_ZN9blueberry10extra_fast18ShiftAffineControl6updateEimmmjj,@function
_ZN9blueberry10extra_fast18ShiftAffineControl6updateEimmmjj: # @_ZN9blueberry10extra_fast18ShiftAffineControl6updateEimmmjj
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$88, %rsp
	.cfi_def_cfa_offset 144
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%r8, %rbx
	movq	%rcx, %r13
	movq	%rdx, %r10
                                        # kill: def $esi killed $esi def $rsi
	leal	(,%rsi,4), %ebp
	movl	$1, %r12d
	movl	%ebp, %ecx
	shlq	%cl, %r12
	leaq	(%r12,%r13), %rax
	decq	%rax
	shrq	%cl, %rax
	movq	%rdi, %r14
	movq	%r8, %r15
	shrq	%cl, %r15
	cmpl	%r15d, %eax
	jae	.LBB13_2
# %bb.1:
	movl	144(%rsp), %ecx
	movl	%ecx, (%rsp)
	movq	%r14, %rdi
	movq	%r14, 8(%rsp)                   # 8-byte Spill
	movq	%rbx, 32(%rsp)                  # 8-byte Spill
	movq	%r13, 16(%rsp)                  # 8-byte Spill
	movq	%rsi, %r13
	movq	%r10, %rdx
	movl	%eax, %ecx
	movl	%r15d, %r8d
	movl	%r9d, %r14d
	movq	%r10, %rbx
	callq	_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj
	movq	%rbx, %r10
	movl	%r14d, %r9d
	movq	%r13, %rsi
	movq	16(%rsp), %r13                  # 8-byte Reload
	movq	32(%rsp), %rbx                  # 8-byte Reload
	movq	8(%rsp), %r14                   # 8-byte Reload
.LBB13_2:
	testl	%esi, %esi
	je	.LBB13_21
# %bb.3:
	leaq	-1(%r12), %r8
	movq	%r13, %rdx
	movl	%ebp, %ecx
	shrq	%cl, %rdx
	andq	%r8, %r13
	movl	%r9d, 28(%rsp)                  # 4-byte Spill
	je	.LBB13_11
# %bb.4:
	movq	%r15, 40(%rsp)                  # 8-byte Spill
	movq	%r13, 16(%rsp)                  # 8-byte Spill
	movq	%r8, 48(%rsp)                   # 8-byte Spill
	leal	1(%rdx), %eax
	movl	%ebp, %ecx
	shlq	%cl, %rax
	movq	%rdx, 80(%rsp)                  # 8-byte Spill
	movl	%edx, %edi
	movq	%rdi, %rdx
	shlq	%cl, %rdx
	movq	%rbx, %rcx
	subq	%rdx, %rcx
	cmpq	%rbx, %rax
	cmovaq	%rcx, %r12
	movslq	%esi, %rdx
	movq	%r10, 8(%rsp)                   # 8-byte Spill
	shlq	$4, %r10
	movq	%rdi, %rax
	movq	%rdi, 72(%rsp)                  # 8-byte Spill
	movq	%r10, 64(%rsp)                  # 8-byte Spill
	leaq	(%rdi,%r10), %rbp
	movq	%rdx, 56(%rsp)                  # 8-byte Spill
	movq	88(%r14,%rdx,8), %r13
	addq	%rbp, %r13
	subq	96(%r14), %r13
	movq	24(%r14), %rax
	movq	48(%r14), %rcx
	movl	(%rax,%r13,4), %r9d
	movl	(%rcx,%r13,4), %eax
	movq	%rsi, 32(%rsp)                  # 8-byte Spill
	leal	-1(%rsi), %r15d
	cmpl	$301989884, %r9d                # imm = 0x11FFFFFC
	jne	.LBB13_6
# %bb.5:
	testl	%eax, %eax
	je	.LBB13_7
.LBB13_6:
	movl	%eax, (%rsp)
	movq	%r14, %rdi
	movl	%r15d, %esi
	movq	%rbp, %rdx
	xorl	%ecx, %ecx
	movl	$16, %r8d
	callq	_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj
	movq	24(%r14), %rax
	movl	$301989884, (%rax,%r13,4)       # imm = 0x11FFFFFC
	movq	48(%r14), %rax
	movl	$0, (%rax,%r13,4)
.LBB13_7:
	movl	144(%rsp), %eax
	movl	%eax, (%rsp)
	movq	%r14, %rdi
	movl	%r15d, %esi
	movq	%rbp, %rdx
	movq	16(%rsp), %r13                  # 8-byte Reload
	movq	%r13, %rcx
	movq	%r12, %r8
	movl	28(%rsp), %r9d                  # 4-byte Reload
	callq	_ZN9blueberry10extra_fast18ShiftAffineControl6updateEimmmjj
	movslq	%r15d, %rax
	shlq	$4, %rbp
	addq	88(%r14,%rax,8), %rbp
	cmpb	$0, 76(%r14)
	movq	(%r14), %rsi
	leaq	(%rsi,%rbp,4), %rdi
	je	.LBB13_8
# %bb.9:
	xorl	%esi, %esi
	movl	$8, %edx
	callq	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
	movl	%eax, %r15d
	movq	(%r14), %rax
	leaq	(%rax,%rbp,4), %rdi
	addq	$32, %rdi
	xorl	%esi, %esi
	movl	$8, %edx
	callq	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
                                        # kill: def $eax killed $eax def $rax
	leal	(%rax,%r15), %edx
	cmpl	$998244353, %edx                # imm = 0x3B800001
	leal	-998244353(%rax,%r15), %ecx
	cmovbl	%edx, %ecx
	movq	(%r14), %rsi
	jmp	.LBB13_10
.LBB13_8:
	movdqu	(%rdi), %xmm1
	movdqu	16(%rdi), %xmm0
	movdqu	32(%rdi), %xmm2
	movdqu	48(%rdi), %xmm3
	pxor	%xmm4, %xmm4
	movdqa	%xmm2, %xmm5
	punpckhdq	%xmm4, %xmm5            # xmm5 = xmm5[2],xmm4[2],xmm5[3],xmm4[3]
	movdqa	%xmm1, %xmm6
	punpckhdq	%xmm4, %xmm6            # xmm6 = xmm6[2],xmm4[2],xmm6[3],xmm4[3]
	paddq	%xmm5, %xmm6
	movdqa	%xmm3, %xmm5
	punpckhdq	%xmm4, %xmm5            # xmm5 = xmm5[2],xmm4[2],xmm5[3],xmm4[3]
	movdqa	%xmm0, %xmm7
	punpckhdq	%xmm4, %xmm7            # xmm7 = xmm7[2],xmm4[2],xmm7[3],xmm4[3]
	paddq	%xmm5, %xmm7
	paddq	%xmm6, %xmm7
	punpckldq	%xmm4, %xmm2            # xmm2 = xmm2[0],xmm4[0],xmm2[1],xmm4[1]
	punpckldq	%xmm4, %xmm1            # xmm1 = xmm1[0],xmm4[0],xmm1[1],xmm4[1]
	paddq	%xmm2, %xmm1
	punpckldq	%xmm4, %xmm3            # xmm3 = xmm3[0],xmm4[0],xmm3[1],xmm4[1]
	punpckldq	%xmm4, %xmm0            # xmm0 = xmm0[0],xmm4[0],xmm0[1],xmm4[1]
	paddq	%xmm3, %xmm0
	paddq	%xmm1, %xmm0
	paddq	%xmm7, %xmm0
	pshufd	$238, %xmm0, %xmm1              # xmm1 = xmm0[2,3,2,3]
	paddq	%xmm0, %xmm1
	movq	%xmm1, %rcx
	movabsq	$-8525806094425994177, %rdx     # imm = 0x89AE40875DE0CC3F
	movq	%rcx, %rax
	mulq	%rdx
	shrq	$29, %rdx
	imull	$998244353, %edx, %eax          # imm = 0x3B800001
	subl	%eax, %ecx
.LBB13_10:
	movq	8(%rsp), %r10                   # 8-byte Reload
	movq	48(%rsp), %r8                   # 8-byte Reload
	movq	40(%rsp), %r15                  # 8-byte Reload
	movq	56(%rsp), %rax                  # 8-byte Reload
	movq	88(%r14,%rax,8), %rax
	leaq	(%rsi,%rax,4), %rax
	movq	64(%rsp), %rdx                  # 8-byte Reload
	leaq	(%rax,%rdx,4), %rax
	movq	72(%rsp), %rdx                  # 8-byte Reload
	movl	%ecx, (%rax,%rdx,4)
	movq	32(%rsp), %rsi                  # 8-byte Reload
	movq	80(%rsp), %rdx                  # 8-byte Reload
.LBB13_11:
	andq	%rbx, %r8
	je	.LBB13_21
# %bb.12:
	testq	%r13, %r13
	je	.LBB13_14
# %bb.13:
	cmpl	%r15d, %edx
	je	.LBB13_21
.LBB13_14:
	movslq	%esi, %r12
	shlq	$4, %r10
	movl	%r15d, %r13d
	movq	%r10, 8(%rsp)                   # 8-byte Spill
	leaq	(%r10,%r13), %rbx
	movq	88(%r14,%r12,8), %r15
	addq	%rbx, %r15
	subq	96(%r14), %r15
	movq	24(%r14), %rax
	movq	48(%r14), %rcx
	movl	(%rax,%r15,4), %r9d
	movl	(%rcx,%r15,4), %eax
	decl	%esi
	cmpl	$301989884, %r9d                # imm = 0x11FFFFFC
	jne	.LBB13_16
# %bb.15:
	testl	%eax, %eax
	je	.LBB13_17
.LBB13_16:
	movl	%eax, (%rsp)
	movq	%r14, %rdi
	movq	%r13, 16(%rsp)                  # 8-byte Spill
	movq	%r12, %r13
	movq	%rsi, %r12
	movq	%rbx, %rdx
	xorl	%ecx, %ecx
	movq	%r8, %rbp
	movl	$16, %r8d
	callq	_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj
	movq	%rbp, %r8
	movq	%r12, %rsi
	movq	%r13, %r12
	movq	16(%rsp), %r13                  # 8-byte Reload
	movq	24(%r14), %rax
	movl	$301989884, (%rax,%r15,4)       # imm = 0x11FFFFFC
	movq	48(%r14), %rax
	movl	$0, (%rax,%r15,4)
.LBB13_17:
	movl	144(%rsp), %eax
	movl	%eax, (%rsp)
	movq	%r14, %rdi
	movq	%rsi, %r15
	movq	%rbx, %rdx
	xorl	%ecx, %ecx
	movl	28(%rsp), %r9d                  # 4-byte Reload
	callq	_ZN9blueberry10extra_fast18ShiftAffineControl6updateEimmmjj
	movslq	%r15d, %rax
	shlq	$4, %rbx
	addq	88(%r14,%rax,8), %rbx
	cmpb	$0, 76(%r14)
	movq	(%r14), %rsi
	leaq	(%rsi,%rbx,4), %rdi
	je	.LBB13_18
# %bb.19:
	xorl	%esi, %esi
	movl	$8, %edx
	callq	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
	movl	%eax, %r15d
	movq	(%r14), %rax
	leaq	(%rax,%rbx,4), %rdi
	addq	$32, %rdi
	xorl	%esi, %esi
	movl	$8, %edx
	callq	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
                                        # kill: def $eax killed $eax def $rax
	leal	(%rax,%r15), %edx
	cmpl	$998244353, %edx                # imm = 0x3B800001
	leal	-998244353(%rax,%r15), %ecx
	cmovbl	%edx, %ecx
	movq	(%r14), %rsi
	jmp	.LBB13_20
.LBB13_18:
	movdqu	(%rdi), %xmm1
	movdqu	16(%rdi), %xmm0
	movdqu	32(%rdi), %xmm2
	movdqu	48(%rdi), %xmm3
	pxor	%xmm4, %xmm4
	movdqa	%xmm2, %xmm5
	punpckhdq	%xmm4, %xmm5            # xmm5 = xmm5[2],xmm4[2],xmm5[3],xmm4[3]
	movdqa	%xmm1, %xmm6
	punpckhdq	%xmm4, %xmm6            # xmm6 = xmm6[2],xmm4[2],xmm6[3],xmm4[3]
	paddq	%xmm5, %xmm6
	movdqa	%xmm3, %xmm5
	punpckhdq	%xmm4, %xmm5            # xmm5 = xmm5[2],xmm4[2],xmm5[3],xmm4[3]
	movdqa	%xmm0, %xmm7
	punpckhdq	%xmm4, %xmm7            # xmm7 = xmm7[2],xmm4[2],xmm7[3],xmm4[3]
	paddq	%xmm5, %xmm7
	paddq	%xmm6, %xmm7
	punpckldq	%xmm4, %xmm2            # xmm2 = xmm2[0],xmm4[0],xmm2[1],xmm4[1]
	punpckldq	%xmm4, %xmm1            # xmm1 = xmm1[0],xmm4[0],xmm1[1],xmm4[1]
	paddq	%xmm2, %xmm1
	punpckldq	%xmm4, %xmm3            # xmm3 = xmm3[0],xmm4[0],xmm3[1],xmm4[1]
	punpckldq	%xmm4, %xmm0            # xmm0 = xmm0[0],xmm4[0],xmm0[1],xmm4[1]
	paddq	%xmm3, %xmm0
	paddq	%xmm1, %xmm0
	paddq	%xmm7, %xmm0
	pshufd	$238, %xmm0, %xmm1              # xmm1 = xmm0[2,3,2,3]
	paddq	%xmm0, %xmm1
	movq	%xmm1, %rcx
	movabsq	$-8525806094425994177, %rdx     # imm = 0x89AE40875DE0CC3F
	movq	%rcx, %rax
	mulq	%rdx
	shrq	$29, %rdx
	imull	$998244353, %edx, %eax          # imm = 0x3B800001
	subl	%eax, %ecx
.LBB13_20:
	movq	88(%r14,%r12,8), %rax
	leaq	(%rsi,%rax,4), %rax
	movq	8(%rsp), %rdx                   # 8-byte Reload
	leaq	(%rax,%rdx,4), %rax
	movl	%ecx, (%rax,%r13,4)
.LBB13_21:
	addq	$88, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end13:
	.size	_ZN9blueberry10extra_fast18ShiftAffineControl6updateEimmmjj, .Lfunc_end13-_ZN9blueberry10extra_fast18ShiftAffineControl6updateEimmmjj
	.cfi_endproc
                                        # -- End function
	.section	.text._ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj,"axG",@progbits,_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj,comdat
	.weak	_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj # -- Begin function _ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj
	.p2align	4, 0x90
	.type	_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj,@function
_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj: # @_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$24, %rsp
	.cfi_def_cfa_offset 80
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%rdi, %r15
	movslq	%esi, %rax
	shlq	$4, %rdx
	addq	88(%rdi,%rax,8), %rdx
	movl	80(%rsp), %r11d
	movl	184(%rdi,%rax,4), %edi
	imulq	%r11, %rdi
	imull	$998244351, %edi, %r10d         # imm = 0x3B7FFFFF
	imulq	$998244353, %r10, %r10          # imm = 0x3B800001
	addq	%rdi, %r10
	shrq	$32, %r10
	leal	-998244353(%r10), %r13d
	cmpl	$998244353, %r10d               # imm = 0x3B800001
	cmovbl	%r10d, %r13d
	leaq	(,%rdx,4), %rdi
	movq	24(%r15), %rbx
	addq	%rdi, %rbx
	movq	96(%r15), %r10
	shlq	$2, %r10
	subq	%r10, %rbx
	movq	48(%r15), %r14
	addq	%rdi, %r14
	subq	%r10, %r14
	xorl	%r10d, %r10d
	testl	%eax, %eax
	cmoveq	%r10, %rbx
	cmoveq	%r10, %r14
	cmpb	$0, 76(%r15)
	je	.LBB14_1
# %bb.7:
	cmpl	$7, %ecx
	ja	.LBB14_9
# %bb.8:
	addq	(%r15), %rdi
	cmpl	$8, %r8d
	movl	$8, %eax
	cmovbl	%r8d, %eax
	movq	%rbx, %rsi
	movq	%rdx, 16(%rsp)                  # 8-byte Spill
	movq	%r14, %rdx
	movl	%ecx, %ebp
	movl	%r8d, 12(%rsp)                  # 4-byte Spill
	movl	%eax, %r8d
	movl	%r9d, %r12d
	pushq	%r13
	.cfi_adjust_cfa_offset 8
	pushq	%r11
	.cfi_adjust_cfa_offset 8
	callq	_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj
	movl	96(%rsp), %r11d
	movq	32(%rsp), %rdx                  # 8-byte Reload
	movl	%ebp, %ecx
	movl	%r12d, %r9d
	movl	28(%rsp), %r8d                  # 4-byte Reload
	addq	$16, %rsp
	.cfi_adjust_cfa_offset -16
.LBB14_9:
	cmpl	$9, %r8d
	jb	.LBB14_6
# %bb.10:
	movq	(%r15), %rax
	leaq	(%rax,%rdx,4), %rdi
	addq	$32, %rdi
	leaq	32(%rbx), %rsi
	testq	%rbx, %rbx
	cmoveq	%rbx, %rsi
	leaq	32(%r14), %rdx
	testq	%r14, %r14
	cmoveq	%r14, %rdx
	xorl	%eax, %eax
	subl	$8, %ecx
	cmovbl	%eax, %ecx
	addl	$-8, %r8d
	pushq	%r13
	.cfi_adjust_cfa_offset 8
	pushq	%r11
	.cfi_adjust_cfa_offset 8
	callq	_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj
	addq	$40, %rsp
	.cfi_adjust_cfa_offset -40
	jmp	.LBB14_11
.LBB14_1:
	.cfi_def_cfa_offset 80
	cmpl	%r8d, %ecx
	jae	.LBB14_6
# %bb.2:
	addq	(%r15), %rdi
	movl	%r9d, %eax
	movl	%ecx, %ecx
	movl	%r8d, %edx
	jmp	.LBB14_3
	.p2align	4, 0x90
.LBB14_5:                               #   in Loop: Header=BB14_3 Depth=1
	incq	%rcx
	cmpq	%rcx, %rdx
	je	.LBB14_6
.LBB14_3:                               # =>This Inner Loop Header: Depth=1
	movl	(%rdi,%rcx,4), %r8d
	imulq	%rax, %r8
	imull	$998244351, %r8d, %r9d          # imm = 0x3B7FFFFF
	imulq	$998244353, %r9, %r9            # imm = 0x3B800001
	addq	%r8, %r9
	shrq	$32, %r9
	leal	-998244353(%r9), %r8d
	cmpl	$998244353, %r9d                # imm = 0x3B800001
	cmovbl	%r9d, %r8d
	leal	(%r8,%r13), %r9d
	cmpl	$998244353, %r9d                # imm = 0x3B800001
	leal	-998244353(%r8,%r13), %r8d
	cmovbl	%r9d, %r8d
	movl	%r8d, (%rdi,%rcx,4)
	testl	%esi, %esi
	je	.LBB14_5
# %bb.4:                                #   in Loop: Header=BB14_3 Depth=1
	movl	(%rbx,%rcx,4), %r8d
	imulq	%rax, %r8
	imull	$998244351, %r8d, %r9d          # imm = 0x3B7FFFFF
	imulq	$998244353, %r9, %r9            # imm = 0x3B800001
	addq	%r8, %r9
	shrq	$32, %r9
	leal	-998244353(%r9), %r8d
	cmpl	$998244353, %r9d                # imm = 0x3B800001
	cmovbl	%r9d, %r8d
	movl	%r8d, (%rbx,%rcx,4)
	movl	(%r14,%rcx,4), %r8d
	imulq	%rax, %r8
	imull	$998244351, %r8d, %r9d          # imm = 0x3B7FFFFF
	imulq	$998244353, %r9, %r9            # imm = 0x3B800001
	addq	%r8, %r9
	shrq	$32, %r9
	leal	-998244353(%r9), %r8d
	cmpl	$998244353, %r9d                # imm = 0x3B800001
	cmovbl	%r9d, %r8d
	leal	(%r8,%r11), %r9d
	cmpl	$998244353, %r9d                # imm = 0x3B800001
	leal	-998244353(%r8,%r11), %r8d
	cmovbl	%r9d, %r8d
	movl	%r8d, (%r14,%rcx,4)
	jmp	.LBB14_5
.LBB14_6:
	addq	$24, %rsp
	.cfi_def_cfa_offset 56
.LBB14_11:
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end14:
	.size	_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj, .Lfunc_end14-_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst32,"aM",@progbits,32
	.p2align	5, 0x0                          # -- Begin function _ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj
.LCPI15_0:
	.long	0                               # 0x0
	.long	1                               # 0x1
	.long	2                               # 0x2
	.long	3                               # 0x3
	.long	4                               # 0x4
	.long	5                               # 0x5
	.long	6                               # 0x6
	.long	7                               # 0x7
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0
.LCPI15_1:
	.quad	998244353                       # 0x3b800001
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0
.LCPI15_2:
	.long	3296722943                      # 0xc47fffff
	.section	.text._ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj,"axG",@progbits,_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj,comdat
	.weak	_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj
	.p2align	4, 0x90
	.type	_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj,@function
_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj: # @_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj
	.cfi_startproc
# %bb.0:
	vmovd	%ecx, %xmm0
	vpbroadcastd	%xmm0, %ymm0
	vmovdqa	.LCPI15_0(%rip), %ymm1          # ymm1 = [0,1,2,3,4,5,6,7]
	vpcmpgtd	%ymm1, %ymm0, %ymm0
	vmovd	%r8d, %xmm2
	vpbroadcastd	%xmm2, %ymm2
	vpcmpgtd	%ymm1, %ymm2, %ymm1
	vpandn	%ymm1, %ymm0, %ymm4
	vmovd	%r9d, %xmm0
	vpbroadcastd	%xmm0, %ymm0
	imull	$998244351, %r9d, %eax          # imm = 0x3B7FFFFF
	vmovd	%eax, %xmm1
	vpbroadcastd	%xmm1, %ymm2
	vmovdqa	(%rdi), %ymm5
	vpbroadcastd	16(%rsp), %ymm6
	vpmulld	%ymm5, %ymm2, %ymm3
	vpmuludq	%ymm0, %ymm5, %ymm7
	vpbroadcastq	.LCPI15_1(%rip), %ymm1  # ymm1 = [998244353,998244353,998244353,998244353]
	vpmuludq	%ymm1, %ymm3, %ymm8
	vpaddq	%ymm7, %ymm8, %ymm7
	vpsrlq	$32, %ymm7, %ymm7
	vpsrlq	$32, %ymm5, %ymm8
	vpmuludq	%ymm0, %ymm8, %ymm8
	vpsrlq	$32, %ymm3, %ymm3
	vpmuludq	%ymm1, %ymm3, %ymm3
	vpaddq	%ymm3, %ymm8, %ymm3
	vpblendd	$170, %ymm3, %ymm7, %ymm7       # ymm7 = ymm7[0],ymm3[1],ymm7[2],ymm3[3],ymm7[4],ymm3[5],ymm7[6],ymm3[7]
	vpbroadcastd	.LCPI15_2(%rip), %ymm3  # ymm3 = [3296722943,3296722943,3296722943,3296722943,3296722943,3296722943,3296722943,3296722943]
	vpaddd	%ymm3, %ymm7, %ymm8
	vpminud	%ymm8, %ymm7, %ymm7
	vpaddd	%ymm6, %ymm7, %ymm6
	vpaddd	%ymm3, %ymm6, %ymm7
	vpminud	%ymm7, %ymm6, %ymm6
	vblendvps	%ymm4, %ymm6, %ymm5, %ymm5
	vmovaps	%ymm5, (%rdi)
	testq	%rsi, %rsi
	je	.LBB15_2
# %bb.1:
	movl	8(%rsp), %eax
	vextracti128	$1, %ymm4, %xmm5
	vpackssdw	%xmm5, %xmm4, %xmm5
	vmovdqa	(%rsi), %ymm6
	vmovdqa	(%rdx), %ymm4
	vpmulld	%ymm6, %ymm2, %ymm7
	vpmuludq	%ymm0, %ymm6, %ymm8
	vpmuludq	%ymm1, %ymm7, %ymm9
	vpaddq	%ymm8, %ymm9, %ymm8
	vpsrlq	$32, %ymm8, %ymm8
	vpsrlq	$32, %ymm6, %ymm9
	vpmuludq	%ymm0, %ymm9, %ymm9
	vpsrlq	$32, %ymm7, %ymm7
	vpmuludq	%ymm1, %ymm7, %ymm7
	vpaddq	%ymm7, %ymm9, %ymm7
	vpblendd	$170, %ymm7, %ymm8, %ymm7       # ymm7 = ymm8[0],ymm7[1],ymm8[2],ymm7[3],ymm8[4],ymm7[5],ymm8[6],ymm7[7]
	vpaddd	%ymm3, %ymm7, %ymm8
	vpminud	%ymm8, %ymm7, %ymm7
	vpaddd	%ymm3, %ymm7, %ymm8
	vpminud	%ymm8, %ymm7, %ymm7
	vpmovsxwd	%xmm5, %ymm5
	vblendvps	%ymm5, %ymm7, %ymm6, %ymm6
	vmovaps	%ymm6, (%rsi)
	vmovd	%eax, %xmm6
	vpbroadcastd	%xmm6, %ymm6
	vpmulld	%ymm4, %ymm2, %ymm2
	vpmuludq	%ymm0, %ymm4, %ymm7
	vpmuludq	%ymm1, %ymm2, %ymm8
	vpaddq	%ymm7, %ymm8, %ymm7
	vpsrlq	$32, %ymm7, %ymm7
	vpsrlq	$32, %ymm4, %ymm8
	vpmuludq	%ymm0, %ymm8, %ymm0
	vpsrlq	$32, %ymm2, %ymm2
	vpmuludq	%ymm1, %ymm2, %ymm1
	vpaddq	%ymm0, %ymm1, %ymm0
	vpblendd	$170, %ymm0, %ymm7, %ymm0       # ymm0 = ymm7[0],ymm0[1],ymm7[2],ymm0[3],ymm7[4],ymm0[5],ymm7[6],ymm0[7]
	vpaddd	%ymm3, %ymm0, %ymm1
	vpminud	%ymm1, %ymm0, %ymm0
	vpaddd	%ymm6, %ymm0, %ymm0
	vpaddd	%ymm3, %ymm0, %ymm1
	vpminud	%ymm1, %ymm0, %ymm0
	vblendvps	%ymm5, %ymm0, %ymm4, %ymm0
	vmovaps	%ymm0, (%rdx)
.LBB15_2:
	vzeroupper
	retq
.Lfunc_end15:
	.size	_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj, .Lfunc_end15-_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj
	.cfi_endproc
                                        # -- End function
	.section	".linker-options","e",@llvm_linker_options
	.ident	"Ubuntu clang version 18.1.3 (1ubuntu1)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
