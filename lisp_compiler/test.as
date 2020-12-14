global    _start
section   .text
_start:
        mov rdx, rsp

        mov rax, 2
        push rax
        push rdx
        mov rdx, rsp
        call label1
        add rsp, 8
        push r9
        mov rsi, rsp
        call print
        add rsp, 8

        mov rax, 60
        xor rdi, rdi
        syscall

print:  mov rax, 1
        mov rdi, 1
        mov rdx, 1
        syscall
        ret
        
label1: mov rax, 0
        push rax
        mov rax, rdx
        add rax, 8
        mov rax, [rax]
        push rax
        pop rax
        pop rbx
        cmp rax, rbx
        jz label4
        mov rax, 1
        push rax
        mov rax, 0
        push rax
        pop rax
        pop rbx
        sub rax, rbx
        push rax
        mov rax, rdx
        add rax, 8
        mov rax, [rax]
        push rax
        pop rax
        pop rbx
        cmp rax, rbx
        jz label2
        mov rax, 2
        push rax
        mov rax, rdx
        add rax, 8
        mov rax, [rax]
        push rax
        pop rax
        pop rbx
        sub rax, rbx
        push rax
        push rdx
        mov rdx, rsp
        call label1
        add rsp, 8
        push r9
        mov rax, 1
        push rax
        mov rax, rdx
        add rax, 8
        mov rax, [rax]
        push rax
        pop rax
        pop rbx
        sub rax, rbx
        push rax
        push rdx
        mov rdx, rsp
        call label1
        add rsp, 8
        push r9
        pop rax
        pop rbx
        add rax, rbx
        push rax
        pop r9
        jmp label3
    
label2: mov rax, 1
        push rax
        pop r9

label3: jmp label5

label4: mov rax, 1
        push rax
        pop r9

label5: ret

section .data

