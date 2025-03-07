.data

PUBLIC g_HookFunction
PUBLIC g_TrampolineAddress

g_HookFunction QWORD ?
g_TrampolineAddress QWORD ?

.code

PUBLIC UnifiedHookHandler

UnifiedHookHandler PROC
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    sub rsp, 64
    movaps [rsp +  0], xmm0
    movaps [rsp + 16], xmm1
    movaps [rsp + 32], xmm2
    movaps [rsp + 48], xmm3

    mov rax, qword ptr [g_HookFunction]  
    call rax

    movaps xmm0, [rsp +  0]
    movaps xmm1, [rsp + 16]
    movaps xmm2, [rsp + 32]
    movaps xmm3, [rsp + 48]
    add rsp, 64

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax

    jmp qword ptr [g_TrampolineAddress]
UnifiedHookHandler ENDP

END
