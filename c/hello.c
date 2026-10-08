// hello.c — минимальная проверка всей цепочки: crt0 -> main() -> printf()
// -> write()-обёртка -> SYS_WRITE, плюс getpid()/gettick() как проверка
// простых syscall-обёрток без указателей.
//
// Сборка: см. build.py (build_c_tests()) — те же флаги gcc/ld, теперь
// заданы там одним местом вместо повтора в каждом файле.

#include <stdio.h>
#include <lufira/syscall.h>

int main(void) {
    printf("Hello from LufiraOS libc!\n");
    printf("pid=%d tick=%lu\n", (int)sys_getpid(), (unsigned long)sys_gettick());
    printf("hex=%x dec=%d neg=%d char=%c pct=%%\n", 0xdead, 1234, -42, 'X');
    printf("all tests done\n");
    return 0;
}
