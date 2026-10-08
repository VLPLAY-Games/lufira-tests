; fork_test.asm — демонстрация fork(): SYS_WRITE=0, SYS_EXIT=2, SYS_FORK=12
;
; Сборка: см. build.py (build_asm_tests()).

global _start

section .text

_start:
    mov rax, 12             ; SYS_FORK
    syscall

    cmp rax, 0
    je .child

.parent:
    mov rax, 0              ; SYS_WRITE
    mov rdi, 1              ; stdout
    lea rsi, [rel parent_msg]
    mov rdx, parent_len
    syscall

    mov rax, 2              ; SYS_EXIT
    mov rdi, 0
    syscall

.child:
    mov rax, 0              ; SYS_WRITE
    mov rdi, 1              ; stdout
    lea rsi, [rel child_msg]
    mov rdx, child_len
    syscall

    mov rax, 2              ; SYS_EXIT
    mov rdi, 0
    syscall

section .data

parent_msg: db 'Parent process here!', 10
parent_len: equ $ - parent_msg

child_msg: db 'Child process here!', 10
child_len: equ $ - child_msg
