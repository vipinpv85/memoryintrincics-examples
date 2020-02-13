; nasm -felf64 -F stabs lookupmatch.asm
; gcc mytest.c -ggdb -m 32 -o mytest

global  myasm_v1

section .text

myasm_v1:
  push ebp
  mov  ebp,esp
  push ebx
  push esi
  push edi

  mov eax,-1                   ; ret = -1
  mov ecx,0                    ; i = 0

  mov esi,[ebp + 8]            ; pkt
  mov edi,[ebp + 12]           ; db

  movdqu xmm2,[esi + 20]       ; db dst_ip
  movdqu xmm3,[edi]            ; pkt dst_ip

search_db: ; search for matching dst_ip
    mov eax,-1
    cmp ecx,1024
    je exitsearch

    mov ebx,[ebp + 20]         ; location of dst_ip in struct
    mov edx,[ebp + 16]         ; size of struct
    movdqu xmm2,[esi + ebx]    ; db dst_ip

    pxor xmm2,xmm3
    ptest xmm2,xmm2
    jz found_dstip

    add esi,edx
    add ecx,1
    jmp search_db

found_dstip: ; search for dst_port and sock id
    mov eax,esi               ; eax = db_entries[i]
    add eax,[ebp + 24]        ; offset of dstport - 16 bit
    mov ax,[eax]              ; eax = db_entries[i].dst_port
    cmp ax,[ebp + 36]         ; is arg-dst_port == db_entries[i].dst_port
    jne search_db

    mov eax,esi               ; eax = db_entries[i]
    add eax,[ebp + 28]        ; offset of sockid - 32 bit
    mov eax,[eax]             ; eax = sock_id
    cmp eax,[ebp + 32]        ; is arg-sockid == db_entries[i].sockid
    jne search_db

    mov eax,ecx                ; ret = i

exitsearch:
  pop edi
  pop esi
  pop ebx
  mov esp,ebp
  pop ebp
  ret
