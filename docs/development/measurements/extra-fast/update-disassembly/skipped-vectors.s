	.file	"update-kernel.cpp"
	.text
	.section	.text._ZN9blueberry10extra_fast14SumSegmentTree8add_simdEmm,"axG",@progbits,_ZN9blueberry10extra_fast14SumSegmentTree8add_simdEmm,comdat
	.align 2
	.p2align 4
	.weak	_ZN9blueberry10extra_fast14SumSegmentTree8add_simdEmm
	.type	_ZN9blueberry10extra_fast14SumSegmentTree8add_simdEmm, @function
_ZN9blueberry10extra_fast14SumSegmentTree8add_simdEmm:
.LFB8341:
	.cfi_startproc
	endbr64
	movq	%rdi, %r8
	vmovq	%rdx, %xmm2
	leaq	_ZN9blueberry10extra_fast14SumSegmentTree6masks_E(%rip), %r10
	xorl	%edi, %edi
	movl	28(%r8), %eax
	vpbroadcastq	%xmm2, %ymm1
	testl	%eax, %eax
	jle	.L9
	.p2align 4,,10
	.p2align 3
.L4:
	movq	%rsi, %rcx
	movq	(%r8), %r9
	movq	%rsi, %rdx
	movq	%rsi, %rax
	andl	$15, %ecx
	andq	$-16, %rdx
	addq	40(%r8,%rdi,8), %rdx
	andl	$12, %eax
	salq	$7, %rcx
	leaq	(%r9,%rdx,8), %rdx
	addq	%r10, %rcx
.L3:
	vpand	(%rcx,%rax,8), %ymm1, %ymm0
	vpaddq	(%rdx,%rax,8), %ymm0, %ymm0
	vmovdqu	%ymm0, (%rdx,%rax,8)
	addq	$4, %rax
	cmpl	$15, %eax
	jle	.L3
	shrq	$4, %rsi
	addq	$1, %rdi
	cmpl	%edi, 28(%r8)
	jg	.L4
.L9:
	vzeroupper
	ret
	.cfi_endproc
.LFE8341:
	.size	_ZN9blueberry10extra_fast14SumSegmentTree8add_simdEmm, .-_ZN9blueberry10extra_fast14SumSegmentTree8add_simdEmm
	.text
	.p2align 4
	.globl	extra_fast_update
	.type	extra_fast_update, @function
extra_fast_update:
.LFB8342:
	.cfi_startproc
	endbr64
	cmpb	$0, 32(%rdi)
	movq	%rdx, %rcx
	jne	.L12
	movslq	28(%rdi), %rax
	testl	%eax, %eax
	jle	.L29
	movq	(%rdi), %r9
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movl	$15, %r10d
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	leaq	(%rdi,%rax,8), %rbx
	leaq	8(%r9), %r11
	.p2align 4,,10
	.p2align 3
.L16:
	movl	%esi, %ebp
	movslq	%esi, %rdx
	movl	%esi, %eax
	movl	%r10d, %r8d
	andl	$15, %eax
	andl	$15, %ebp
	andq	$-16, %rdx
	addq	40(%rdi), %rdx
	addq	%rax, %rdx
	subl	%ebp, %r8d
	leaq	(%r9,%rdx,8), %rax
	addq	%r8, %rdx
	leaq	(%r11,%rdx,8), %rdx
	movq	%rdx, %r8
	subq	%rax, %r8
	andl	$8, %r8d
	je	.L15
	addq	%rcx, (%rax)
	addq	$8, %rax
	cmpq	%rax, %rdx
	je	.L24
	.p2align 4,,10
	.p2align 3
.L15:
	addq	%rcx, (%rax)
	addq	%rcx, 8(%rax)
	addq	$16, %rax
	cmpq	%rax, %rdx
	jne	.L15
.L24:
	addq	$8, %rdi
	sarl	$4, %esi
	cmpq	%rdi, %rbx
	jne	.L16
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	ret
.L12:
	.cfi_restore 3
	.cfi_restore 6
	movslq	%esi, %rsi
	jmp	_ZN9blueberry10extra_fast14SumSegmentTree8add_simdEmm
.L29:
	ret
	.cfi_endproc
.LFE8342:
	.size	extra_fast_update, .-extra_fast_update
	.weak	_ZN9blueberry10extra_fast14SumSegmentTree6masks_E
	.section	.rodata._ZN9blueberry10extra_fast14SumSegmentTree6masks_E,"aG",@progbits,_ZN9blueberry10extra_fast14SumSegmentTree6masks_E,comdat
	.align 32
	.type	_ZN9blueberry10extra_fast14SumSegmentTree6masks_E, @gnu_unique_object
	.size	_ZN9blueberry10extra_fast14SumSegmentTree6masks_E, 2048
_ZN9blueberry10extra_fast14SumSegmentTree6masks_E:
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.zero	8
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.zero	16
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.zero	24
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.zero	32
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.zero	40
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.zero	48
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.zero	56
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.zero	64
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.zero	72
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.zero	80
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.zero	88
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.zero	96
	.quad	-1
	.quad	-1
	.quad	-1
	.quad	-1
	.zero	104
	.quad	-1
	.quad	-1
	.quad	-1
	.zero	112
	.quad	-1
	.quad	-1
	.zero	120
	.quad	-1
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
