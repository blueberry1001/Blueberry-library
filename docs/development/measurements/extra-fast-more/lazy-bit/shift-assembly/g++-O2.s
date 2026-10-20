	.file	"probe-affine.cpp"
	.text
	.section	.text._ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj,"axG",@progbits,_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj,comdat
	.p2align 4
	.weak	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
	.type	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj, @function
_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj:
.LFB8385:
	.cfi_startproc
	endbr64
	vmovdqa	.LC0(%rip), %ymm2
	vmovd	%esi, %xmm0
	vmovd	%edx, %xmm1
	movl	$-998244353, %eax
	vpbroadcastd	%xmm0, %ymm0
	vpbroadcastd	%xmm1, %ymm1
	vpcmpgtd	%ymm2, %ymm1, %ymm1
	vpcmpgtd	%ymm2, %ymm0, %ymm0
	vmovd	%eax, %xmm2
	vpshufd	$0, %xmm2, %xmm2
	vpandn	%ymm1, %ymm0, %ymm0
	vpand	(%rdi), %ymm0, %ymm0
	vextracti128	$0x1, %ymm0, %xmm1
	vpaddd	%xmm1, %xmm0, %xmm0
	vpaddd	%xmm2, %xmm0, %xmm1
	vpminud	%xmm1, %xmm0, %xmm0
	vpshufd	$78, %xmm0, %xmm1
	vpaddd	%xmm1, %xmm0, %xmm0
	vpaddd	%xmm2, %xmm0, %xmm1
	vpminud	%xmm1, %xmm0, %xmm0
	vpshufd	$177, %xmm0, %xmm1
	vpaddd	%xmm0, %xmm1, %xmm0
	vpaddd	%xmm2, %xmm0, %xmm2
	vpminud	%xmm2, %xmm0, %xmm0
	vmovd	%xmm0, %eax
	vzeroupper
	ret
	.cfi_endproc
.LFE8385:
	.size	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj, .-_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
	.section	.text._ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm,"axG",@progbits,_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm,comdat
	.align 2
	.p2align 4
	.weak	_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm
	.type	_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm, @function
_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm:
.LFB8380:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	movslq	%esi, %r10
	movq	%rdx, %r9
	xorl	%edx, %edx
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	movq	%r10, %r14
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	movq	%rdi, %r12
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	xorl	%ebx, %ebx
	subq	$56, %rsp
	.cfi_def_cfa_offset 112
	movq	184(%rdi,%r10,8), %rbp
	leaq	-1(%rbp,%rcx), %rax
	divq	%rbp
	xorl	%edx, %edx
	movq	%rax, %r13
	movl	%eax, %esi
	movq	%r8, %rax
	divq	%rbp
	movq	%rdx, (%rsp)
	movq	%rax, %r15
	cmpl	%eax, %r13d
	jnb	.L4
	movq	%r9, %r11
	salq	$4, %r11
	addq	88(%rdi,%r10,8), %r11
	cmpb	$0, 76(%rdi)
	jne	.L45
	movq	(%rdi), %rsi
	leal	-1(%r15), %edx
	movl	%r13d, %edi
	xorl	%ebx, %ebx
	addq	%r11, %rdi
	subl	%r13d, %edx
	addq	%rdi, %rdx
	leaq	(%rsi,%rdi,4), %rax
	leaq	4(%rsi,%rdx,4), %rsi
	.p2align 4,,10
	.p2align 3
.L7:
	movl	(%rax), %edx
	addq	$4, %rax
	addq	%rdx, %rbx
	cmpq	%rax, %rsi
	jne	.L7
	movabsq	$-8525806094425994177, %rax
	mulq	%rbx
	shrq	$29, %rdx
	imulq	$998244353, %rdx, %rdx
	subl	%edx, %ebx
.L4:
	testl	%r14d, %r14d
	je	.L3
	movq	%rcx, %rax
	xorl	%edx, %edx
	divq	%rbp
	movq	%rdx, %r13
	movq	%rax, %r11
	testq	%rdx, %rdx
	je	.L46
	movl	%eax, %edi
	leal	1(%rax), %eax
	imulq	%rbp, %rax
	movq	%rdi, 8(%rsp)
	cmpq	%rax, %r8
	jb	.L47
.L11:
	movq	8(%rsp), %rax
	salq	$4, %r9
	subl	$1, %r14d
	movq	%r13, %rcx
	movl	%r14d, %esi
	movq	%r12, %rdi
	movq	%rbp, %r8
	movq	%r11, 32(%rsp)
	leaq	(%r9,%rax), %rdx
	movq	%r10, 24(%rsp)
	movq	%r9, 16(%rsp)
	call	_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm
	movq	16(%rsp), %r9
	movq	24(%rsp), %r10
	movq	8(%rsp), %rdi
	movq	24(%r12), %rcx
	movq	%r9, %rdx
	addq	88(%r12,%r10,8), %rdx
	subq	96(%r12), %rdx
	addq	%rdi, %rdx
	movq	48(%r12), %rsi
	movq	32(%rsp), %r11
	movl	(%rcx,%rdx,4), %ecx
	movl	(%rsi,%rdx,4), %esi
	cmpl	$301989884, %ecx
	jne	.L24
	testl	%esi, %esi
	jne	.L24
	addl	%eax, %ebx
	cmpl	$998244352, %ebx
	jbe	.L43
.L16:
	subl	$998244353, %ebx
.L43:
	cmpq	$0, (%rsp)
	je	.L3
	cmpl	%r11d, %r15d
	je	.L3
.L10:
	movl	%r15d, %r15d
	movq	(%rsp), %r8
	movl	%r14d, %esi
	movq	%r12, %rdi
	leaq	(%r15,%r9), %rdx
	xorl	%ecx, %ecx
	movq	%r10, 16(%rsp)
	movq	%r9, 8(%rsp)
	call	_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm
	movq	16(%rsp), %r10
	movq	8(%rsp), %r9
	movq	24(%r12), %rdx
	addq	88(%r12,%r10,8), %r15
	subq	96(%r12), %r15
	addq	%r9, %r15
	movl	(%rdx,%r15,4), %esi
	movq	48(%r12), %rdx
	movl	(%rdx,%r15,4), %edi
	cmpl	$301989884, %esi
	jne	.L25
	testl	%edi, %edi
	je	.L18
.L25:
	movq	(%rsp), %r11
	movl	%eax, %eax
	movl	%r11d, %edx
	movl	%r11d, %ecx
	imulq	$324829455, %rdx, %rdx
	shrq	$32, %rdx
	subl	%edx, %ecx
	shrl	%ecx
	addl	%edx, %ecx
	movl	%r11d, %edx
	shrl	$29, %ecx
	imull	$998244353, %ecx, %ecx
	subl	%ecx, %edx
	imulq	%rdi, %rdx
	imull	$998244351, %edx, %ecx
	imulq	$998244353, %rcx, %rcx
	addq	%rcx, %rdx
	shrq	$32, %rdx
	cmpq	$998244352, %rdx
	leal	-998244353(%rdx), %ecx
	cmova	%ecx, %edx
	imulq	%rax, %rsi
	imull	$998244351, %esi, %eax
	imulq	$998244353, %rax, %rax
	addq	%rsi, %rax
	shrq	$32, %rax
	cmpq	$998244352, %rax
	leal	-998244353(%rax), %ecx
	cmova	%ecx, %eax
	addl	%edx, %eax
	leal	-998244353(%rax), %edx
	cmpl	$998244352, %eax
	cmova	%edx, %eax
.L18:
	addl	%eax, %ebx
	leal	-998244353(%rbx), %eax
	cmpl	$998244352, %ebx
	cmova	%eax, %ebx
.L3:
	addq	$56, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movl	%ebx, %eax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L46:
	.cfi_restore_state
	cmpq	$0, (%rsp)
	je	.L3
	salq	$4, %r9
	subl	$1, %r14d
	jmp	.L10
	.p2align 4,,10
	.p2align 3
.L24:
	subl	%r13d, %ebp
	movl	%eax, %eax
	movl	%ebp, %edi
	movl	%ebp, %edx
	imulq	$324829455, %rdi, %rdi
	shrq	$32, %rdi
	subl	%edi, %edx
	shrl	%edx
	addl	%edi, %edx
	shrl	$29, %edx
	imull	$998244353, %edx, %edx
	subl	%edx, %ebp
	imulq	%rsi, %rbp
	imull	$998244351, %ebp, %edx
	imulq	$998244353, %rdx, %rdx
	addq	%rbp, %rdx
	shrq	$32, %rdx
	cmpq	$998244352, %rdx
	leal	-998244353(%rdx), %esi
	cmova	%esi, %edx
	imulq	%rax, %rcx
	imull	$998244351, %ecx, %eax
	imulq	$998244353, %rax, %rax
	addq	%rcx, %rax
	shrq	$32, %rax
	cmpq	$998244352, %rax
	leal	-998244353(%rax), %ecx
	cmova	%ecx, %eax
	addl	%edx, %eax
	cmpl	$998244352, %eax
	leal	-998244353(%rax), %edx
	cmova	%edx, %eax
	addl	%eax, %ebx
	cmpl	$998244352, %ebx
	ja	.L16
	jmp	.L43
	.p2align 4,,10
	.p2align 3
.L45:
	cmpl	$7, %r13d
	ja	.L6
	movl	$8, %edx
	movq	%r8, 40(%rsp)
	cmpl	%edx, %eax
	movq	%rcx, 32(%rsp)
	cmovbe	%eax, %edx
	movq	(%rdi), %rax
	movq	%r9, 24(%rsp)
	movq	%r10, 16(%rsp)
	leaq	(%rax,%r11,4), %rdi
	movq	%r11, 8(%rsp)
	call	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
	movq	40(%rsp), %r8
	movq	32(%rsp), %rcx
	movq	24(%rsp), %r9
	movq	16(%rsp), %r10
	movl	%eax, %ebx
	movq	8(%rsp), %r11
.L6:
	cmpl	$8, %r15d
	jbe	.L4
	movl	$8, %eax
	leal	-8(%r15), %edx
	movq	%r8, 32(%rsp)
	cmpl	%eax, %r13d
	movq	%rcx, 24(%rsp)
	cmovnb	%r13d, %eax
	movq	%r9, 16(%rsp)
	movq	%r10, 8(%rsp)
	leal	-8(%rax), %esi
	movq	(%r12), %rax
	leaq	32(%rax,%r11,4), %rdi
	call	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
	movq	8(%rsp), %r10
	movq	16(%rsp), %r9
	addl	%eax, %ebx
	movq	24(%rsp), %rcx
	movq	32(%rsp), %r8
	leal	-998244353(%rbx), %eax
	cmpl	$998244352, %ebx
	cmova	%eax, %ebx
	jmp	.L4
	.p2align 4,,10
	.p2align 3
.L47:
	movq	%rdi, %rax
	imulq	%rbp, %rax
	subq	%rax, %r8
	movq	%r8, %rbp
	jmp	.L11
	.cfi_endproc
.LFE8380:
	.size	_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm, .-_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm
	.section	.text._ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj,"axG",@progbits,_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj,comdat
	.p2align 4
	.weak	_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj
	.type	_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj, @function
_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj:
.LFB8387:
	.cfi_startproc
	endbr64
	vmovd	%r9d, %xmm4
	imull	$998244351, %r9d, %r9d
	vmovd	%ecx, %xmm5
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	vmovdqa	.LC0(%rip), %ymm1
	vmovd	%r8d, %xmm0
	vpbroadcastd	%xmm5, %ymm5
	vmovdqa	(%rdi), %ymm6
	vpbroadcastd	%xmm0, %ymm0
	movl	$998244353, %ecx
	vpbroadcastd	%xmm4, %ymm4
	vpcmpgtd	%ymm1, %ymm0, %ymm0
	vpmuludq	%ymm4, %ymm6, %ymm7
	vmovd	%ecx, %xmm3
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	vpcmpgtd	%ymm1, %ymm5, %ymm5
	vmovd	%r9d, %xmm1
	vpbroadcastd	%xmm3, %ymm3
	movl	$-998244353, %ecx
	vpbroadcastd	%xmm1, %ymm1
	vpmulld	%ymm6, %ymm1, %ymm2
	vpandn	%ymm0, %ymm5, %ymm5
	vpmuludq	%ymm3, %ymm2, %ymm0
	vpsrlq	$32, %ymm2, %ymm2
	vpaddq	%ymm7, %ymm0, %ymm0
	vpmuludq	%ymm3, %ymm2, %ymm7
	vpsrlq	$32, %ymm6, %ymm2
	vpsrlq	$32, %ymm0, %ymm0
	vpmuludq	%ymm4, %ymm2, %ymm2
	vpaddq	%ymm7, %ymm2, %ymm2
	vpsrlq	$32, %ymm2, %ymm2
	vpsllq	$32, %ymm2, %ymm2
	vpor	%ymm2, %ymm0, %ymm0
	vmovd	%ecx, %xmm2
	vpbroadcastd	%xmm2, %ymm2
	vpaddd	%ymm2, %ymm0, %ymm7
	vpminud	%ymm7, %ymm0, %ymm0
	vpbroadcastd	24(%rbp), %ymm7
	vpaddd	%ymm7, %ymm0, %ymm0
	vpaddd	%ymm2, %ymm0, %ymm7
	vpminud	%ymm7, %ymm0, %ymm0
	vpxor	%xmm7, %xmm7, %xmm7
	vpcmpgtb	%ymm5, %ymm7, %ymm5
	vpblendvb	%ymm5, %ymm0, %ymm6, %ymm6
	vmovdqa	%ymm6, (%rdi)
	testq	%rsi, %rsi
	je	.L53
	vmovdqa	(%rsi), %ymm8
	vmovdqa	(%rdx), %ymm6
	vpmulld	%ymm8, %ymm1, %ymm7
	vpmuludq	%ymm4, %ymm8, %ymm0
	vpmulld	%ymm6, %ymm1, %ymm1
	vpmuludq	%ymm3, %ymm7, %ymm9
	vpsrlq	$32, %ymm7, %ymm7
	vpaddq	%ymm9, %ymm0, %ymm0
	vpmuludq	%ymm3, %ymm7, %ymm9
	vpsrlq	$32, %ymm8, %ymm7
	vpsrlq	$32, %ymm0, %ymm0
	vpmuludq	%ymm4, %ymm7, %ymm7
	vpaddq	%ymm9, %ymm7, %ymm7
	vpsrlq	$32, %ymm7, %ymm7
	vpsllq	$32, %ymm7, %ymm7
	vpor	%ymm7, %ymm0, %ymm0
	vpaddd	%ymm2, %ymm0, %ymm7
	vpminud	%ymm7, %ymm0, %ymm0
	vpaddd	%ymm2, %ymm0, %ymm7
	vpminud	%ymm7, %ymm0, %ymm0
	vpmuludq	%ymm3, %ymm1, %ymm7
	vpsrlq	$32, %ymm1, %ymm1
	vpblendvb	%ymm5, %ymm0, %ymm8, %ymm8
	vpmuludq	%ymm3, %ymm1, %ymm1
	vpsrlq	$32, %ymm6, %ymm3
	vmovdqa	%ymm8, (%rsi)
	vpmuludq	%ymm4, %ymm3, %ymm3
	vpmuludq	%ymm4, %ymm6, %ymm0
	vpaddq	%ymm1, %ymm3, %ymm1
	vpaddq	%ymm7, %ymm0, %ymm0
	vpsrlq	$32, %ymm1, %ymm1
	vpsrlq	$32, %ymm0, %ymm0
	vpsllq	$32, %ymm1, %ymm1
	vpor	%ymm1, %ymm0, %ymm0
	vpaddd	%ymm2, %ymm0, %ymm1
	vpminud	%ymm1, %ymm0, %ymm0
	vpbroadcastd	16(%rbp), %ymm1
	vpaddd	%ymm1, %ymm0, %ymm0
	vpaddd	%ymm2, %ymm0, %ymm2
	vpminud	%ymm2, %ymm0, %ymm0
	vpblendvb	%ymm5, %ymm0, %ymm6, %ymm6
	vmovdqa	%ymm6, (%rdx)
.L53:
	vzeroupper
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8387:
	.size	_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj, .-_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj
	.section	.text._ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj,"axG",@progbits,_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj,comdat
	.align 2
	.p2align 4
	.weak	_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj
	.type	_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj, @function
_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj:
.LFB8375:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	movl	%ecx, %r10d
	movslq	%esi, %rcx
	movq	%rdx, %rax
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	salq	$4, %rax
	movq	%rdi, %r14
	movl	%r8d, %r15d
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$24, %rsp
	.cfi_def_cfa_offset 80
	movl	280(%rdi,%rcx,4), %edx
	addq	88(%rdi,%rcx,8), %rax
	movl	80(%rsp), %r12d
	movl	%r12d, %ecx
	imulq	%rcx, %rdx
	imull	$998244351, %edx, %ebx
	imulq	$998244353, %rbx, %rbx
	addq	%rdx, %rbx
	shrq	$32, %rbx
	leal	-998244353(%rbx), %edx
	cmpq	$998244352, %rbx
	cmova	%edx, %ebx
	testl	%esi, %esi
	je	.L72
	movq	24(%rdi), %r13
	movq	%rax, %rbp
	subq	96(%rdi), %rbp
	salq	$2, %rbp
	addq	%rbp, %r13
	addq	48(%rdi), %rbp
	cmpb	$0, 76(%r14)
	jne	.L58
.L82:
	cmpl	%r15d, %r10d
	jnb	.L55
	subl	$1, %r15d
	movl	%r10d, %edx
	movq	(%r14), %rdi
	movl	%r9d, %r9d
	subl	%r10d, %r15d
	leaq	0(,%rdx,4), %rcx
	leaq	1(%rdx,%r15), %rdx
	leaq	(%rdi,%rax,4), %rdi
	salq	$2, %rdx
	.p2align 4,,10
	.p2align 3
.L71:
	movl	(%rdi,%rcx), %r8d
	imulq	%r9, %r8
	imull	$998244351, %r8d, %eax
	imulq	$998244353, %rax, %rax
	addq	%r8, %rax
	shrq	$32, %rax
	cmpq	$998244352, %rax
	leal	-998244353(%rax), %r8d
	cmova	%r8d, %eax
	addl	%ebx, %eax
	leal	-998244353(%rax), %r8d
	cmpl	$998244352, %eax
	cmova	%r8d, %eax
	movl	%eax, (%rdi,%rcx)
	testl	%esi, %esi
	je	.L67
	movl	0(%r13,%rcx), %r8d
	imulq	%r9, %r8
	imull	$998244351, %r8d, %eax
	imulq	$998244353, %rax, %rax
	addq	%r8, %rax
	shrq	$32, %rax
	cmpq	$998244352, %rax
	leal	-998244353(%rax), %r8d
	cmova	%r8d, %eax
	movl	%eax, 0(%r13,%rcx)
	movl	0(%rbp,%rcx), %r8d
	imulq	%r9, %r8
	imull	$998244351, %r8d, %eax
	imulq	$998244353, %rax, %rax
	addq	%r8, %rax
	shrq	$32, %rax
	cmpq	$998244352, %rax
	leal	-998244353(%rax), %r8d
	cmova	%r8d, %eax
	addl	%r12d, %eax
	leal	-998244353(%rax), %r8d
	cmpl	$998244352, %eax
	cmova	%r8d, %eax
	movl	%eax, 0(%rbp,%rcx)
.L67:
	addq	$4, %rcx
	cmpq	%rcx, %rdx
	jne	.L71
.L55:
	addq	$24, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L72:
	.cfi_restore_state
	xorl	%r13d, %r13d
	xorl	%ebp, %ebp
	cmpb	$0, 76(%r14)
	je	.L82
.L58:
	cmpl	$7, %r10d
	jbe	.L83
.L61:
	cmpl	$8, %r15d
	jbe	.L55
	movl	$8, %edx
	leal	-8(%r15), %r8d
	cmpl	%edx, %r10d
	cmovnb	%r10d, %edx
	testq	%rbp, %rbp
	leal	-8(%rdx), %ecx
	leaq	32(%rbp), %rdx
	cmovne	%rdx, %rbp
	testq	%r13, %r13
	leaq	32(%r13), %rdx
	cmovne	%rdx, %r13
	movq	(%r14), %rdx
	pushq	%rbx
	.cfi_def_cfa_offset 88
	pushq	%r12
	.cfi_def_cfa_offset 96
	leaq	32(%rdx,%rax,4), %rdi
	movq	%r13, %rsi
	movq	%rbp, %rdx
	call	_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj
	popq	%rax
	.cfi_def_cfa_offset 88
	popq	%rdx
	.cfi_def_cfa_offset 80
	addq	$24, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L83:
	.cfi_restore_state
	movl	$8, %r8d
	movq	(%r14), %rdx
	movl	%r10d, %ecx
	movq	%r13, %rsi
	movq	%rax, 8(%rsp)
	cmpl	%r8d, %r15d
	pushq	%rbx
	.cfi_def_cfa_offset 88
	cmovbe	%r15d, %r8d
	leaq	(%rdx,%rax,4), %rdi
	movq	%rbp, %rdx
	pushq	%r12
	.cfi_def_cfa_offset 96
	movl	%r9d, 20(%rsp)
	movl	%r10d, 16(%rsp)
	call	_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj
	popq	%rcx
	.cfi_def_cfa_offset 88
	popq	%rsi
	.cfi_def_cfa_offset 80
	movq	8(%rsp), %rax
	movl	4(%rsp), %r9d
	movl	(%rsp), %r10d
	jmp	.L61
	.cfi_endproc
.LFE8375:
	.size	_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj, .-_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj
	.section	.text._ZN9blueberry10extra_fast22AffineSumSegmentTree1612update_childEimjmmjj,"axG",@progbits,_ZN9blueberry10extra_fast22AffineSumSegmentTree1612update_childEimjmmjj,comdat
	.align 2
	.p2align 4
	.weak	_ZN9blueberry10extra_fast22AffineSumSegmentTree1612update_childEimjmmjj
	.type	_ZN9blueberry10extra_fast22AffineSumSegmentTree1612update_childEimjmmjj, @function
_ZN9blueberry10extra_fast22AffineSumSegmentTree1612update_childEimjmmjj:
.LFB8377:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	movslq	%esi, %rax
	salq	$4, %rdx
	movl	%ecx, %ecx
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	leaq	(%rdi,%rax,8), %r12
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	movq	%rax, %rbp
	leaq	(%rdx,%rcx), %rax
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subl	$1, %ebp
	movq	%rdi, %rbx
	subq	$72, %rsp
	.cfi_def_cfa_offset 128
	movq	88(%r12), %r14
	movq	96(%rdi), %rsi
	movq	24(%rdi), %r15
	movq	48(%rdi), %rdx
	movq	%r8, 16(%rsp)
	addq	%rax, %r14
	movq	%r9, 32(%rsp)
	subq	%rsi, %r14
	movq	%rax, 8(%rsp)
	salq	$4, %rax
	salq	$2, %r14
	movq	%rax, 24(%rsp)
	leaq	(%r15,%r14), %r8
	leaq	(%rdx,%r14), %r9
	movl	(%r8), %r13d
	movl	(%r9), %r10d
	cmpl	$301989884, %r13d
	jne	.L112
	testl	%r10d, %r10d
	jne	.L112
.L85:
	movq	176(%r12), %r13
	movq	16(%rsp), %rax
	xorl	%edx, %edx
	leaq	-1(%r13,%rax), %rax
	divq	%r13
	xorl	%edx, %edx
	movq	%rax, %rcx
	movq	32(%rsp), %rax
	divq	%r13
	movl	%eax, 40(%rsp)
	movq	%rdx, %r14
	movq	%rax, %r15
	cmpl	%eax, %ecx
	jb	.L134
.L100:
	testl	%ebp, %ebp
	je	.L102
	movq	16(%rsp), %rax
	xorl	%edx, %edx
	divq	%r13
	testq	%rdx, %rdx
	je	.L135
	leal	1(%rax), %ecx
	movq	32(%rsp), %rdi
	movl	%eax, %esi
	imulq	%r13, %rcx
	cmpq	%rcx, %rdi
	jb	.L136
.L106:
	movq	%rax, 16(%rsp)
	movq	%rdx, %r8
	movl	%esi, %ecx
	movq	%rbx, %rdi
	movl	136(%rsp), %eax
	movq	%r13, %r9
	movl	%ebp, %esi
	pushq	%rax
	.cfi_def_cfa_offset 136
	movl	136(%rsp), %eax
	pushq	%rax
	.cfi_def_cfa_offset 144
	movq	24(%rsp), %rdx
	call	_ZN9blueberry10extra_fast22AffineSumSegmentTree1612update_childEimjmmjj
	popq	%rdi
	.cfi_def_cfa_offset 136
	popq	%r8
	.cfi_def_cfa_offset 128
	testq	%r14, %r14
	je	.L102
	movq	16(%rsp), %rax
	cmpl	%eax, %r15d
	jne	.L104
	.p2align 4,,10
	.p2align 3
.L102:
	movq	24(%rsp), %rax
	addq	80(%r12), %rax
	cmpb	$0, 76(%rbx)
	movq	(%rbx), %rbp
	jne	.L137
	leaq	0(%rbp,%rax,4), %rax
	pxor	%xmm1, %xmm1
	pxor	%xmm2, %xmm2
	leaq	64(%rax), %rdx
.L109:
	movdqu	(%rax), %xmm0
	addq	$16, %rax
	movdqa	%xmm0, %xmm3
	punpckhdq	%xmm2, %xmm0
	punpckldq	%xmm2, %xmm3
	paddq	%xmm3, %xmm0
	paddq	%xmm0, %xmm1
	cmpq	%rdx, %rax
	jne	.L109
	movabsq	$-8525806094425994177, %rax
	movdqa	%xmm1, %xmm0
	psrldq	$8, %xmm0
	paddq	%xmm0, %xmm1
	movq	%xmm1, %rcx
	mulq	%rcx
	movl	%ecx, %eax
	shrq	$29, %rdx
	imulq	$998244353, %rdx, %rdx
	subl	%edx, %eax
.L108:
	movq	8(%rsp), %rbx
	addq	88(%r12), %rbx
	movl	%eax, 0(%rbp,%rbx,4)
	addq	$72, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L135:
	.cfi_restore_state
	testq	%r14, %r14
	je	.L102
.L104:
	movl	136(%rsp), %eax
	movl	%ebp, %esi
	movq	%r14, %r9
	xorl	%r8d, %r8d
	movq	%rbx, %rdi
	pushq	%rax
	.cfi_def_cfa_offset 136
	movl	136(%rsp), %eax
	pushq	%rax
	.cfi_def_cfa_offset 144
	movl	56(%rsp), %ecx
	movq	24(%rsp), %rdx
	call	_ZN9blueberry10extra_fast22AffineSumSegmentTree1612update_childEimjmmjj
	popq	%rcx
	.cfi_def_cfa_offset 136
	popq	%rsi
	.cfi_def_cfa_offset 128
	jmp	.L102
	.p2align 4,,10
	.p2align 3
.L112:
	movslq	%ebp, %rax
	movq	24(%rsp), %rcx
	addq	80(%r12), %rcx
	movl	280(%rbx,%rax,4), %edi
	movl	%r10d, %eax
	imulq	%rax, %rdi
	imull	$998244351, %edi, %eax
	imulq	$998244353, %rax, %rax
	addq	%rdi, %rax
	shrq	$32, %rax
	cmpq	$998244352, %rax
	leal	-998244353(%rax), %edi
	cmovbe	%eax, %edi
	movzbl	76(%rbx), %eax
	movl	%edi, %r11d
	leaq	0(,%rcx,4), %rdi
	movq	%rdi, 40(%rsp)
	addq	(%rbx), %rdi
	testl	%ebp, %ebp
	je	.L88
	subq	%rsi, %rcx
	salq	$2, %rcx
	addq	%rcx, %r15
	addq	%rcx, %rdx
	testb	%al, %al
	jne	.L89
.L111:
	xorl	%ecx, %ecx
	.p2align 4,,10
	.p2align 3
.L90:
	movl	(%rdi,%rcx), %esi
	imulq	%r13, %rsi
	imull	$998244351, %esi, %eax
	imulq	$998244353, %rax, %rax
	addq	%rsi, %rax
	shrq	$32, %rax
	cmpq	$998244352, %rax
	leal	-998244353(%rax), %esi
	cmova	%esi, %eax
	addl	%r11d, %eax
	leal	-998244353(%rax), %esi
	cmpl	$998244352, %eax
	cmova	%esi, %eax
	movl	%eax, (%rdi,%rcx)
	testl	%ebp, %ebp
	je	.L96
	movl	(%r15,%rcx), %esi
	imulq	%r13, %rsi
	imull	$998244351, %esi, %eax
	imulq	$998244353, %rax, %rax
	addq	%rsi, %rax
	shrq	$32, %rax
	cmpq	$998244352, %rax
	leal	-998244353(%rax), %esi
	cmova	%esi, %eax
	movl	%eax, (%r15,%rcx)
	movl	(%rdx,%rcx), %esi
	imulq	%r13, %rsi
	imull	$998244351, %esi, %eax
	imulq	$998244353, %rax, %rax
	addq	%rsi, %rax
	shrq	$32, %rax
	cmpq	$998244352, %rax
	leal	-998244353(%rax), %esi
	cmova	%esi, %eax
	addl	%r10d, %eax
	leal	-998244353(%rax), %esi
	cmpl	$998244352, %eax
	cmova	%esi, %eax
	movl	%eax, (%rdx,%rcx)
.L96:
	addq	$4, %rcx
	cmpq	$64, %rcx
	jne	.L90
	movl	$301989884, (%r8)
	movl	$0, (%r9)
	jmp	.L85
	.p2align 4,,10
	.p2align 3
.L137:
	leaq	0(,%rax,4), %r13
	xorl	%esi, %esi
	movl	$8, %edx
	leaq	0(%rbp,%r13), %rdi
	call	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
	movl	$8, %edx
	xorl	%esi, %esi
	leaq	32(%rbp,%r13), %rdi
	movl	%eax, %ebx
	call	_ZN9blueberry10extra_fast22AffineSumSegmentTree169fold_simdEPKjjj
	addl	%ebx, %eax
	leal	-998244353(%rax), %edx
	cmpl	$998244352, %eax
	cmova	%edx, %eax
	jmp	.L108
	.p2align 4,,10
	.p2align 3
.L134:
	subq	$8, %rsp
	.cfi_def_cfa_offset 136
	movl	%r15d, %r8d
	movl	%ebp, %esi
	movq	%rbx, %rdi
	movl	144(%rsp), %eax
	pushq	%rax
	.cfi_def_cfa_offset 144
	movl	144(%rsp), %r9d
	movq	24(%rsp), %rdx
	call	_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj
	popq	%r9
	.cfi_def_cfa_offset 136
	popq	%r10
	.cfi_def_cfa_offset 128
	jmp	.L100
	.p2align 4,,10
	.p2align 3
.L88:
	testb	%al, %al
	jne	.L110
	xorl	%edx, %edx
	xorl	%r15d, %r15d
	jmp	.L111
	.p2align 4,,10
	.p2align 3
.L136:
	movl	%eax, %ecx
	imulq	%r13, %rcx
	subq	%rcx, %rdi
	movq	%rdi, %r13
	jmp	.L106
	.p2align 4,,10
	.p2align 3
.L89:
	pushq	%r11
	.cfi_def_cfa_offset 136
	xorl	%ecx, %ecx
	movl	%r13d, %r9d
	movl	$8, %r8d
	movq	%r15, %rsi
	movl	%r11d, 68(%rsp)
	pushq	%r10
	.cfi_def_cfa_offset 144
	movl	%r10d, 72(%rsp)
	movq	%rdx, 64(%rsp)
	call	_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj
	popq	%rax
	.cfi_def_cfa_offset 136
	popq	%rdx
	.cfi_def_cfa_offset 128
	movq	48(%rsp), %rdx
	movl	56(%rsp), %r10d
	movl	60(%rsp), %r11d
	testq	%rdx, %rdx
	je	.L91
	addq	$32, %rdx
.L91:
	leaq	32(%r15), %rcx
	testq	%r15, %r15
	cmovne	%rcx, %r15
.L92:
	movq	40(%rsp), %rax
	movq	(%rbx), %rcx
	pushq	%r11
	.cfi_def_cfa_offset 136
	movl	%r13d, %r9d
	pushq	%r10
	.cfi_def_cfa_offset 144
	movl	$8, %r8d
	movq	%r15, %rsi
	leaq	32(%rcx,%rax), %rdi
	xorl	%ecx, %ecx
	call	_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj
	movq	24(%rbx), %r8
	movq	48(%rbx), %r9
	popq	%r11
	.cfi_def_cfa_offset 136
	popq	%r13
	.cfi_def_cfa_offset 128
	addq	%r14, %r8
	addq	%r14, %r9
	movl	$301989884, (%r8)
	movl	$0, (%r9)
	jmp	.L85
.L110:
	pushq	%r11
	.cfi_def_cfa_offset 136
	xorl	%edx, %edx
	movl	%r13d, %r9d
	movl	$8, %r8d
	xorl	%ecx, %ecx
	xorl	%esi, %esi
	xorl	%r15d, %r15d
	movl	%r11d, 64(%rsp)
	pushq	%r10
	.cfi_def_cfa_offset 144
	movl	%r10d, 64(%rsp)
	call	_ZN9blueberry10extra_fast22AffineSumSegmentTree1614transform_simdEPjS2_S2_jjjjj
	popq	%rax
	.cfi_def_cfa_offset 136
	popq	%rdx
	.cfi_def_cfa_offset 128
	xorl	%edx, %edx
	movl	48(%rsp), %r10d
	movl	56(%rsp), %r11d
	jmp	.L92
	.cfi_endproc
.LFE8377:
	.size	_ZN9blueberry10extra_fast22AffineSumSegmentTree1612update_childEimjmmjj, .-_ZN9blueberry10extra_fast22AffineSumSegmentTree1612update_childEimjmmjj
	.section	.text._ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj,"axG",@progbits,_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj,comdat
	.p2align 4
	.weak	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
	.type	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj, @function
_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj:
.LFB8446:
	.cfi_startproc
	endbr64
	vmovdqa	.LC0(%rip), %ymm2
	vmovd	%esi, %xmm0
	vmovd	%edx, %xmm1
	movl	$-998244353, %eax
	vpbroadcastd	%xmm0, %ymm0
	vpbroadcastd	%xmm1, %ymm1
	vpcmpgtd	%ymm2, %ymm1, %ymm1
	vpcmpgtd	%ymm2, %ymm0, %ymm0
	vmovd	%eax, %xmm2
	vpshufd	$0, %xmm2, %xmm2
	vpandn	%ymm1, %ymm0, %ymm0
	vpand	(%rdi), %ymm0, %ymm0
	vextracti128	$0x1, %ymm0, %xmm1
	vpaddd	%xmm1, %xmm0, %xmm0
	vpaddd	%xmm2, %xmm0, %xmm1
	vpminud	%xmm1, %xmm0, %xmm0
	vpshufd	$78, %xmm0, %xmm1
	vpaddd	%xmm1, %xmm0, %xmm0
	vpaddd	%xmm2, %xmm0, %xmm1
	vpminud	%xmm1, %xmm0, %xmm0
	vpshufd	$177, %xmm0, %xmm1
	vpaddd	%xmm0, %xmm1, %xmm0
	vpaddd	%xmm2, %xmm0, %xmm2
	vpminud	%xmm2, %xmm0, %xmm0
	vmovd	%xmm0, %eax
	vzeroupper
	ret
	.cfi_endproc
.LFE8446:
	.size	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj, .-_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
	.section	.text._ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm,"axG",@progbits,_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm,comdat
	.align 2
	.p2align 4
	.weak	_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm
	.type	_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm, @function
_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm:
.LFB8441:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	movq	%r8, %r11
	movl	$1, %r8d
	movq	%rdx, %r15
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	movq	%r11, %r14
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	movl	%esi, %r13d
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	movq	%rcx, %r12
	leal	0(,%rsi,4), %ecx
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	salq	%cl, %r8
	shrq	%cl, %r14
	movq	%rdi, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	leaq	-1(%r8,%r12), %r9
	xorl	%ebx, %ebx
	shrq	%cl, %r9
	subq	$56, %rsp
	.cfi_def_cfa_offset 112
	cmpl	%r14d, %r9d
	jnb	.L140
	movslq	%esi, %rax
	movq	%rdx, %r10
	salq	$4, %r10
	addq	88(%rdi,%rax,8), %r10
	cmpb	$0, 76(%rdi)
	jne	.L180
	movq	(%rdi), %rsi
	leal	-1(%r14), %edx
	movl	%r9d, %edi
	xorl	%ebx, %ebx
	addq	%r10, %rdi
	subl	%r9d, %edx
	addq	%rdi, %rdx
	leaq	(%rsi,%rdi,4), %rax
	leaq	4(%rsi,%rdx,4), %rsi
	.p2align 4,,10
	.p2align 3
.L143:
	movl	(%rax), %edx
	addq	$4, %rax
	addq	%rdx, %rbx
	cmpq	%rax, %rsi
	jne	.L143
	movabsq	$-8525806094425994177, %rax
	mulq	%rbx
	shrq	$29, %rdx
	imulq	$998244353, %rdx, %rdx
	subl	%edx, %ebx
.L140:
	testl	%r13d, %r13d
	je	.L139
	leaq	-1(%r8), %rax
	movq	%rax, %r9
	andq	%r11, %r9
	andq	%r12, %rax
	movq	%rax, 8(%rsp)
	je	.L181
	shrq	%cl, %r12
	movq	%r11, %rdx
	leal	-1(%r13), %esi
	movq	%r9, 40(%rsp)
	movl	%r12d, %r10d
	leal	1(%r12), %eax
	movl	%esi, 16(%rsp)
	movq	%r10, %rdi
	salq	%cl, %rax
	movq	%r10, 32(%rsp)
	salq	%cl, %rdi
	movq	8(%rsp), %rcx
	subq	%rdi, %rdx
	cmpq	%rax, %r11
	movq	%rbp, %rdi
	cmovb	%rdx, %r8
	salq	$4, %r15
	leaq	(%r15,%r10), %rdx
	movq	%r8, 24(%rsp)
	call	_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm
	movq	32(%rsp), %r10
	movslq	%r13d, %r11
	movq	%r15, %rdx
	movq	24(%rbp), %rcx
	subq	96(%rbp), %rdx
	addq	88(%rbp,%r11,8), %rdx
	movq	48(%rbp), %rdi
	addq	%r10, %rdx
	movl	16(%rsp), %esi
	movq	24(%rsp), %r8
	movl	(%rcx,%rdx,4), %ecx
	movl	(%rdi,%rdx,4), %edi
	movq	40(%rsp), %r9
	cmpl	$301989884, %ecx
	jne	.L160
	testl	%edi, %edi
	jne	.L160
	addl	%eax, %ebx
	cmpl	$998244352, %ebx
	jbe	.L179
.L152:
	subl	$998244353, %ebx
.L179:
	testq	%r9, %r9
	je	.L139
	cmpl	%r12d, %r14d
	je	.L139
.L146:
	movl	%r14d, %r14d
	xorl	%ecx, %ecx
	movq	%r9, %r8
	movq	%rbp, %rdi
	leaq	(%r14,%r15), %rdx
	movq	%r11, 16(%rsp)
	movq	%r9, 8(%rsp)
	call	_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm
	movq	16(%rsp), %r11
	movq	24(%rbp), %rdx
	subq	96(%rbp), %r14
	movq	8(%rsp), %r9
	addq	%r15, %r14
	addq	88(%rbp,%r11,8), %r14
	movl	(%rdx,%r14,4), %ecx
	movq	48(%rbp), %rdx
	cmpl	$301989884, %ecx
	movl	(%rdx,%r14,4), %edi
	jne	.L161
	testl	%edi, %edi
	je	.L154
.L161:
	movl	%r9d, %esi
	movl	%r9d, %edx
	movl	%eax, %eax
	imulq	$324829455, %rsi, %rsi
	shrq	$32, %rsi
	subl	%esi, %edx
	shrl	%edx
	addl	%esi, %edx
	shrl	$29, %edx
	imull	$998244353, %edx, %edx
	subl	%edx, %r9d
	imulq	%rdi, %r9
	imull	$998244351, %r9d, %edx
	imulq	$998244353, %rdx, %rdx
	addq	%r9, %rdx
	shrq	$32, %rdx
	cmpq	$998244352, %rdx
	leal	-998244353(%rdx), %esi
	cmova	%esi, %edx
	imulq	%rax, %rcx
	imull	$998244351, %ecx, %eax
	imulq	$998244353, %rax, %rax
	addq	%rcx, %rax
	shrq	$32, %rax
	cmpq	$998244352, %rax
	leal	-998244353(%rax), %ecx
	cmova	%ecx, %eax
	addl	%edx, %eax
	leal	-998244353(%rax), %edx
	cmpl	$998244352, %eax
	cmova	%edx, %eax
.L154:
	addl	%eax, %ebx
	leal	-998244353(%rbx), %eax
	cmpl	$998244352, %ebx
	cmova	%eax, %ebx
.L139:
	addq	$56, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movl	%ebx, %eax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L181:
	.cfi_restore_state
	testq	%r9, %r9
	je	.L139
	movslq	%r13d, %r11
	salq	$4, %r15
	leal	-1(%r13), %esi
	jmp	.L146
	.p2align 4,,10
	.p2align 3
.L160:
	movl	8(%rsp), %edx
	movl	%eax, %eax
	subl	%edx, %r8d
	movl	%r8d, %r10d
	movl	%r8d, %edx
	imulq	$324829455, %r10, %r10
	shrq	$32, %r10
	subl	%r10d, %edx
	shrl	%edx
	addl	%r10d, %edx
	shrl	$29, %edx
	imull	$998244353, %edx, %edx
	subl	%edx, %r8d
	imulq	%rdi, %r8
	imull	$998244351, %r8d, %edx
	imulq	$998244353, %rdx, %rdx
	addq	%r8, %rdx
	shrq	$32, %rdx
	cmpq	$998244352, %rdx
	leal	-998244353(%rdx), %edi
	cmova	%edi, %edx
	imulq	%rax, %rcx
	imull	$998244351, %ecx, %eax
	imulq	$998244353, %rax, %rax
	addq	%rcx, %rax
	shrq	$32, %rax
	cmpq	$998244352, %rax
	leal	-998244353(%rax), %ecx
	cmova	%ecx, %eax
	addl	%edx, %eax
	cmpl	$998244352, %eax
	leal	-998244353(%rax), %edx
	cmova	%edx, %eax
	addl	%eax, %ebx
	cmpl	$998244352, %ebx
	ja	.L152
	jmp	.L179
	.p2align 4,,10
	.p2align 3
.L180:
	cmpl	$7, %r9d
	ja	.L142
	movl	$8, %edx
	movq	(%rdi), %rax
	movl	%r9d, %esi
	movq	%r11, 40(%rsp)
	cmpl	%edx, %r14d
	movl	%ecx, 32(%rsp)
	cmovbe	%r14d, %edx
	leaq	(%rax,%r10,4), %rdi
	movq	%r8, 24(%rsp)
	movq	%r9, 16(%rsp)
	movq	%r10, 8(%rsp)
	call	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
	movq	40(%rsp), %r11
	movl	32(%rsp), %ecx
	movq	24(%rsp), %r8
	movq	16(%rsp), %r9
	movl	%eax, %ebx
	movq	8(%rsp), %r10
.L142:
	cmpl	$8, %r14d
	jbe	.L140
	movl	$8, %eax
	leal	-8(%r14), %edx
	movq	%r11, 24(%rsp)
	cmpl	%eax, %r9d
	movl	%ecx, 16(%rsp)
	cmovnb	%r9d, %eax
	movq	%r8, 8(%rsp)
	leal	-8(%rax), %esi
	movq	0(%rbp), %rax
	leaq	32(%rax,%r10,4), %rdi
	call	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
	movq	8(%rsp), %r8
	movl	16(%rsp), %ecx
	addl	%eax, %ebx
	movq	24(%rsp), %r11
	leal	-998244353(%rbx), %eax
	cmpl	$998244352, %ebx
	cmova	%eax, %ebx
	jmp	.L140
	.cfi_endproc
.LFE8441:
	.size	_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm, .-_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm
	.section	.text._ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj,"axG",@progbits,_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj,comdat
	.p2align 4
	.weak	_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj
	.type	_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj, @function
_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj:
.LFB8448:
	.cfi_startproc
	endbr64
	vmovd	%r9d, %xmm4
	imull	$998244351, %r9d, %r9d
	vmovd	%ecx, %xmm5
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	vmovdqa	.LC0(%rip), %ymm1
	vmovd	%r8d, %xmm0
	vpbroadcastd	%xmm5, %ymm5
	vmovdqa	(%rdi), %ymm6
	vpbroadcastd	%xmm0, %ymm0
	movl	$998244353, %ecx
	vpbroadcastd	%xmm4, %ymm4
	vpcmpgtd	%ymm1, %ymm0, %ymm0
	vpmuludq	%ymm4, %ymm6, %ymm7
	vmovd	%ecx, %xmm3
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	vpcmpgtd	%ymm1, %ymm5, %ymm5
	vmovd	%r9d, %xmm1
	vpbroadcastd	%xmm3, %ymm3
	movl	$-998244353, %ecx
	vpbroadcastd	%xmm1, %ymm1
	vpmulld	%ymm6, %ymm1, %ymm2
	vpandn	%ymm0, %ymm5, %ymm5
	vpmuludq	%ymm3, %ymm2, %ymm0
	vpsrlq	$32, %ymm2, %ymm2
	vpaddq	%ymm7, %ymm0, %ymm0
	vpmuludq	%ymm3, %ymm2, %ymm7
	vpsrlq	$32, %ymm6, %ymm2
	vpsrlq	$32, %ymm0, %ymm0
	vpmuludq	%ymm4, %ymm2, %ymm2
	vpaddq	%ymm7, %ymm2, %ymm2
	vpsrlq	$32, %ymm2, %ymm2
	vpsllq	$32, %ymm2, %ymm2
	vpor	%ymm2, %ymm0, %ymm0
	vmovd	%ecx, %xmm2
	vpbroadcastd	%xmm2, %ymm2
	vpaddd	%ymm2, %ymm0, %ymm7
	vpminud	%ymm7, %ymm0, %ymm0
	vpbroadcastd	24(%rbp), %ymm7
	vpaddd	%ymm7, %ymm0, %ymm0
	vpaddd	%ymm2, %ymm0, %ymm7
	vpminud	%ymm7, %ymm0, %ymm0
	vpxor	%xmm7, %xmm7, %xmm7
	vpcmpgtb	%ymm5, %ymm7, %ymm5
	vpblendvb	%ymm5, %ymm0, %ymm6, %ymm6
	vmovdqa	%ymm6, (%rdi)
	testq	%rsi, %rsi
	je	.L187
	vmovdqa	(%rsi), %ymm8
	vmovdqa	(%rdx), %ymm6
	vpmulld	%ymm8, %ymm1, %ymm7
	vpmuludq	%ymm4, %ymm8, %ymm0
	vpmulld	%ymm6, %ymm1, %ymm1
	vpmuludq	%ymm3, %ymm7, %ymm9
	vpsrlq	$32, %ymm7, %ymm7
	vpaddq	%ymm9, %ymm0, %ymm0
	vpmuludq	%ymm3, %ymm7, %ymm9
	vpsrlq	$32, %ymm8, %ymm7
	vpsrlq	$32, %ymm0, %ymm0
	vpmuludq	%ymm4, %ymm7, %ymm7
	vpaddq	%ymm9, %ymm7, %ymm7
	vpsrlq	$32, %ymm7, %ymm7
	vpsllq	$32, %ymm7, %ymm7
	vpor	%ymm7, %ymm0, %ymm0
	vpaddd	%ymm2, %ymm0, %ymm7
	vpminud	%ymm7, %ymm0, %ymm0
	vpaddd	%ymm2, %ymm0, %ymm7
	vpminud	%ymm7, %ymm0, %ymm0
	vpmuludq	%ymm3, %ymm1, %ymm7
	vpsrlq	$32, %ymm1, %ymm1
	vpblendvb	%ymm5, %ymm0, %ymm8, %ymm8
	vpmuludq	%ymm3, %ymm1, %ymm1
	vpsrlq	$32, %ymm6, %ymm3
	vmovdqa	%ymm8, (%rsi)
	vpmuludq	%ymm4, %ymm3, %ymm3
	vpmuludq	%ymm4, %ymm6, %ymm0
	vpaddq	%ymm1, %ymm3, %ymm1
	vpaddq	%ymm7, %ymm0, %ymm0
	vpsrlq	$32, %ymm1, %ymm1
	vpsrlq	$32, %ymm0, %ymm0
	vpsllq	$32, %ymm1, %ymm1
	vpor	%ymm1, %ymm0, %ymm0
	vpaddd	%ymm2, %ymm0, %ymm1
	vpminud	%ymm1, %ymm0, %ymm0
	vpbroadcastd	16(%rbp), %ymm1
	vpaddd	%ymm1, %ymm0, %ymm0
	vpaddd	%ymm2, %ymm0, %ymm2
	vpminud	%ymm2, %ymm0, %ymm0
	vpblendvb	%ymm5, %ymm0, %ymm6, %ymm6
	vmovdqa	%ymm6, (%rdx)
.L187:
	vzeroupper
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8448:
	.size	_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj, .-_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj
	.section	.text._ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj,"axG",@progbits,_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj,comdat
	.align 2
	.p2align 4
	.weak	_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj
	.type	_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj, @function
_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj:
.LFB8436:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	movl	%ecx, %r10d
	movslq	%esi, %rcx
	movq	%rdx, %rax
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	salq	$4, %rax
	movq	%rdi, %r14
	movl	%r8d, %r15d
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$24, %rsp
	.cfi_def_cfa_offset 80
	movl	184(%rdi,%rcx,4), %edx
	addq	88(%rdi,%rcx,8), %rax
	movl	80(%rsp), %r12d
	movl	%r12d, %ecx
	imulq	%rcx, %rdx
	imull	$998244351, %edx, %ebx
	imulq	$998244353, %rbx, %rbx
	addq	%rdx, %rbx
	shrq	$32, %rbx
	leal	-998244353(%rbx), %edx
	cmpq	$998244352, %rbx
	cmova	%edx, %ebx
	testl	%esi, %esi
	je	.L206
	movq	24(%rdi), %r13
	movq	%rax, %rbp
	subq	96(%rdi), %rbp
	salq	$2, %rbp
	addq	%rbp, %r13
	addq	48(%rdi), %rbp
	cmpb	$0, 76(%r14)
	jne	.L192
.L216:
	cmpl	%r15d, %r10d
	jnb	.L189
	subl	$1, %r15d
	movl	%r10d, %edx
	movq	(%r14), %rdi
	movl	%r9d, %r9d
	subl	%r10d, %r15d
	leaq	0(,%rdx,4), %rcx
	leaq	1(%rdx,%r15), %rdx
	leaq	(%rdi,%rax,4), %rdi
	salq	$2, %rdx
	.p2align 4,,10
	.p2align 3
.L205:
	movl	(%rdi,%rcx), %r8d
	imulq	%r9, %r8
	imull	$998244351, %r8d, %eax
	imulq	$998244353, %rax, %rax
	addq	%r8, %rax
	shrq	$32, %rax
	cmpq	$998244352, %rax
	leal	-998244353(%rax), %r8d
	cmova	%r8d, %eax
	addl	%ebx, %eax
	leal	-998244353(%rax), %r8d
	cmpl	$998244352, %eax
	cmova	%r8d, %eax
	movl	%eax, (%rdi,%rcx)
	testl	%esi, %esi
	je	.L201
	movl	0(%r13,%rcx), %r8d
	imulq	%r9, %r8
	imull	$998244351, %r8d, %eax
	imulq	$998244353, %rax, %rax
	addq	%r8, %rax
	shrq	$32, %rax
	cmpq	$998244352, %rax
	leal	-998244353(%rax), %r8d
	cmova	%r8d, %eax
	movl	%eax, 0(%r13,%rcx)
	movl	0(%rbp,%rcx), %r8d
	imulq	%r9, %r8
	imull	$998244351, %r8d, %eax
	imulq	$998244353, %rax, %rax
	addq	%r8, %rax
	shrq	$32, %rax
	cmpq	$998244352, %rax
	leal	-998244353(%rax), %r8d
	cmova	%r8d, %eax
	addl	%r12d, %eax
	leal	-998244353(%rax), %r8d
	cmpl	$998244352, %eax
	cmova	%r8d, %eax
	movl	%eax, 0(%rbp,%rcx)
.L201:
	addq	$4, %rcx
	cmpq	%rcx, %rdx
	jne	.L205
.L189:
	addq	$24, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L206:
	.cfi_restore_state
	xorl	%r13d, %r13d
	xorl	%ebp, %ebp
	cmpb	$0, 76(%r14)
	je	.L216
.L192:
	cmpl	$7, %r10d
	jbe	.L217
.L195:
	cmpl	$8, %r15d
	jbe	.L189
	movl	$8, %edx
	leal	-8(%r15), %r8d
	cmpl	%edx, %r10d
	cmovnb	%r10d, %edx
	testq	%rbp, %rbp
	leal	-8(%rdx), %ecx
	leaq	32(%rbp), %rdx
	cmovne	%rdx, %rbp
	testq	%r13, %r13
	leaq	32(%r13), %rdx
	cmovne	%rdx, %r13
	movq	(%r14), %rdx
	pushq	%rbx
	.cfi_def_cfa_offset 88
	pushq	%r12
	.cfi_def_cfa_offset 96
	leaq	32(%rdx,%rax,4), %rdi
	movq	%r13, %rsi
	movq	%rbp, %rdx
	call	_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj
	popq	%rax
	.cfi_def_cfa_offset 88
	popq	%rdx
	.cfi_def_cfa_offset 80
	addq	$24, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L217:
	.cfi_restore_state
	movl	$8, %r8d
	movq	(%r14), %rdx
	movl	%r10d, %ecx
	movq	%r13, %rsi
	movq	%rax, 8(%rsp)
	cmpl	%r8d, %r15d
	pushq	%rbx
	.cfi_def_cfa_offset 88
	cmovbe	%r15d, %r8d
	leaq	(%rdx,%rax,4), %rdi
	movq	%rbp, %rdx
	pushq	%r12
	.cfi_def_cfa_offset 96
	movl	%r9d, 20(%rsp)
	movl	%r10d, 16(%rsp)
	call	_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj
	popq	%rcx
	.cfi_def_cfa_offset 88
	popq	%rsi
	.cfi_def_cfa_offset 80
	movq	8(%rsp), %rax
	movl	4(%rsp), %r9d
	movl	(%rsp), %r10d
	jmp	.L195
	.cfi_endproc
.LFE8436:
	.size	_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj, .-_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj
	.section	.text._ZN9blueberry10extra_fast18ShiftAffineControl12update_childEimjmmjj,"axG",@progbits,_ZN9blueberry10extra_fast18ShiftAffineControl12update_childEimjmmjj,comdat
	.align 2
	.p2align 4
	.weak	_ZN9blueberry10extra_fast18ShiftAffineControl12update_childEimjmmjj
	.type	_ZN9blueberry10extra_fast18ShiftAffineControl12update_childEimjmmjj, @function
_ZN9blueberry10extra_fast18ShiftAffineControl12update_childEimjmmjj:
.LFB8438:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	movslq	%esi, %rax
	salq	$4, %rdx
	movl	%ecx, %ecx
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	leaq	(%rdi,%rax,8), %r14
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	movq	%rdi, %rbp
	leaq	(%rdx,%rcx), %rdi
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movq	%rax, %rbx
	subl	$1, %ebx
	subq	$72, %rsp
	.cfi_def_cfa_offset 128
	movq	88(%r14), %r13
	movq	24(%rbp), %r15
	movq	%r8, 8(%rsp)
	movq	96(%rbp), %r8
	addq	%rdi, %r13
	movq	48(%rbp), %rdx
	movq	%r9, 16(%rsp)
	subq	%r8, %r13
	movq	%rdi, 24(%rsp)
	salq	$4, %rdi
	salq	$2, %r13
	movq	%rdi, 32(%rsp)
	leaq	(%r15,%r13), %r10
	leaq	(%rdx,%r13), %r11
	movl	(%r10), %r9d
	movl	(%r11), %eax
	cmpl	$301989884, %r9d
	jne	.L247
	testl	%eax, %eax
	jne	.L247
.L219:
	movq	8(%rsp), %rax
	movl	$1, %r12d
	movq	16(%rsp), %r15
	leal	0(,%rbx,4), %r13d
	movl	%r13d, %ecx
	salq	%cl, %r12
	shrq	%cl, %r15
	leaq	-1(%r12,%rax), %rdx
	movl	%r15d, 40(%rsp)
	shrq	%cl, %rdx
	cmpl	%r15d, %edx
	jb	.L269
.L234:
	testl	%ebx, %ebx
	je	.L236
	movq	16(%rsp), %r10
	leaq	-1(%r12), %r8
	andq	%r8, %r10
	andq	8(%rsp), %r8
	je	.L270
	movq	8(%rsp), %r11
	movl	%r13d, %ecx
	movq	16(%rsp), %rax
	shrq	%cl, %r11
	leal	1(%r11), %edx
	movl	%r11d, %esi
	salq	%cl, %rdx
	cmpq	%rdx, %rax
	jb	.L271
.L239:
	movl	136(%rsp), %eax
	movl	%esi, %ecx
	movq	%r10, 8(%rsp)
	movl	%ebx, %esi
	movq	%r11, 16(%rsp)
	movq	%r12, %r9
	movq	%rbp, %rdi
	pushq	%rax
	.cfi_def_cfa_offset 136
	movl	136(%rsp), %eax
	pushq	%rax
	.cfi_def_cfa_offset 144
	movq	40(%rsp), %rdx
	call	_ZN9blueberry10extra_fast18ShiftAffineControl12update_childEimjmmjj
	popq	%rcx
	.cfi_def_cfa_offset 136
	popq	%rsi
	.cfi_def_cfa_offset 128
	movq	8(%rsp), %r10
	testq	%r10, %r10
	je	.L236
	movq	16(%rsp), %r11
	cmpl	%r11d, %r15d
	jne	.L240
	.p2align 4,,10
	.p2align 3
.L236:
	movq	32(%rsp), %rax
	addq	80(%r14), %rax
	cmpb	$0, 76(%rbp)
	movq	0(%rbp), %r12
	jne	.L272
	leaq	(%r12,%rax,4), %rax
	pxor	%xmm1, %xmm1
	pxor	%xmm2, %xmm2
	leaq	64(%rax), %rdx
.L244:
	movdqu	(%rax), %xmm0
	addq	$16, %rax
	movdqa	%xmm0, %xmm3
	punpckhdq	%xmm2, %xmm0
	punpckldq	%xmm2, %xmm3
	paddq	%xmm3, %xmm0
	paddq	%xmm0, %xmm1
	cmpq	%rax, %rdx
	jne	.L244
	movabsq	$-8525806094425994177, %rax
	movdqa	%xmm1, %xmm0
	psrldq	$8, %xmm0
	paddq	%xmm0, %xmm1
	movq	%xmm1, %rcx
	mulq	%rcx
	movl	%ecx, %eax
	shrq	$29, %rdx
	imulq	$998244353, %rdx, %rdx
	subl	%edx, %eax
.L243:
	movq	24(%rsp), %rdx
	addq	88(%r14), %rdx
	movl	%eax, (%r12,%rdx,4)
	addq	$72, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L270:
	.cfi_restore_state
	testq	%r10, %r10
	je	.L236
.L240:
	movl	136(%rsp), %eax
	xorl	%r8d, %r8d
	movq	%rbp, %rdi
	movq	%r10, %r9
	movl	%ebx, %esi
	pushq	%rax
	.cfi_def_cfa_offset 136
	movl	136(%rsp), %eax
	pushq	%rax
	.cfi_def_cfa_offset 144
	movl	56(%rsp), %ecx
	movq	40(%rsp), %rdx
	call	_ZN9blueberry10extra_fast18ShiftAffineControl12update_childEimjmmjj
	popq	%rdi
	.cfi_def_cfa_offset 136
	popq	%r8
	.cfi_def_cfa_offset 128
	jmp	.L236
	.p2align 4,,10
	.p2align 3
.L247:
	movslq	%ebx, %rsi
	movl	%eax, %edi
	movq	32(%rsp), %rcx
	addq	80(%r14), %rcx
	movl	184(%rbp,%rsi,4), %esi
	imulq	%rdi, %rsi
	leaq	0(,%rcx,4), %rdi
	movq	%rdi, 40(%rsp)
	imull	$998244351, %esi, %r12d
	imulq	$998244353, %r12, %r12
	addq	%rsi, %r12
	shrq	$32, %r12
	leal	-998244353(%r12), %esi
	cmpq	$998244352, %r12
	cmova	%esi, %r12d
	movzbl	76(%rbp), %esi
	addq	0(%rbp), %rdi
	testl	%ebx, %ebx
	je	.L222
	subq	%r8, %rcx
	salq	$2, %rcx
	addq	%rcx, %r15
	addq	%rcx, %rdx
	testb	%sil, %sil
	jne	.L223
.L246:
	xorl	%esi, %esi
	.p2align 4,,10
	.p2align 3
.L224:
	movl	(%rdi,%rsi), %r8d
	imulq	%r9, %r8
	imull	$998244351, %r8d, %ecx
	imulq	$998244353, %rcx, %rcx
	addq	%r8, %rcx
	shrq	$32, %rcx
	cmpq	$998244352, %rcx
	leal	-998244353(%rcx), %r8d
	cmova	%r8d, %ecx
	addl	%r12d, %ecx
	leal	-998244353(%rcx), %r8d
	cmpl	$998244352, %ecx
	cmova	%r8d, %ecx
	movl	%ecx, (%rdi,%rsi)
	testl	%ebx, %ebx
	je	.L230
	movl	(%r15,%rsi), %r8d
	imulq	%r9, %r8
	imull	$998244351, %r8d, %ecx
	imulq	$998244353, %rcx, %rcx
	addq	%r8, %rcx
	shrq	$32, %rcx
	cmpq	$998244352, %rcx
	leal	-998244353(%rcx), %r8d
	cmova	%r8d, %ecx
	movl	%ecx, (%r15,%rsi)
	movl	(%rdx,%rsi), %r8d
	imulq	%r9, %r8
	imull	$998244351, %r8d, %ecx
	imulq	$998244353, %rcx, %rcx
	addq	%r8, %rcx
	shrq	$32, %rcx
	cmpq	$998244352, %rcx
	leal	-998244353(%rcx), %r8d
	cmova	%r8d, %ecx
	addl	%eax, %ecx
	leal	-998244353(%rcx), %r8d
	cmpl	$998244352, %ecx
	cmova	%r8d, %ecx
	movl	%ecx, (%rdx,%rsi)
.L230:
	addq	$4, %rsi
	cmpq	$64, %rsi
	jne	.L224
	movl	$301989884, (%r10)
	movl	$0, (%r11)
	jmp	.L219
	.p2align 4,,10
	.p2align 3
.L272:
	leaq	0(,%rax,4), %rbp
	xorl	%esi, %esi
	movl	$8, %edx
	leaq	(%r12,%rbp), %rdi
	call	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
	movl	$8, %edx
	xorl	%esi, %esi
	leaq	32(%r12,%rbp), %rdi
	movl	%eax, %ebx
	call	_ZN9blueberry10extra_fast18ShiftAffineControl9fold_simdEPKjjj
	addl	%ebx, %eax
	leal	-998244353(%rax), %edx
	cmpl	$998244352, %eax
	cmova	%edx, %eax
	jmp	.L243
	.p2align 4,,10
	.p2align 3
.L269:
	subq	$8, %rsp
	.cfi_def_cfa_offset 136
	movl	%edx, %ecx
	movl	%r15d, %r8d
	movl	%ebx, %esi
	movl	144(%rsp), %eax
	movq	%rbp, %rdi
	pushq	%rax
	.cfi_def_cfa_offset 144
	movl	144(%rsp), %r9d
	movq	40(%rsp), %rdx
	call	_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj
	popq	%r9
	.cfi_def_cfa_offset 136
	popq	%r10
	.cfi_def_cfa_offset 128
	jmp	.L234
	.p2align 4,,10
	.p2align 3
.L222:
	testb	%sil, %sil
	jne	.L245
	xorl	%edx, %edx
	xorl	%r15d, %r15d
	jmp	.L246
	.p2align 4,,10
	.p2align 3
.L271:
	movl	%r11d, %edx
	salq	%cl, %rdx
	subq	%rdx, %rax
	movq	%rax, %r12
	jmp	.L239
	.p2align 4,,10
	.p2align 3
.L223:
	pushq	%r12
	.cfi_def_cfa_offset 136
	xorl	%ecx, %ecx
	movl	$8, %r8d
	movq	%r15, %rsi
	pushq	%rax
	.cfi_def_cfa_offset 144
	movl	%r9d, 72(%rsp)
	movl	%eax, 76(%rsp)
	movq	%rdx, 64(%rsp)
	call	_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj
	popq	%rax
	.cfi_def_cfa_offset 136
	popq	%rdx
	.cfi_def_cfa_offset 128
	movq	48(%rsp), %rdx
	movl	56(%rsp), %r9d
	movl	60(%rsp), %eax
	testq	%rdx, %rdx
	je	.L225
	addq	$32, %rdx
.L225:
	leaq	32(%r15), %rcx
	testq	%r15, %r15
	cmovne	%rcx, %r15
.L226:
	movq	40(%rsp), %rsi
	movq	0(%rbp), %rcx
	pushq	%r12
	.cfi_def_cfa_offset 136
	movl	$8, %r8d
	pushq	%rax
	.cfi_def_cfa_offset 144
	leaq	32(%rcx,%rsi), %rdi
	xorl	%ecx, %ecx
	movq	%r15, %rsi
	call	_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj
	movq	24(%rbp), %r10
	movq	48(%rbp), %r11
	popq	%r12
	.cfi_def_cfa_offset 136
	addq	%r13, %r10
	addq	%r13, %r11
	popq	%r13
	.cfi_def_cfa_offset 128
	movl	$301989884, (%r10)
	movl	$0, (%r11)
	jmp	.L219
.L245:
	pushq	%r12
	.cfi_def_cfa_offset 136
	xorl	%edx, %edx
	movl	$8, %r8d
	xorl	%ecx, %ecx
	pushq	%rax
	.cfi_def_cfa_offset 144
	xorl	%esi, %esi
	xorl	%r15d, %r15d
	movl	%r9d, 64(%rsp)
	movl	%eax, 72(%rsp)
	call	_ZN9blueberry10extra_fast18ShiftAffineControl14transform_simdEPjS2_S2_jjjjj
	popq	%rax
	.cfi_def_cfa_offset 136
	popq	%rdx
	.cfi_def_cfa_offset 128
	xorl	%edx, %edx
	movl	48(%rsp), %r9d
	movl	56(%rsp), %eax
	jmp	.L226
	.cfi_endproc
.LFE8438:
	.size	_ZN9blueberry10extra_fast18ShiftAffineControl12update_childEimjmmjj, .-_ZN9blueberry10extra_fast18ShiftAffineControl12update_childEimjmmjj
	.text
	.p2align 4
	.globl	baseline_prod
	.type	baseline_prod, @function
baseline_prod:
.LFB8449:
	.cfi_startproc
	endbr64
	cmpl	%edx, %esi
	jne	.L275
	xorl	%eax, %eax
	ret
	.p2align 4,,10
	.p2align 3
.L275:
	movl	80(%rdi), %eax
	movslq	%esi, %rcx
	movslq	%edx, %r8
	xorl	%edx, %edx
	leal	-1(%rax), %esi
	jmp	_ZNK9blueberry10extra_fast22AffineSumSegmentTree165queryEimmm
	.cfi_endproc
.LFE8449:
	.size	baseline_prod, .-baseline_prod
	.p2align 4
	.globl	shift_prod
	.type	shift_prod, @function
shift_prod:
.LFB8450:
	.cfi_startproc
	endbr64
	cmpl	%edx, %esi
	jne	.L278
	xorl	%eax, %eax
	ret
	.p2align 4,,10
	.p2align 3
.L278:
	movl	80(%rdi), %eax
	movslq	%esi, %rcx
	movslq	%edx, %r8
	xorl	%edx, %edx
	leal	-1(%rax), %esi
	jmp	_ZNK9blueberry10extra_fast18ShiftAffineControl5queryEimmm
	.cfi_endproc
.LFE8450:
	.size	shift_prod, .-shift_prod
	.p2align 4
	.globl	baseline_apply
	.type	baseline_apply, @function
baseline_apply:
.LFB8451:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	movslq	%edx, %r13
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	movslq	%esi, %r12
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$40, %rsp
	.cfi_def_cfa_offset 96
	cmpl	%r13d, %r12d
	je	.L279
	movabsq	$-8525806094425994177, %rsi
	movl	%ecx, %ebx
	movl	%r8d, %ebp
	salq	$32, %rbx
	salq	$32, %rbp
	movq	%rbx, %rax
	mulq	%rsi
	movq	%rbp, %rax
	shrq	$29, %rdx
	imulq	$998244353, %rdx, %rdx
	subq	%rdx, %rbx
	mulq	%rsi
	shrq	$29, %rdx
	imulq	$998244353, %rdx, %rdx
	subq	%rdx, %rbp
	cmpl	$301989884, %ebx
	jne	.L288
	testl	%ebp, %ebp
	jne	.L288
.L279:
	addq	$40, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L288:
	.cfi_restore_state
	movl	80(%rdi), %eax
	xorl	%edx, %edx
	leal	-1(%rax), %r15d
	movslq	%r15d, %rax
	movq	184(%rdi,%rax,8), %r14
	leaq	-1(%r12,%r14), %rax
	divq	%r14
	xorl	%edx, %edx
	movq	%rax, %r8
	movl	%eax, %ecx
	movq	%r13, %rax
	divq	%r14
	movl	%eax, 4(%rsp)
	movq	%rdx, %r11
	movq	%rax, %r10
	cmpl	%eax, %r8d
	jb	.L302
.L284:
	testl	%r15d, %r15d
	je	.L279
	movq	%r12, %rax
	xorl	%edx, %edx
	divq	%r14
	movq	%rax, %r12
	testq	%rdx, %rdx
	jne	.L285
	testq	%r11, %r11
	je	.L279
.L286:
	pushq	%rbp
	.cfi_def_cfa_offset 104
	xorl	%edx, %edx
	movq	%r11, %r9
	xorl	%r8d, %r8d
	pushq	%rbx
	.cfi_def_cfa_offset 112
	movl	20(%rsp), %ecx
	movl	%r15d, %esi
	call	_ZN9blueberry10extra_fast22AffineSumSegmentTree1612update_childEimjmmjj
	popq	%rax
	.cfi_def_cfa_offset 104
	popq	%rdx
	.cfi_def_cfa_offset 96
	jmp	.L279
	.p2align 4,,10
	.p2align 3
.L302:
	movq	%rdx, 24(%rsp)
	subq	$8, %rsp
	.cfi_def_cfa_offset 104
	movl	%eax, %r8d
	movl	%ebx, %r9d
	pushq	%rbp
	.cfi_def_cfa_offset 112
	xorl	%edx, %edx
	movl	%r15d, %esi
	movq	%rax, 32(%rsp)
	movq	%rdi, 24(%rsp)
	call	_ZN9blueberry10extra_fast22AffineSumSegmentTree169transformEimjjjj
	popq	%rdi
	.cfi_def_cfa_offset 104
	popq	%r8
	.cfi_def_cfa_offset 96
	movq	24(%rsp), %r11
	movq	16(%rsp), %r10
	movq	8(%rsp), %rdi
	jmp	.L284
	.p2align 4,,10
	.p2align 3
.L285:
	movl	%eax, %ecx
	leal	1(%rax), %eax
	imulq	%r14, %rax
	cmpq	%rax, %r13
	jb	.L303
.L287:
	movq	%r11, 8(%rsp)
	movq	%rdx, %r8
	movl	%r15d, %esi
	movq	%r14, %r9
	movq	%r10, 24(%rsp)
	xorl	%edx, %edx
	pushq	%rbp
	.cfi_def_cfa_offset 104
	pushq	%rbx
	.cfi_def_cfa_offset 112
	movq	%rdi, 32(%rsp)
	call	_ZN9blueberry10extra_fast22AffineSumSegmentTree1612update_childEimjmmjj
	popq	%rcx
	.cfi_def_cfa_offset 104
	popq	%rsi
	.cfi_def_cfa_offset 96
	movq	8(%rsp), %r11
	testq	%r11, %r11
	je	.L279
	movq	24(%rsp), %r10
	movq	16(%rsp), %rdi
	cmpl	%r12d, %r10d
	je	.L279
	jmp	.L286
	.p2align 4,,10
	.p2align 3
.L303:
	movl	%r12d, %eax
	imulq	%r14, %rax
	movq	%r13, %r14
	subq	%rax, %r14
	jmp	.L287
	.cfi_endproc
.LFE8451:
	.size	baseline_apply, .-baseline_apply
	.p2align 4
	.globl	shift_apply
	.type	shift_apply, @function
shift_apply:
.LFB8452:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	movslq	%esi, %r12
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$40, %rsp
	.cfi_def_cfa_offset 96
	cmpl	%edx, %r12d
	je	.L304
	movl	%ecx, %ebx
	movq	%rdi, %r13
	movl	%edx, %edi
	movl	%r8d, %ebp
	salq	$32, %rbx
	salq	$32, %rbp
	movabsq	$-8525806094425994177, %rsi
	movq	%rbx, %rax
	mulq	%rsi
	movq	%rbp, %rax
	shrq	$29, %rdx
	imulq	$998244353, %rdx, %rdx
	subq	%rdx, %rbx
	mulq	%rsi
	shrq	$29, %rdx
	imulq	$998244353, %rdx, %rdx
	subq	%rdx, %rbp
	cmpl	$301989884, %ebx
	jne	.L314
	testl	%ebp, %ebp
	jne	.L314
.L304:
	addq	$40, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L314:
	.cfi_restore_state
	movl	80(%r13), %eax
	movl	$1, %r14d
	movslq	%edi, %r11
	leal	-1(%rax), %esi
	movq	%r11, %rax
	leal	0(,%rsi,4), %r15d
	movl	%r15d, %ecx
	salq	%cl, %r14
	shrq	%cl, %rax
	leaq	-1(%r12,%r14), %rdx
	movl	%eax, 4(%rsp)
	shrq	%cl, %rdx
	cmpl	%eax, %edx
	jb	.L328
.L309:
	testl	%esi, %esi
	je	.L304
	leaq	-1(%r14), %r8
	movq	%r11, %r10
	andq	%r8, %r10
	andq	%r12, %r8
	jne	.L310
	testq	%r10, %r10
	je	.L304
.L312:
	pushq	%rbp
	.cfi_def_cfa_offset 104
	movq	%r10, %r9
	xorl	%r8d, %r8d
	xorl	%edx, %edx
	pushq	%rbx
	.cfi_def_cfa_offset 112
	movl	20(%rsp), %ecx
	movq	%r13, %rdi
	call	_ZN9blueberry10extra_fast18ShiftAffineControl12update_childEimjmmjj
	popq	%rcx
	.cfi_def_cfa_offset 104
	popq	%rsi
	.cfi_def_cfa_offset 96
	jmp	.L304
	.p2align 4,,10
	.p2align 3
.L328:
	movq	%r11, 24(%rsp)
	subq	$8, %rsp
	.cfi_def_cfa_offset 104
	movl	%eax, %r8d
	movl	%edx, %ecx
	pushq	%rbp
	.cfi_def_cfa_offset 112
	movq	%r13, %rdi
	movl	%ebx, %r9d
	xorl	%edx, %edx
	movq	%rax, 32(%rsp)
	movl	%esi, 24(%rsp)
	call	_ZN9blueberry10extra_fast18ShiftAffineControl9transformEimjjjj
	popq	%rdi
	.cfi_def_cfa_offset 104
	popq	%r8
	.cfi_def_cfa_offset 96
	movq	24(%rsp), %r11
	movq	16(%rsp), %rax
	movl	8(%rsp), %esi
	jmp	.L309
	.p2align 4,,10
	.p2align 3
.L310:
	movl	%r15d, %ecx
	shrq	%cl, %r12
	leal	1(%r12), %edi
	movl	%r12d, %edx
	salq	%cl, %rdi
	cmpq	%rdi, %r11
	jb	.L329
.L311:
	movq	%r10, 8(%rsp)
	movl	%edx, %ecx
	movq	%r14, %r9
	xorl	%edx, %edx
	movq	%rax, 24(%rsp)
	movq	%r13, %rdi
	pushq	%rbp
	.cfi_def_cfa_offset 104
	pushq	%rbx
	.cfi_def_cfa_offset 112
	movl	%esi, 32(%rsp)
	call	_ZN9blueberry10extra_fast18ShiftAffineControl12update_childEimjmmjj
	popq	%rax
	.cfi_def_cfa_offset 104
	popq	%rdx
	.cfi_def_cfa_offset 96
	movq	8(%rsp), %r10
	testq	%r10, %r10
	je	.L304
	movq	24(%rsp), %rax
	movl	16(%rsp), %esi
	cmpl	%r12d, %eax
	je	.L304
	jmp	.L312
	.p2align 4,,10
	.p2align 3
.L329:
	movl	%r12d, %edi
	salq	%cl, %rdi
	subq	%rdi, %r11
	movq	%r11, %r14
	jmp	.L311
	.cfi_endproc
.LFE8452:
	.size	shift_apply, .-shift_apply
	.section	.rodata.cst32,"aM",@progbits,32
	.align 32
.LC0:
	.long	0
	.long	1
	.long	2
	.long	3
	.long	4
	.long	5
	.long	6
	.long	7
	.ident	"GCC: (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
