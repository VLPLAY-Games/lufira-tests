// argv_test.c — проверяет реальную передачу argc/argv/envp через
// crt0.S -> main() (build_exec_stack() в process.c, elf_exec() в elf.c) —
// раньше ядро не передавало аргументы командной строки вовсе.
//
// Сборка: см. build.py (build_c_tests()).

#include <stdio.h>
#include <lufira/syscall.h>

int main(int argc, char **argv, char **envp) {
    printf("argc=%d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]=%s\n", i, argv[i]);
    }
    printf("envp-null=%d\n", envp == 0 || envp[0] == 0);
    printf("all tests done\n");
    return 0;
}
