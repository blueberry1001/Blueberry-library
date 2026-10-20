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
	.text
	.p2align 4
	.globl	_Z10query_manyRKN9blueberry10extra_fast14StaticRangeMinIjEEPKSt4pairIiiEm
	.type	_Z10query_manyRKN9blueberry10extra_fast14StaticRangeMinIjEEPKSt4pairIiiEm, @function
_Z10query_manyRKN9blueberry10extra_fast14StaticRangeMinIjEEPKSt4pairIiiEm:
.LFB8775:
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
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$8, %rsp
	.cfi_def_cfa_offset 64
	testq	%rdx, %rdx
	je	.L13
	movq	%rdi, %r14
	movq	%rsi, %r12
	leaq	(%rsi,%rdx,8), %rbx
	xorl	%r13d, %r13d
	movl	$4294967295, %r15d
	movl	$31, %ebp
	.p2align 4,,10
	.p2align 3
.L12:
	movl	4(%r12), %ecx
	movl	(%r12), %edx
	movq	%r15, %rax
	cmpl	%edx, %ecx
	je	.L5
	leal	-1(%rcx), %eax
	movl	%edx, %esi
	movslq	%edx, %r10
	movl	%eax, %r9d
	sarl	$4, %esi
	leaq	0(,%r10,4), %rdi
	sarl	$4, %r9d
	cmpl	%r9d, %esi
	jne	.L6
	movq	(%r14), %rsi
	addq	%rsi, %rdi
	cmpb	$0, 356(%r14)
	je	.L7
	movl	%ecx, %eax
	subl	%edx, %eax
	cmpl	$7, %eax
	jle	.L7
	movl	%eax, %esi
	call	_ZN9blueberry10extra_fast14StaticRangeMinIjE14short_min_simdEPKji
.L8:
	movl	%eax, %eax
.L5:
	addq	$8, %r12
	addq	%rax, %r13
	cmpq	%rbx, %r12
	jne	.L12
.L3:
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movq	%r13, %rax
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
.L6:
	.cfi_restore_state
	movq	24(%r14), %rcx
	movq	48(%r14), %rdx
	cltq
	movl	(%rdx,%r10,4), %edx
	movl	(%rcx,%rax,4), %eax
	cmpl	%edx, %eax
	cmova	%rdx, %rax
	movl	%r9d, %edx
	subl	%esi, %edx
	subl	$1, %edx
	je	.L5
	bsrl	%edx, %edx
	movl	%ebp, %ecx
	movq	72(%r14), %rdi
	movslq	%r9d, %r9
	xorl	$31, %edx
	subl	%edx, %ecx
	movslq	%ecx, %rdx
	movq	96(%r14,%rdx,8), %r10
	movl	$1, %edx
	salq	%cl, %rdx
	movq	%r10, %rcx
	subq	%rdx, %rcx
	leal	1(%rsi), %edx
	movslq	%edx, %rdx
	addq	%r9, %rcx
	addq	%r10, %rdx
	movl	(%rdi,%rcx,4), %ecx
	movl	(%rdi,%rdx,4), %edx
	cmpl	%edx, %ecx
	cmovbe	%rcx, %rdx
	cmpl	%edx, %eax
	cmova	%rdx, %rax
	jmp	.L5
	.p2align 4,,10
	.p2align 3
.L7:
	addl	$1, %edx
	movl	(%rdi), %eax
	cmpl	%edx, %ecx
	jle	.L8
	movslq	%edx, %rdx
	movq	%rdx, %rdi
	movl	(%rsi,%rdx,4), %r9d
	notq	%rdi
	addl	%ecx, %edi
	andl	$1, %edi
	cmpl	%r9d, %eax
	cmova	%r9d, %eax
	addq	$1, %rdx
	cmpl	%edx, %ecx
	jle	.L8
	testl	%edi, %edi
	je	.L10
	movl	(%rsi,%rdx,4), %edi
	cmpl	%edi, %eax
	cmova	%edi, %eax
	addq	$1, %rdx
	cmpl	%edx, %ecx
	jle	.L8
	.p2align 4,,10
	.p2align 3
.L10:
	movl	(%rsi,%rdx,4), %edi
	cmpl	%edi, %eax
	cmova	%edi, %eax
	movl	4(%rsi,%rdx,4), %edi
	cmpl	%edi, %eax
	cmova	%edi, %eax
	addq	$2, %rdx
	cmpl	%edx, %ecx
	jg	.L10
	jmp	.L8
.L13:
	xorl	%r13d, %r13d
	jmp	.L3
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
