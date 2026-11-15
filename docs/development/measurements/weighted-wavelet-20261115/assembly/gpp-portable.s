	.file	"weighted-wavelet-assembly.cpp"
	.intel_syntax noprefix
	.text
	.globl	__popcountdi2
	.p2align 4
	.globl	weighted_add
	.type	weighted_add, @function
weighted_add:
.LFB2333:
	.cfi_startproc
	endbr64
	push	r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	lea	eax, 1[rsi]
	push	r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	push	r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	mov	r13d, esi
	push	r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	mov	r12, rdx
	push	rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	push	rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	sub	rsp, 24
	.cfi_def_cfa_offset 80
	mov	esi, DWORD PTR 56[rdi]
	cmp	esi, eax
	jl	.L2
	mov	rcx, QWORD PTR 64[rdi]
	.p2align 4,,10
	.p2align 3
.L3:
	lea	edx, -1[rax]
	movsx	rdx, edx
	add	QWORD PTR [rcx+rdx*8], r12
	mov	edx, eax
	neg	edx
	and	edx, eax
	add	eax, edx
	cmp	esi, eax
	jge	.L3
.L2:
	mov	rax, QWORD PTR 40[rdi]
	mov	r14, QWORD PTR 32[rdi]
	mov	QWORD PTR 8[rsp], rax
	cmp	r14, rax
	je	.L1
	.p2align 4,,10
	.p2align 3
.L9:
	test	r13d, r13d
	lea	edx, 63[r13]
	mov	rax, QWORD PTR [r14]
	mov	ecx, r13d
	cmovns	edx, r13d
	mov	rdi, -1
	mov	r15d, r13d
	sal	rdi, cl
	and	r15d, 63
	sar	edx, 6
	not	rdi
	movsx	rbx, edx
	mov	rbp, QWORD PTR [rax+rbx*8]
	and	rdi, rbp
	call	__popcountdi2@PLT
	mov	rcx, QWORD PTR 24[r14]
	add	eax, DWORD PTR [rcx+rbx*4]
	sub	r13d, eax
	bt	rbp, r15
	jnc	.L6
	add	eax, DWORD PTR 48[r14]
	mov	r13d, eax
.L6:
	mov	edi, DWORD PTR 56[r14]
	lea	eax, 1[r13]
	cmp	edi, eax
	jl	.L7
	mov	rsi, QWORD PTR 64[r14]
	.p2align 4,,10
	.p2align 3
.L8:
	lea	edx, -1[rax]
	movsx	rdx, edx
	add	QWORD PTR [rsi+rdx*8], r12
	mov	edx, eax
	neg	edx
	and	edx, eax
	add	eax, edx
	cmp	edi, eax
	jge	.L8
.L7:
	add	r14, 88
	cmp	QWORD PTR 8[rsp], r14
	jne	.L9
.L1:
	add	rsp, 24
	.cfi_def_cfa_offset 56
	pop	rbx
	.cfi_def_cfa_offset 48
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
	mov	rax, rcx
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
	mov	ebp, esi
	push	rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	sub	rsp, 72
	.cfi_def_cfa_offset 128
	mov	rcx, QWORD PTR 8[rdi]
	mov	r10, QWORD PTR [rax]
	mov	rax, QWORD PTR 16[rdi]
	sub	rax, rcx
	mov	rsi, rax
	sar	rsi, 3
	test	rax, rax
	jle	.L53
	mov	ebx, edx
	mov	r9, rcx
	mov	rax, rsi
	jmp	.L19
	.p2align 4,,10
	.p2align 3
.L82:
	sub	rax, rdx
	lea	r9, 8[r11]
	sub	rax, 1
	test	rax, rax
	jle	.L81
.L19:
	mov	rdx, rax
	sar	rdx
	lea	r11, [r9+rdx*8]
	cmp	r10, QWORD PTR [r11]
	jg	.L82
	mov	rax, rdx
	test	rax, rax
	jg	.L19
.L81:
	sub	r9, rcx
	mov	r10, QWORD PTR [r8]
	mov	rax, rsi
	mov	r8, rcx
	sar	r9, 3
	mov	DWORD PTR 32[rsp], r9d
	jmp	.L21
	.p2align 4,,10
	.p2align 3
.L84:
	sub	rax, rdx
	lea	r8, 8[r11]
	sub	rax, 1
	test	rax, rax
	jle	.L83
.L21:
	mov	rdx, rax
	sar	rdx
	lea	r11, [r8+rdx*8]
	cmp	r10, QWORD PTR [r11]
	jg	.L84
	mov	rax, rdx
	test	rax, rax
	jg	.L21
.L83:
	sub	r8, rcx
	sar	r8, 3
	mov	DWORD PTR 24[rsp], r8d
	cmp	r8d, r9d
	je	.L53
	mov	r15, QWORD PTR 40[rdi]
	mov	r13, QWORD PTR 32[rdi]
	movabs	rax, 3353953467947191203
	mov	rcx, r15
	sub	rcx, r13
	sar	rcx, 3
	imul	rcx, rax
	mov	r11d, ecx
	cmp	r8d, esi
	je	.L85
	test	ecx, ecx
	jle	.L53
	mov	eax, r9d
	xor	r14d, r14d
	mov	r15d, ebx
	mov	DWORD PTR 44[rsp], ecx
	xor	eax, r8d
	mov	ebx, r14d
	mov	QWORD PTR 48[rsp], r13
	mov	r14, r13
	mov	DWORD PTR 36[rsp], eax
	lea	eax, -1[rcx]
	mov	DWORD PTR 40[rsp], eax
	mov	QWORD PTR 56[rsp], rcx
	jmp	.L35
	.p2align 4,,10
	.p2align 3
.L87:
	mov	eax, DWORD PTR 48[r14]
	add	ebx, 1
	add	r14, 88
	mov	edi, eax
	sub	eax, ecx
	add	r15d, eax
	mov	eax, DWORD PTR 44[rsp]
	sub	edi, r12d
	add	ebp, edi
	cmp	ebx, eax
	je	.L53
.L35:
	mov	r10d, DWORD PTR 40[rsp]
	mov	eax, DWORD PTR 36[rsp]
	sub	r10d, ebx
	mov	ecx, r10d
	sar	eax, cl
	test	eax, eax
	jne	.L86
	test	ebp, ebp
	lea	r12d, 63[rbp]
	mov	r11, QWORD PTR [r14]
	mov	ecx, ebp
	cmovns	r12d, ebp
	mov	rdi, -1
	mov	r13, QWORD PTR 24[r14]
	mov	DWORD PTR 16[rsp], r10d
	sal	rdi, cl
	mov	QWORD PTR 8[rsp], r11
	sar	r12d, 6
	not	rdi
	movsx	r12, r12d
	and	rdi, QWORD PTR [r11+r12*8]
	call	__popcountdi2@PLT
	add	eax, DWORD PTR 0[r13+r12*4]
	mov	r12d, ebp
	mov	ecx, r15d
	mov	QWORD PTR [rsp], r13
	sub	r12d, eax
	lea	r13d, 63[r15]
	test	r15d, r15d
	cmovns	r13d, r15d
	mov	r11, QWORD PTR 8[rsp]
	mov	rdi, -1
	sal	rdi, cl
	sar	r13d, 6
	not	rdi
	movsx	r13, r13d
	and	rdi, QWORD PTR [r11+r13*8]
	call	__popcountdi2@PLT
	mov	rsi, QWORD PTR [rsp]
	mov	ecx, r15d
	mov	r10d, DWORD PTR 16[rsp]
	add	eax, DWORD PTR [rsi+r13*4]
	sub	ecx, eax
	mov	eax, DWORD PTR 32[rsp]
	bt	eax, r10d
	jc	.L87
	mov	eax, DWORD PTR 44[rsp]
	add	ebx, 1
	mov	r15d, ecx
	mov	ebp, r12d
	add	r14, 88
	cmp	ebx, eax
	jne	.L35
.L53:
	xor	eax, eax
.L16:
	add	rsp, 72
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	pop	rbx
	.cfi_def_cfa_offset 48
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
	mov	rcx, QWORD PTR 56[rsp]
	mov	r14d, ebx
	mov	r13, QWORD PTR 48[rsp]
	mov	ebx, r15d
	cmp	r14d, ecx
	jge	.L53
	movsx	rsi, r14d
	mov	DWORD PTR 36[rsp], r10d
	mov	r14d, ebx
	lea	rax, [rsi+rsi*4]
	mov	DWORD PTR 40[rsp], ebp
	lea	rax, [rsi+rax*2]
	mov	DWORD PTR 44[rsp], ebx
	mov	ebx, r10d
	mov	QWORD PTR 16[rsp], 0
	lea	r15, 0[r13+rax*8]
	mov	r13d, ebp
	mov	QWORD PTR 8[rsp], r15
	mov	rbp, r15
	.p2align 4,,10
	.p2align 3
.L43:
	test	r13d, r13d
	lea	edx, 63[r13]
	mov	r9, QWORD PTR 0[rbp]
	mov	ecx, r13d
	cmovns	edx, r13d
	mov	rdi, -1
	mov	r15, QWORD PTR 24[rbp]
	sal	rdi, cl
	mov	QWORD PTR [rsp], r9
	sar	edx, 6
	not	rdi
	movsx	r12, edx
	and	rdi, QWORD PTR [r9+r12*8]
	call	__popcountdi2@PLT
	add	eax, DWORD PTR [r15+r12*4]
	mov	r12d, r13d
	lea	edx, 63[r14]
	sub	r12d, eax
	test	r14d, r14d
	mov	r9, QWORD PTR [rsp]
	mov	ecx, r14d
	cmovns	edx, r14d
	mov	rdi, -1
	sal	rdi, cl
	sar	edx, 6
	not	rdi
	movsx	rdx, edx
	and	rdi, QWORD PTR [r9+rdx*8]
	mov	QWORD PTR [rsp], rdx
	call	__popcountdi2@PLT
	mov	rdx, QWORD PTR [rsp]
	mov	r9d, r14d
	add	eax, DWORD PTR [r15+rdx*4]
	sub	r9d, eax
	mov	eax, DWORD PTR 24[rsp]
	bt	eax, ebx
	jnc	.L60
	test	r9d, r9d
	jle	.L61
	mov	r11, QWORD PTR 64[rbp]
	mov	eax, r9d
	xor	ecx, ecx
	.p2align 4,,10
	.p2align 3
.L40:
	lea	edx, -1[rax]
	movsx	rsi, edx
	add	rcx, QWORD PTR [r11+rsi*8]
	and	eax, edx
	jne	.L40
.L39:
	test	r12d, r12d
	jle	.L41
	mov	r11, QWORD PTR 64[rbp]
	mov	eax, r12d
	xor	edi, edi
	.p2align 4,,10
	.p2align 3
.L42:
	lea	edx, -1[rax]
	movsx	rsi, edx
	add	rdi, QWORD PTR [r11+rsi*8]
	and	eax, edx
	jne	.L42
	sub	rcx, rdi
.L41:
	mov	eax, DWORD PTR 48[rbp]
	add	QWORD PTR 16[rsp], rcx
	mov	edx, eax
	sub	eax, r9d
	sub	edx, r12d
	add	r14d, eax
	add	r13d, edx
.L38:
	add	rbp, 88
	sub	ebx, 1
	jnb	.L43
	xor	r8d, r8d
	mov	r15, QWORD PTR 8[rsp]
	mov	r12d, DWORD PTR 36[rsp]
	mov	ebp, DWORD PTR 40[rsp]
	mov	ebx, DWORD PTR 44[rsp]
	mov	QWORD PTR 8[rsp], r8
	mov	r13d, r12d
	.p2align 4,,10
	.p2align 3
.L49:
	test	ebp, ebp
	lea	edx, 63[rbp]
	mov	r9, QWORD PTR [r15]
	mov	ecx, ebp
	cmovns	edx, ebp
	mov	rdi, -1
	mov	r12, QWORD PTR 24[r15]
	sal	rdi, cl
	mov	QWORD PTR [rsp], r9
	sar	edx, 6
	not	rdi
	movsx	r14, edx
	and	rdi, QWORD PTR [r9+r14*8]
	call	__popcountdi2@PLT
	add	eax, DWORD PTR [r12+r14*4]
	mov	r14d, ebp
	lea	edx, 63[rbx]
	sub	r14d, eax
	test	ebx, ebx
	mov	r9, QWORD PTR [rsp]
	mov	ecx, ebx
	cmovns	edx, ebx
	mov	rdi, -1
	sal	rdi, cl
	sar	edx, 6
	not	rdi
	movsx	rdx, edx
	and	rdi, QWORD PTR [r9+rdx*8]
	mov	QWORD PTR [rsp], rdx
	call	__popcountdi2@PLT
	mov	rdx, QWORD PTR [rsp]
	mov	r9d, ebx
	add	eax, DWORD PTR [r12+rdx*4]
	sub	r9d, eax
	mov	eax, DWORD PTR 32[rsp]
	bt	eax, r13d
	jnc	.L62
	test	r9d, r9d
	jle	.L63
	mov	r10, QWORD PTR 64[r15]
	mov	eax, r9d
	xor	ecx, ecx
	.p2align 4,,10
	.p2align 3
.L46:
	lea	edx, -1[rax]
	movsx	rsi, edx
	add	rcx, QWORD PTR [r10+rsi*8]
	and	eax, edx
	jne	.L46
.L45:
	test	r14d, r14d
	jle	.L47
	mov	r10, QWORD PTR 64[r15]
	mov	eax, r14d
	xor	edi, edi
	.p2align 4,,10
	.p2align 3
.L48:
	lea	edx, -1[rax]
	movsx	rsi, edx
	add	rdi, QWORD PTR [r10+rsi*8]
	and	eax, edx
	jne	.L48
	sub	rcx, rdi
.L47:
	mov	eax, DWORD PTR 48[r15]
	add	QWORD PTR 8[rsp], rcx
	mov	edx, eax
	sub	eax, r9d
	sub	edx, r14d
	add	ebx, eax
	add	ebp, edx
.L44:
	add	r15, 88
	sub	r13d, 1
	jnb	.L49
	mov	r8, QWORD PTR 8[rsp]
	mov	rax, QWORD PTR 16[rsp]
	add	rsp, 72
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	pop	rbx
	.cfi_def_cfa_offset 48
	pop	rbp
	.cfi_def_cfa_offset 40
	sub	rax, r8
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
.L62:
	.cfi_restore_state
	mov	ebx, r9d
	mov	ebp, r14d
	jmp	.L44
	.p2align 4,,10
	.p2align 3
.L60:
	mov	r14d, r9d
	mov	r13d, r12d
	jmp	.L38
	.p2align 4,,10
	.p2align 3
.L85:
	test	ebx, ebx
	jle	.L54
	mov	r8, QWORD PTR 64[rdi]
	mov	edx, ebx
	xor	eax, eax
	.p2align 4,,10
	.p2align 3
.L24:
	lea	ecx, -1[rdx]
	movsx	rsi, ecx
	add	rax, QWORD PTR [r8+rsi*8]
	and	edx, ecx
	jne	.L24
.L23:
	test	ebp, ebp
	jle	.L25
	mov	r8, QWORD PTR 64[rdi]
	mov	edx, ebp
	xor	esi, esi
	.p2align 4,,10
	.p2align 3
.L26:
	lea	ecx, -1[rdx]
	movsx	rdi, ecx
	add	rsi, QWORD PTR [r8+rdi*8]
	and	edx, ecx
	jne	.L26
	sub	rax, rsi
.L25:
	cmp	r15, r13
	je	.L16
	xor	ecx, ecx
	mov	QWORD PTR 24[rsp], rax
	mov	r14, r13
	mov	r13d, r11d
	mov	QWORD PTR 16[rsp], rcx
	mov	QWORD PTR 8[rsp], r15
	mov	r15d, ebp
	.p2align 4,,10
	.p2align 3
.L33:
	sub	r13d, 1
	lea	r9d, 63[r15]
	test	r15d, r15d
	mov	r10, QWORD PTR [r14]
	cmovns	r9d, r15d
	mov	ecx, r15d
	mov	rdi, -1
	mov	r12, QWORD PTR 24[r14]
	sal	rdi, cl
	mov	QWORD PTR [rsp], r10
	sar	r9d, 6
	not	rdi
	movsx	rbp, r9d
	and	rdi, QWORD PTR [r10+rbp*8]
	call	__popcountdi2@PLT
	add	eax, DWORD PTR [r12+rbp*4]
	lea	r9d, 63[rbx]
	mov	ecx, ebx
	sub	r15d, eax
	test	ebx, ebx
	mov	r10, QWORD PTR [rsp]
	mov	rdi, -1
	cmovns	r9d, ebx
	sal	rdi, cl
	mov	ebp, eax
	not	rdi
	sar	r9d, 6
	movsx	r9, r9d
	and	rdi, QWORD PTR [r10+r9*8]
	mov	QWORD PTR [rsp], r9
	call	__popcountdi2@PLT
	mov	r9, QWORD PTR [rsp]
	add	eax, DWORD PTR [r12+r9*4]
	sub	ebx, eax
	mov	r10d, eax
	mov	eax, DWORD PTR 32[rsp]
	bt	eax, r13d
	jnc	.L28
	test	ebx, ebx
	jle	.L55
	mov	r9, QWORD PTR 64[r14]
	xor	ecx, ecx
	.p2align 4,,10
	.p2align 3
.L30:
	lea	eax, -1[rbx]
	movsx	rdi, eax
	add	rcx, QWORD PTR [r9+rdi*8]
	and	ebx, eax
	jne	.L30
.L29:
	test	r15d, r15d
	jle	.L31
	mov	rbx, QWORD PTR 64[r14]
	xor	edi, edi
	.p2align 4,,10
	.p2align 3
.L32:
	lea	eax, -1[r15]
	movsx	r9, eax
	add	rdi, QWORD PTR [rbx+r9*8]
	and	r15d, eax
	jne	.L32
	sub	rcx, rdi
.L31:
	add	QWORD PTR 16[rsp], rcx
	mov	ebx, DWORD PTR 48[r14]
	lea	r15d, [rbx+rbp]
	add	ebx, r10d
.L28:
	add	r14, 88
	cmp	QWORD PTR 8[rsp], r14
	jne	.L33
	mov	rcx, QWORD PTR 16[rsp]
	mov	rax, QWORD PTR 24[rsp]
	add	rsp, 72
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	pop	rbx
	.cfi_def_cfa_offset 48
	pop	rbp
	.cfi_def_cfa_offset 40
	sub	rax, rcx
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
	xor	ecx, ecx
	jmp	.L45
	.p2align 4,,10
	.p2align 3
.L61:
	xor	ecx, ecx
	jmp	.L39
.L55:
	xor	ecx, ecx
	jmp	.L29
.L54:
	xor	eax, eax
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
