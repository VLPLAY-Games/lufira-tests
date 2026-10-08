; busy_loop.asm — тест вытесняющей многозадачности: процесс без единого
; syscall (даже SYS_EXIT), крутит бесконечный счётчик. До
; context_enter_ring3()/timer-preemption такой процесс не покидал ring0
; и не мог быть вытеснен таймером — зависал навсегда.
;
; Сборка: см. build.py (build_asm_tests()).

global _start

section .text

_start:
    xor rax, rax
.loop:
    inc rax
    jmp .loop
