;/*######   Copyright (c) 2013-2015 Ufasoft  http://ufasoft.com  mailto:support@ufasoft.com,  Sergey Pavlov  mailto:dev@ufasoft.com ####
;#                                                                                                                                     #
;# 		See LICENSE for licensing information                                                                                         #
;#####################################################################################################################################*/

; Montgomery Multiplication algorithm using SSE2
; http://en.wikipedia.org/wiki/Montgomery_reduction

INCLUDE el/x86x64.inc


IF X64
ELSE

.XMM

MontgomeryMul32_SSE PROC PUBLIC USES ZBX ZCX ZDX ZSI ZDI, pr:DWORD, pa:DWORD, pb:DWORD, pn:DWORD, pn0:DWORD, num:DWORD
	LOCAL	i:DWORD

	mov		ecx, num
	add		ecx, 2
	xor		eax, eax
$ZeroT:
	push	eax
	loop	$ZeroT
	mov		esi, esp						; esi = t (reversed) zeroed

	mov		edi, pn
	mov		ebx, pb	
	and		esp, NOT 0Fh					; (.... n2 n3 b2 b3 n0 n1 b0 b1), [esp+ecx*8]	in loops will be 16-aligned
	mov		ecx, num
	inc		ecx
	jmp		$StartFillNB
$FillNBLoop:
	push	DWORD PTR [ebx+4]
	push	DWORD PTR [ebx]
	push	DWORD PTR [edi+4]
	push	DWORD PTR [edi]
	add		edi, 8
	add		ebx, 8
$StartFillNB:
	sub		ecx, 2
	ja		$FillNBLoop
	jb		$EndNBLoop
	push	DWORD PTR [edi]
	push	DWORD PTR [ebx]					; (bLast nLast)
$EndNBLoop:
	sub		esp, 8
										
	pxor	xmm5, xmm5						; xmm5 = (0 0 0 0)
	mov		eax, -1
	movd	xmm6, eax						; const mask xmm6 = (-1 0 0 0)
	mov		ebx, pn0
	mov		ebx, [ebx]						; ebx = n'[0]
	mov		i, 0
	mov		edi, pa
	mov		edx, i
	mov		ecx, num
$Loop:
	mov		eax, [edi+edx*4]				; a[i]
	inc		edx
	mov		i, edx
	movd	xmm2, eax						; xmm2 = (a[i] 0 0 0)
	pshufd	xmm2, xmm2, 011001111b			; xmm2 = (0 0 a[i] 0)

	mul		DWORD PTR [esp+ecx*8]			; *= b0
	add		eax, [esi+ecx*4-4]
	mul		ebx								; eax = mul2
	movd	xmm1, eax						
	por		xmm2, xmm1						; xmm2 = (mul2  ?  a[i] ?)
	pxor	xmm7, xmm7						; carry xmm7 = 0
	
	inc		ecx
	jmp		$StartLoop
$Loop2:
	movdqa	xmm1, [esp+ecx*8]				; xmm1 = (n0 n1 b0 b1)
	movq	xmm3, QWORD PTR [esi+ecx*4-4]
	pshufd	xmm0, xmm1, 0110001b			; xmm0 = (n1 x b1 x)
	pshufd	xmm4, xmm3, 011111100b			; xmm4 = (t1 0 0 0)	
	pshufd	xmm3, xmm3, 011111101b			; xmm3 = (t0 0 0 0)	
	pmuludq	xmm1, xmm2
	pmuludq	xmm0, xmm2
	paddq	xmm1, xmm3						
	paddq	xmm0, xmm4

	paddq	xmm1, xmm7						; xmm1 = (maxsum1  sum2)
	pshufd	xmm7, xmm1, 001001110b			; xmm7 = (sum2 maxsum1)
	pand	xmm1, xmm6						; xmm1 = (Low(maxsum1)  0 0 0)
	paddq	xmm7, xmm1						; xmm7 = (sum2+Low(maxsum1) maxsum1)
	movd	DWORD PTR [esi+ecx*4+4], xmm7
	psrlq	xmm7, 32						; xmm7 = (c0 0 c1 0)
	
	paddq	xmm0, xmm7						; xmm0 = (maxsum1  sum2)
	pshufd	xmm7, xmm0, 001001110b			; xmm7 = (sum2 maxsum1)
	pand	xmm0, xmm6						; xmm0 = (Low(maxsum1)  0 0 0)
	paddq	xmm7, xmm0						; xmm7 = (sum2+Low(maxsum1) maxsum1)
	movd	DWORD PTR [esi+ecx*4], xmm7
	psrlq	xmm7, 32						; xmm7 = (c0 0 c1 0)
$StartLoop:
	sub		ecx, 2
	ja		$Loop2
	jb		$EndLoop

	pshufd	xmm1, [esp], 000100011b			; xmm1 = (x x bLast nLast)
	movd	xmm3, DWORD PTR [esi]
	pmuludq	xmm1, xmm2
	paddq	xmm7, xmm3						
	paddq	xmm1, xmm7						; xmm1 = (maxsum1  sum2)
	pshufd	xmm7, xmm1, 001001110b			; xmm7 = (sum2 maxsum1)
	pand	xmm1, xmm6						; xmm1 = (Low(maxsum1)  0 0 0)
	paddq	xmm7, xmm1						; xmm7 = (sum2+Low(maxsum1) maxsum1)
	movd	DWORD PTR [esi+4], xmm7
	psrlq	xmm7, 32						; xmm7 = (c0 0 c1 0)
$EndLoop:
	paddq	xmm5, xmm7
	pshufd	xmm7, xmm7, 011111110b				; xmm7 = (c1 0 0 0)
	paddq	xmm5, xmm7
	movd	DWORD PTR [esi], xmm5
	pshufd	xmm5, xmm5, 011111101b
	mov		edx, i
	mov		ecx, num
	cmp		edx, ecx
	jb		$Loop

	mov		ebx, pn	
	movd	eax, xmm5
	cmp		eax, 0
	mov		edi, pr
	ja		$SubLoop
	push	esi
$Cmp:
	lodsd
	cmp		eax, [ebx+ecx*4-4]
	loope	$Cmp
	pop		esi
	mov		ecx, num
	jb		$Mov
$SubLoop:									; CF==0 at iteration 0
	mov		eax, [esi+ecx*4-4]
	sbb		eax, [ebx]
	lea		ebx, [ebx+4]
	stosd
	loop	$SubLoop
	jmp		$Ret
$Mov:
	mov		eax, [esi+ecx*4-4]
	stosd
	loop	$Mov
$Ret:
	mov		eax, 1
	lea		esp, [ebp-6*4]
	ret
MontgomeryMul32_SSE	ENDP

ENDIF ; X64

		END



