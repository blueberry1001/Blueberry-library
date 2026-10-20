	.file	"direct.cpp.snapshot"
	.text
	.section	.text._ZN9blueberry10extra_fast14StaticRangeMinIjE14short_min_simdEPKji,"axG",@progbits,_ZN9blueberry10extra_fast14StaticRangeMinIjE14short_min_simdEPKji,comdat
	.p2align 4
	.weak	_ZN9blueberry10extra_fast14StaticRangeMinIjE14short_min_simdEPKji
	.type	_ZN9blueberry10extra_fast14StaticRangeMinIjE14short_min_simdEPKji, @function
_ZN9blueberry10extra_fast14StaticRangeMinIjE14short_min_simdEPKji:
.LFB8790:
	.cfi_startproc
	endbr64
	movslq	%esi, %rsi
	vmovdqu	(%rdi), %ymm2
	vpminud	-32(%rdi,%rsi,4), %ymm2, %ymm0
	vperm2i128	$1, %ymm0, %ymm0, %ymm1
	vpminud	%ymm1, %ymm0, %ymm0
	vpshufd	$78, %ymm0, %ymm1
	vpminud	%ymm1, %ymm0, %ymm0
	vpshufd	$177, %ymm0, %ymm1
	vpminud	%ymm1, %ymm0, %ymm0
	vmovd	%xmm0, %eax
	vzeroupper
	ret
	.cfi_endproc
.LFE8790:
	.size	_ZN9blueberry10extra_fast14StaticRangeMinIjE14short_min_simdEPKji, .-_ZN9blueberry10extra_fast14StaticRangeMinIjE14short_min_simdEPKji
	.section	.text._ZNK9blueberry10extra_fast14StaticRangeMinIjE4prodEii,"axG",@progbits,_ZNK9blueberry10extra_fast14StaticRangeMinIjE4prodEii,comdat
	.align 2
	.p2align 4
	.weak	_ZNK9blueberry10extra_fast14StaticRangeMinIjE4prodEii
	.type	_ZNK9blueberry10extra_fast14StaticRangeMinIjE4prodEii, @function
_ZNK9blueberry10extra_fast14StaticRangeMinIjE4prodEii:
.LFB8787:
	.cfi_startproc
	endbr64
	movq	%rdi, %r8
	cmpl	%edx, %esi
	je	.L10
	leal	-1(%rdx), %eax
	movl	%esi, %r9d
	movslq	%esi, %rcx
	movl	%eax, %r10d
	sarl	$4, %r9d
	leaq	0(,%rcx,4), %rdi
	sarl	$4, %r10d
	cmpl	%r10d, %r9d
	je	.L22
	movq	24(%r8), %rsi
	movq	48(%r8), %rdx
	cltq
	movl	(%rdx,%rcx,4), %edx
	movl	(%rsi,%rax,4), %eax
	movl	%r10d, %ecx
	cmpl	%edx, %eax
	cmova	%edx, %eax
	subl	%r9d, %ecx
	subl	$1, %ecx
	je	.L3
	bsrl	%ecx, %ecx
	movq	72(%r8), %rsi
	movslq	%ecx, %rdx
	movq	96(%r8,%rdx,8), %rdi
	movl	$1, %edx
	salq	%cl, %rdx
	movq	%rdx, %rcx
	movq	%rdi, %rdx
	subq	%rcx, %rdx
	movslq	%r10d, %rcx
	addq	%rdx, %rcx
	leal	1(%r9), %edx
	movslq	%edx, %rdx
	movl	(%rsi,%rcx,4), %ecx
	addq	%rdi, %rdx
	movl	(%rsi,%rdx,4), %edx
	cmpl	%edx, %ecx
	cmovbe	%ecx, %edx
	cmpl	%edx, %eax
	cmova	%edx, %eax
	ret
	.p2align 4,,10
	.p2align 3
.L10:
	movl	$-1, %eax
.L3:
	ret
	.p2align 4,,10
	.p2align 3
.L22:
	movq	(%r8), %rcx
	addq	%rcx, %rdi
	cmpb	$0, 356(%r8)
	je	.L6
	movl	%edx, %eax
	subl	%esi, %eax
	cmpl	$7, %eax
	jle	.L6
	movl	%eax, %esi
	jmp	_ZN9blueberry10extra_fast14StaticRangeMinIjE14short_min_simdEPKji
	.p2align 4,,10
	.p2align 3
.L6:
	addl	$1, %esi
	movl	(%rdi), %eax
	cmpl	%esi, %edx
	jle	.L3
	movslq	%esi, %rsi
	movq	%rsi, %rdi
	movl	(%rcx,%rsi,4), %r8d
	notq	%rdi
	addl	%edx, %edi
	andl	$1, %edi
	cmpl	%r8d, %eax
	cmova	%r8d, %eax
	addq	$1, %rsi
	cmpl	%esi, %edx
	jle	.L23
	testl	%edi, %edi
	je	.L8
	movl	(%rcx,%rsi,4), %edi
	cmpl	%edi, %eax
	cmova	%edi, %eax
	addq	$1, %rsi
	cmpl	%esi, %edx
	jle	.L24
	.p2align 4,,10
	.p2align 3
.L8:
	movl	(%rcx,%rsi,4), %edi
	cmpl	%edi, %eax
	cmova	%edi, %eax
	movl	4(%rcx,%rsi,4), %edi
	cmpl	%edi, %eax
	cmova	%edi, %eax
	addq	$2, %rsi
	cmpl	%esi, %edx
	jg	.L8
	ret
	.p2align 4,,10
	.p2align 3
.L23:
	ret
.L24:
	ret
	.cfi_endproc
.LFE8787:
	.size	_ZNK9blueberry10extra_fast14StaticRangeMinIjE4prodEii, .-_ZNK9blueberry10extra_fast14StaticRangeMinIjE4prodEii
	.text
	.p2align 4
	.globl	_Z10query_manyRKN9blueberry10extra_fast14StaticRangeMinIjEEPKSt4pairIiiEm
	.type	_Z10query_manyRKN9blueberry10extra_fast14StaticRangeMinIjEEPKSt4pairIiiEm, @function
_Z10query_manyRKN9blueberry10extra_fast14StaticRangeMinIjEEPKSt4pairIiiEm:
.LFB8775:
	.cfi_startproc
	endbr64
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	pushq	%r12
	.cfi_def_cfa_offset 24
	.cfi_offset 12, -24
	pushq	%rbp
	.cfi_def_cfa_offset 32
	.cfi_offset 6, -32
	pushq	%rbx
	.cfi_def_cfa_offset 40
	.cfi_offset 3, -40
	subq	$8, %rsp
	.cfi_def_cfa_offset 48
	testq	%rdx, %rdx
	je	.L28
	movq	%rdi, %rbp
	movq	%rsi, %rbx
	leaq	(%rsi,%rdx,8), %r13
	xorl	%r12d, %r12d
	.p2align 4,,10
	.p2align 3
.L27:
	movl	4(%rbx), %edx
	movl	(%rbx), %esi
	movq	%rbp, %rdi
	addq	$8, %rbx
	call	_ZNK9blueberry10extra_fast14StaticRangeMinIjE4prodEii
	movl	%eax, %eax
	addq	%rax, %r12
	cmpq	%rbx, %r13
	jne	.L27
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 40
	movq	%r12, %rax
	popq	%rbx
	.cfi_def_cfa_offset 32
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L28:
	.cfi_restore_state
	addq	$8, %rsp
	.cfi_def_cfa_offset 40
	xorl	%r12d, %r12d
	popq	%rbx
	.cfi_def_cfa_offset 32
	movq	%r12, %rax
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE8775:
	.size	_Z10query_manyRKN9blueberry10extra_fast14StaticRangeMinIjEEPKSt4pairIiiEm, .-_Z10query_manyRKN9blueberry10extra_fast14StaticRangeMinIjEEPKSt4pairIiiEm
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
