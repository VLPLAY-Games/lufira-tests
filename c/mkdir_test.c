// mkdir_test.c — проверяет новые syscall'ы SYS_MKDIR/SYS_RMDIR/SYS_UNLINK/
// SYS_READDIR (syscall.c) — vfs_mkdir()/vfs_rmdir()/vfs_unlink()/
// vfs_readdir() (vfs.h) уже были реализованы, но не выставлены в ring3.
//
// Сборка: см. build.py (build_c_tests()).

#include <stdio.h>
#include <lufira/syscall.h>
#include <string.h>

int main(void) {
    // 1. mkdir нового каталога в /tests (уже существует и доступен на
    // запись — сам /tests стейджится debug-сборкой).
    long r = sys_mkdir("/tests/mkdir_test_dir", 0755);
    if (r != 0) { printf("mkdir FAIL (%ld)\n", r); return 1; }
    printf("mkdir OK\n");

    // 2. Повторный mkdir того же имени должен провалиться (уже существует).
    r = sys_mkdir("/tests/mkdir_test_dir", 0755);
    if (r >= 0) { printf("mkdir-dup FAIL (expected error, got %ld)\n", r); return 1; }
    printf("mkdir-dup OK\n");

    // 3. Создаём файл внутри через sys_open(O_CREAT), затем ищем его через
    // sys_readdir() на открытом каталоге.
    long fd = sys_open("/tests/mkdir_test_dir/leaf.txt", O_CREAT | O_WRONLY, 0644);
    if (fd < 0) { printf("touch-leaf FAIL (%ld)\n", fd); return 1; }
    sys_close((int)fd);
    printf("touch-leaf OK\n");

    long dfd = sys_open("/tests/mkdir_test_dir", O_RDONLY, 0);
    if (dfd < 0) { printf("open-dir FAIL (%ld)\n", dfd); return 1; }

    struct lufira_dirent ent;
    int found = 0;
    for (;;) {
        long n = sys_readdir((int)dfd, &ent);
        if (n <= 0) break;
        if (strcmp(ent.name, "leaf.txt") == 0) { found = 1; break; }
    }
    sys_close((int)dfd);
    if (!found) { printf("readdir FAIL (leaf.txt not found)\n"); return 1; }
    printf("readdir OK\n");

    // 4. unlink файла, затем rmdir каталога — оба должны теперь пройти.
    r = sys_unlink("/tests/mkdir_test_dir/leaf.txt");
    if (r != 0) { printf("unlink FAIL (%ld)\n", r); return 1; }
    printf("unlink OK\n");

    r = sys_rmdir("/tests/mkdir_test_dir");
    if (r != 0) { printf("rmdir FAIL (%ld)\n", r); return 1; }
    printf("rmdir OK\n");

    // 5. Отрицательный случай: rmdir несуществующего пути должен провалиться.
    r = sys_rmdir("/tests/mkdir_test_dir");
    if (r >= 0) { printf("rmdir-missing FAIL (expected error, got %ld)\n", r); return 1; }
    printf("rmdir-missing OK\n");

    printf("all tests done\n");
    return 0;
}
