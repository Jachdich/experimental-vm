global    _start
section   .text
_start:
mov rdx, rsp
;wasn't sure if I should mess with rbp, temporarily abusing rdx as a base pointer

push 66
push 65
push rdx
mov rdx, rsp
call func
add rsp, 16

mov rax, 60
xor rdi, rdi
syscall

print:
mov rsi, rsp ;takes value on the stack, I know that's wrong...
mov rax, 1
mov rdi, 1
mov rdx, 1
syscall
add rsp, 8 ;clean up, since didn't pop
ret

func:
mov rax, rdx
add rax, 8
mov rax, [rax]
mov rbx, 0
cmp rax, rbx
jz true

mov rax, rdx
add rax, 8
mov rax, [rax]
push rax
call print
ret

true:
mov rax, rdx
add rax, 16
mov rax, [rax]
push rax
call print
ret
