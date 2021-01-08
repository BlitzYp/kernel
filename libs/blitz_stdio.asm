global strlen
strlen:
    push rcx
    mov rcx, rdi
    loop:
        cmp byte [rcx], 0
        je end
        inc rcx
        jmp loop
    end:
        ; Maybe this could also work mov rax, [rcx-rdi]
        sub rcx, rdi
        mov rax, rcx
        pop rcx
        ret

global int_to_string
;Takes in a number and a variable to store the string(int) in
int_to_string:
    mov rax, rsi ; The number
    mov r11, rdi ;The variable
    mov rbx, 10
    push rbx
put_values_in_the_stack:
    mov rdx, 0
    div rbx
    add rdx, 48
    push rdx
    cmp rax, 0
    jne put_values_in_the_stack
putting_the_number_in_the_variable:
    pop rcx
    mov [r11], rcx
    inc r11
    cmp rcx, 10
    jne putting_the_number_in_the_variable
    ret

global atoi
atoi:
    mov rax, 0x0
    push rbx
    mov rbx, 0xA
    push r11
    push rcx
    push rdi
    mov r11, 0x2D ; '-'
    cmp [rdi], r11
    je switch_ops_atoi
    mov r11, 1
    atoi_loop:
        movzx rcx, byte [rdi]
        mul rbx
        sub rcx, 0x30 ; ACSII 48
        add rax, rcx
        inc rdi
        cmp byte [rdi], 0x0
        je atoi_end
        jmp atoi_loop
    switch_ops_atoi:
        mov r11, -1
        inc rdi
        jmp atoi_loop
    atoi_end:
        imul rax, r11
        pop rdi
        pop rcx
        pop r11
        pop rbx
        ret

global strcpy
; Src is rdi
; dest is rsi
strcpy:
    push rax
    xor rax, rax
    strcpy_loop:
        mov rcx, [rdi+rax]
        mov [rsi+rax], rcx
        inc rax
        cmp byte [rdi+rax], 0x0
        je strcpy_end
        jmp strcpy_loop
    strcpy_end:
        pop rax
        ret

global strcmp
strcmp:
    push rdi
    push rsi
    loop_strcmp:
        mov bx, [rsi]
        mov cx, [rdi]
        cmp bx, cx
        jne strcmp_end
        inc rsi
        inc rdi
        test cx, cx
        jz strcmp_end
        test bx, bx
        jz strcmp_end
    strcmp_end:
        sub bx, cx
        pop rsi
        pop rdi
        ret