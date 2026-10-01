// hello.c — минимальная проверка всей цепочки: crt0 -> main() -> printf()
// -> write()-обёртка -> настоящий SYS_WRITE. Плюс getpid()/gettick() как
// побочная проверка простых syscall-обёрток без указателей.
//
// Сборка (см. подробный header-комментарий в libc/crt0.S):
//   FLAGS="-m64 -ffreestanding -fno-builtin -fno-pic -fno-pie
//          -mgeneral-regs-only -mno-red-zone -nostdlib -static
//          -Wall -Wextra -Ilibc/include"
//   gcc $FLAGS -c libc/crt0.S -o crt0.o
//   gcc $FLAGS -c libc/src/string.c -o string.o
//   gcc $FLAGS -c libc/src/malloc.c -o malloc.o
//   gcc $FLAGS -c libc/src/printf.c -o printf.o
//   gcc $FLAGS -c libc/src/stdlib.c -o stdlib.o
//   gcc $FLAGS -c test/c/hello.c -o hello.o
//   ld -m elf_x86_64 -static -nostdlib -no-pie -o hello_c.elf
//      crt0.o hello.o string.o malloc.o printf.o stdlib.o

#include <stdio.h>
#include <lufira/syscall.h>

int main(void) {
    printf("Hello from LufiraOS libc!\n");
    printf("pid=%d tick=%lu\n", (int)sys_getpid(), (unsigned long)sys_gettick());
    printf("hex=%x dec=%d neg=%d char=%c pct=%%\n", 0xdead, 1234, -42, 'X');
    printf("all tests done\n");
    return 0;
}
