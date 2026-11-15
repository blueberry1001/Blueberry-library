	.file	"weighted-wavelet-assembly.cpp"
	.intel_syntax noprefix
	.text
	.p2align 4
	.globl	weighted_add
	.type	weighted_add, @function
weighted_add:
.LFB2333:
	.cfi_startproc
	endbr64
	lea	eax, 1[rsi]
	mov	r9d, esi
	mov	esi, DWORD PTR 56[rdi]
	mov	r10, rdi
	mov	r8, rdx
	cmp	esi, eax
	jl	.L2
	mov	rcx, QWORD PTR 64[rdi]
	.p2align 4,,10
	.p2align 3
.L3:
	lea	edx, -1[rax]
	movsx	rdx, edx
	add	QWORD PTR [rcx+rdx*8], r8
	blsi	edx, eax
	add	eax, edx
	cmp	esi, eax
	jge	.L3
.L2:
	mov	rdi, QWORD PTR 32[r10]
	mov	r10, QWORD PTR 40[r10]
	cmp	rdi, r10
	je	.L14
	mov	r11, -1
	.p2align 4,,10
	.p2align 3
.L9:
	test	r9d, r9d
	lea	eax, 63[r9]
	mov	rdx, QWORD PTR [rdi]
	mov	rsi, QWORD PTR 24[rdi]
	cmovns	eax, r9d
	sar	eax, 6
	cdqe
	mov	rcx, QWORD PTR [rdx+rax*8]
	shlx	rdx, r11, r9
	andn	rdx, rdx, rcx
	popcnt	rdx, rdx
	add	edx, DWORD PTR [rsi+rax*4]
	shrx	rcx, rcx, r9
	sub	r9d, edx
	and	ecx, 1
	mov	eax, edx
	je	.L6
	add	eax, DWORD PTR 48[rdi]
	mov	r9d, eax
.L6:
	mov	esi, DWORD PTR 56[rdi]
	lea	eax, 1[r9]
	cmp	esi, eax
	jl	.L7
	mov	rcx, QWORD PTR 64[rdi]
	.p2align 4,,10
	.p2align 3
.L8:
	lea	edx, -1[rax]
	movsx	rdx, edx
	add	QWORD PTR [rcx+rdx*8], r8
	blsi	edx, eax
	add	eax, edx
	cmp	esi, eax
	jge	.L8
.L7:
	add	rdi, 88
	cmp	r10, rdi
	jne	.L9
.L14:
	ret
	.cfi_endproc
.LFE2333:
	.size	weighted_add, .-weighted_add
	.section	.text._ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_,"axG",@progbits,_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_,comdat
	.align 2
	.p2align 4
	.weak	_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_
	.type	_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_, @function
_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_:
.LFB2345:
	.cfi_startproc
	endbr64
	push	r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	mov	eax, edx
	push	r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	push	r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	push	r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	push	rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	push	rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	mov	rbx, QWORD PTR 8[rdi]
	mov	rdx, QWORD PTR 16[rdi]
	mov	rbp, QWORD PTR [rcx]
	sub	rdx, rbx
	mov	r11, rdx
	sar	r11, 3
	test	rdx, rdx
	jle	.L53
	mov	r10, rdi
	mov	r9, rbx
	mov	rdx, r11
	jmp	.L19
	.p2align 4,,10
	.p2align 3
.L82:
	sub	rdx, rcx
	lea	r9, 8[rdi]
	sub	rdx, 1
	test	rdx, rdx
	jle	.L81
.L19:
	mov	rcx, rdx
	sar	rcx
	lea	rdi, [r9+rcx*8]
	cmp	rbp, QWORD PTR [rdi]
	jg	.L82
	mov	rdx, rcx
	test	rdx, rdx
	jg	.L19
.L81:
	sub	r9, rbx
	mov	r12, QWORD PTR [r8]
	mov	rbp, rbx
	mov	rdx, r11
	sar	r9, 3
	mov	edi, r9d
	jmp	.L21
	.p2align 4,,10
	.p2align 3
.L84:
	sub	rdx, rcx
	lea	rbp, 8[r8]
	sub	rdx, 1
	test	rdx, rdx
	jle	.L83
.L21:
	mov	rcx, rdx
	sar	rcx
	lea	r8, 0[rbp+rcx*8]
	cmp	r12, QWORD PTR [r8]
	jg	.L84
	mov	rdx, rcx
	test	rdx, rdx
	jg	.L21
.L83:
	sub	rbp, rbx
	mov	rdx, rbp
	sar	rdx, 3
	cmp	edx, r9d
	mov	DWORD PTR -32[rsp], edx
	je	.L53
	mov	r8, QWORD PTR 40[r10]
	mov	rbp, QWORD PTR 32[r10]
	movabs	rbx, 3353953467947191203
	mov	rcx, r8
	sub	rcx, rbp
	sar	rcx, 3
	imul	rbx, rcx
	cmp	edx, r11d
	mov	r12d, ebx
	je	.L85
	test	ebx, ebx
	jle	.L53
	mov	QWORD PTR -24[rsp], rbp
	xor	r9d, edx
	lea	r15d, -1[rbx]
	mov	r11, rbp
	mov	QWORD PTR -16[rsp], rbx
	xor	r10d, r10d
	mov	r13, -1
	jmp	.L35
	.p2align 4,,10
	.p2align 3
.L87:
	mov	edx, DWORD PTR 48[r11]
	add	r10d, 1
	add	r11, 88
	mov	r8d, edx
	sub	edx, ecx
	sub	r8d, r14d
	add	eax, edx
	add	esi, r8d
	cmp	r10d, r12d
	je	.L53
.L35:
	mov	edx, r15d
	sub	edx, r10d
	sarx	ecx, r9d, edx
	test	ecx, ecx
	jne	.L86
	test	esi, esi
	lea	ecx, 63[rsi]
	mov	rbx, QWORD PTR 24[r11]
	mov	rbp, QWORD PTR [r11]
	cmovns	ecx, esi
	shlx	r8, r13, rsi
	sar	ecx, 6
	movsx	rcx, ecx
	mov	r14d, DWORD PTR [rbx+rcx*4]
	andn	r8, r8, QWORD PTR 0[rbp+rcx*8]
	popcnt	r8, r8
	add	r14d, r8d
	shlx	r8, r13, rax
	mov	ecx, r14d
	mov	r14d, esi
	sub	r14d, ecx
	test	eax, eax
	lea	ecx, 63[rax]
	cmovns	ecx, eax
	sar	ecx, 6
	movsx	rcx, ecx
	andn	r8, r8, QWORD PTR 0[rbp+rcx*8]
	popcnt	r8, r8
	add	r8d, DWORD PTR [rbx+rcx*4]
	mov	ecx, eax
	sub	ecx, r8d
	bt	edi, edx
	jc	.L87
	add	r10d, 1
	add	r11, 88
	mov	eax, ecx
	mov	esi, r14d
	cmp	r10d, r12d
	jne	.L35
.L53:
	xor	ecx, ecx
.L16:
	pop	rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 48
	mov	rax, rcx
	pop	rbp
	.cfi_def_cfa_offset 40
	pop	r12
	.cfi_def_cfa_offset 32
	pop	r13
	.cfi_def_cfa_offset 24
	pop	r14
	.cfi_def_cfa_offset 16
	pop	r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L86:
	.cfi_restore_state
	mov	rbx, QWORD PTR -16[rsp]
	mov	rbp, QWORD PTR -24[rsp]
	cmp	r10d, ebx
	jge	.L53
	movsx	r10, r10d
	mov	DWORD PTR -24[rsp], edi
	xor	ebx, ebx
	mov	r14d, DWORD PTR -32[rsp]
	lea	rcx, [r10+r10*4]
	mov	DWORD PTR -4[rsp], eax
	mov	r13d, edx
	mov	r9d, esi
	lea	rcx, [r10+rcx*2]
	mov	r15d, edx
	mov	r10d, eax
	mov	rax, rbx
	lea	rcx, 0[rbp+rcx*8]
	mov	QWORD PTR -16[rsp], rcx
	mov	r11, rcx
	.p2align 4,,10
	.p2align 3
.L43:
	test	r9d, r9d
	lea	edx, 63[r9]
	mov	rdi, QWORD PTR 24[r11]
	mov	r8, QWORD PTR [r11]
	cmovns	edx, r9d
	mov	rbp, -1
	mov	r12d, r10d
	shlx	rcx, rbp, r9
	sar	edx, 6
	movsx	rdx, edx
	mov	ebx, DWORD PTR [rdi+rdx*4]
	andn	rcx, rcx, QWORD PTR [r8+rdx*8]
	popcnt	rcx, rcx
	add	ebx, ecx
	shlx	rcx, rbp, r10
	mov	edx, ebx
	mov	ebx, r9d
	sub	ebx, edx
	test	r10d, r10d
	lea	edx, 63[r10]
	cmovns	edx, r10d
	sar	edx, 6
	movsx	rdx, edx
	andn	rcx, rcx, QWORD PTR [r8+rdx*8]
	popcnt	rcx, rcx
	add	ecx, DWORD PTR [rdi+rdx*4]
	sub	r12d, ecx
	bt	r14d, r13d
	jnc	.L60
	test	r12d, r12d
	jle	.L61
	mov	rbp, QWORD PTR 64[r11]
	mov	edx, r12d
	xor	edi, edi
	.p2align 4,,10
	.p2align 3
.L40:
	lea	ecx, -1[rdx]
	movsx	r8, ecx
	add	rdi, QWORD PTR 0[rbp+r8*8]
	and	edx, ecx
	jne	.L40
.L39:
	test	ebx, ebx
	jle	.L41
	mov	QWORD PTR -32[rsp], rdi
	mov	rbp, QWORD PTR 64[r11]
	mov	edx, ebx
	xor	r8d, r8d
	.p2align 4,,10
	.p2align 3
.L42:
	lea	ecx, -1[rdx]
	movsx	rdi, ecx
	add	r8, QWORD PTR 0[rbp+rdi*8]
	and	edx, ecx
	jne	.L42
	mov	rdi, QWORD PTR -32[rsp]
	sub	rdi, r8
.L41:
	mov	edx, DWORD PTR 48[r11]
	add	rax, rdi
	mov	ecx, edx
	sub	edx, r12d
	sub	ecx, ebx
	add	r10d, edx
	add	r9d, ecx
.L38:
	add	r11, 88
	sub	r13d, 1
	jnb	.L43
	mov	QWORD PTR -32[rsp], rax
	mov	edi, DWORD PTR -24[rsp]
	mov	edx, r15d
	xor	r14d, r14d
	mov	rcx, QWORD PTR -16[rsp]
	mov	eax, DWORD PTR -4[rsp]
	mov	r13, -1
	.p2align 4,,10
	.p2align 3
.L49:
	test	esi, esi
	lea	r8d, 63[rsi]
	mov	r10, QWORD PTR 24[rcx]
	mov	rbx, QWORD PTR [rcx]
	cmovns	r8d, esi
	mov	r11d, esi
	shlx	r9, r13, rsi
	mov	ebp, eax
	sar	r8d, 6
	movsx	r8, r8d
	mov	r15d, DWORD PTR [r10+r8*4]
	andn	r9, r9, QWORD PTR [rbx+r8*8]
	popcnt	r9, r9
	lea	r8d, 63[rax]
	add	r15d, r9d
	shlx	r9, r13, rax
	sub	r11d, r15d
	test	eax, eax
	cmovns	r8d, eax
	sar	r8d, 6
	movsx	r8, r8d
	andn	r9, r9, QWORD PTR [rbx+r8*8]
	mov	ebx, DWORD PTR [r10+r8*4]
	popcnt	r9, r9
	add	ebx, r9d
	sub	ebp, ebx
	bt	edi, edx
	jnc	.L62
	test	ebp, ebp
	jle	.L63
	mov	r12, QWORD PTR 64[rcx]
	mov	r8d, ebp
	xor	r10d, r10d
	.p2align 4,,10
	.p2align 3
.L46:
	lea	r9d, -1[r8]
	movsx	rbx, r9d
	add	r10, QWORD PTR [r12+rbx*8]
	and	r8d, r9d
	jne	.L46
.L45:
	test	r11d, r11d
	jle	.L47
	mov	r15, QWORD PTR 64[rcx]
	mov	r8d, r11d
	xor	ebx, ebx
	.p2align 4,,10
	.p2align 3
.L48:
	lea	r9d, -1[r8]
	movsx	r12, r9d
	add	rbx, QWORD PTR [r15+r12*8]
	and	r8d, r9d
	jne	.L48
	sub	r10, rbx
.L47:
	mov	r8d, DWORD PTR 48[rcx]
	add	r14, r10
	mov	r9d, r8d
	sub	r8d, ebp
	sub	r9d, r11d
	add	eax, r8d
	add	esi, r9d
.L44:
	add	rcx, 88
	sub	edx, 1
	jnb	.L49
	mov	rcx, QWORD PTR -32[rsp]
	pop	rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 48
	pop	rbp
	.cfi_def_cfa_offset 40
	pop	r12
	.cfi_def_cfa_offset 32
	sub	rcx, r14
	pop	r13
	.cfi_def_cfa_offset 24
	pop	r14
	.cfi_def_cfa_offset 16
	mov	rax, rcx
	pop	r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L62:
	.cfi_restore_state
	mov	eax, ebp
	mov	esi, r11d
	jmp	.L44
	.p2align 4,,10
	.p2align 3
.L60:
	mov	r10d, r12d
	mov	r9d, ebx
	jmp	.L38
	.p2align 4,,10
	.p2align 3
.L85:
	test	eax, eax
	jle	.L54
	mov	rbx, QWORD PTR 64[r10]
	mov	edx, eax
	xor	ecx, ecx
	.p2align 4,,10
	.p2align 3
.L24:
	lea	r9d, -1[rdx]
	movsx	r11, r9d
	add	rcx, QWORD PTR [rbx+r11*8]
	and	edx, r9d
	jne	.L24
.L23:
	test	esi, esi
	jle	.L25
	mov	rbx, QWORD PTR 64[r10]
	mov	edx, esi
	xor	r10d, r10d
	.p2align 4,,10
	.p2align 3
.L26:
	lea	r9d, -1[rdx]
	movsx	r11, r9d
	add	r10, QWORD PTR [rbx+r11*8]
	and	edx, r9d
	jne	.L26
	sub	rcx, r10
.L25:
	cmp	r8, rbp
	je	.L16
	mov	r9, rbp
	xor	r11d, r11d
	mov	r10, -1
	.p2align 4,,10
	.p2align 3
.L33:
	sub	r12d, 1
	lea	edx, 63[rsi]
	test	esi, esi
	mov	rbx, QWORD PTR 24[r9]
	cmovns	edx, esi
	mov	r14, QWORD PTR [r9]
	shlx	rbp, r10, rsi
	shlx	r13, r10, rax
	sar	edx, 6
	movsx	rdx, edx
	andn	rbp, rbp, QWORD PTR [r14+rdx*8]
	popcnt	rbp, rbp
	add	ebp, DWORD PTR [rbx+rdx*4]
	lea	edx, 63[rax]
	sub	esi, ebp
	test	eax, eax
	cmovns	edx, eax
	sar	edx, 6
	movsx	rdx, edx
	andn	r13, r13, QWORD PTR [r14+rdx*8]
	popcnt	r13, r13
	add	r13d, DWORD PTR [rbx+rdx*4]
	sub	eax, r13d
	bt	edi, r12d
	jnc	.L28
	test	eax, eax
	jle	.L55
	mov	r15, QWORD PTR 64[r9]
	xor	ebx, ebx
	.p2align 4,,10
	.p2align 3
.L30:
	lea	edx, -1[rax]
	movsx	r14, edx
	add	rbx, QWORD PTR [r15+r14*8]
	and	eax, edx
	jne	.L30
.L29:
	test	esi, esi
	jle	.L31
	mov	r15, QWORD PTR 64[r9]
	xor	edx, edx
	.p2align 4,,10
	.p2align 3
.L32:
	lea	eax, -1[rsi]
	movsx	r14, eax
	add	rdx, QWORD PTR [r15+r14*8]
	and	esi, eax
	jne	.L32
	sub	rbx, rdx
.L31:
	mov	eax, DWORD PTR 48[r9]
	add	r11, rbx
	lea	esi, [rax+rbp]
	add	eax, r13d
.L28:
	add	r9, 88
	cmp	r8, r9
	jne	.L33
	sub	rcx, r11
	pop	rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 48
	pop	rbp
	.cfi_def_cfa_offset 40
	mov	rax, rcx
	pop	r12
	.cfi_def_cfa_offset 32
	pop	r13
	.cfi_def_cfa_offset 24
	pop	r14
	.cfi_def_cfa_offset 16
	pop	r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L63:
	.cfi_restore_state
	xor	r10d, r10d
	jmp	.L45
	.p2align 4,,10
	.p2align 3
.L61:
	xor	edi, edi
	jmp	.L39
.L55:
	xor	ebx, ebx
	jmp	.L29
.L54:
	xor	ecx, ecx
	jmp	.L23
	.cfi_endproc
.LFE2345:
	.size	_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_, .-_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_
	.text
	.p2align 4
	.globl	weighted_sum
	.type	weighted_sum, @function
weighted_sum:
.LFB2334:
	.cfi_startproc
	endbr64
	sub	rsp, 24
	.cfi_def_cfa_offset 32
	mov	QWORD PTR 8[rsp], rcx
	lea	rcx, 8[rsp]
	mov	QWORD PTR [rsp], r8
	mov	r8, rsp
	call	_ZNK9blueberry21WeightedWaveletMatrixIxxE3sumEiiRKxS3_
	add	rsp, 24
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE2334:
	.size	weighted_sum, .-weighted_sum
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
