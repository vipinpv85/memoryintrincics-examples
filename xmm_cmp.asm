;  - windows visual studio MASM compiled code 
;  - extern "C" char* mystrchr_asm(const char* string, const unsigned char ch);

.model flat,c

.data

.code
	mystrchr_asm	PROC
		push ebp
		mov ebp,esp

		mov eax,[ebp +8]		; load ptr of orginal string
		mov ebx,[ebp +12]		; load ascii of the value to compare
		xor ecx,ecx				; set counter for 16 byte stride
		xor edx,edx				; set counter for byte pos

		pxor xmm4,xmm4			; set xmm4 = 0
		movd xmm3,ebx			; set xmm4 = ascii value to compare
		pshufb xmm3,xmm4		; set all bytes with index 0 value

@@:
		pxor xmm5,xmm5			; check for end of string
		movdqa xmm4,xmm3		; check for comparision

		movdqu xmm2,[eax + ecx]	; load the original text
				
		pcmpeqb xmm4,xmm2		; if match the byte is set 0xFF 
		pcmpeqb xmm5,xmm2		; if end of string (0) is found set 0xFF

		pmovmskb esi,xmm4		; extract MSB in xmm4 to form 8 bit value
		cmp esi,0
		jg chrmatch				; match found

		pmovmskb edi,xmm5		; extract MSB in xmm5 EOS
		cmp edi,0
		jg strend				; end of string found

		add ecx,16				; if no EOS or no match, stride to next 16 bytes
		jmp @B

chrmatch:
		bsf edx,esi				; edx = position in which first bit is set
		add edx,ecx				; ecx is stride index
								; actual position = stride index + byte match position
		add eax,edx				; update the pointer to location of match

		pop ebp
		ret

strend:
		mov eax,0				; no match return ptr as NULL

		pop ebp
		ret
	mystrchr_asm	ENDP
end
