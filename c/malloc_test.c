// malloc_test.c — стресс-проверка аллокатора (libc/src/malloc.c):
// 1) ~3000 блоков живы одновременно, связаны в список прямо в
//    выделенной памяти (не на 16КБ стеке). ~192КБ суммарно продавливает
//    несколько ARENA_SIZE=64KiB арен — проверка, что "арена+подвыделение"
//    не упирается в MAX_MMAP_REGIONS=32, в отличие от mmap-на-malloc;
// 2) блоки разного размера с уникальным паттерном — нет порчи между ними;
// 3) освобождение половины + реаллок (слияние блоков) — живые не страдают.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRESS_COUNT    3000
#define STRESS_OBJ_SIZE 40
#define LIVE_COUNT      24

int main(void) {
    int fail = 0;

    // --- 1: тысячи блоков живы одновременно, список через первые 8 байт
    // каждого (не на стеке) ---
    void *head = NULL;
    int allocated = 0;
    for (int i = 0; i < STRESS_COUNT; i++) {
        void *p = malloc(STRESS_OBJ_SIZE);
        if (!p) {
            printf("stress-alloc FAIL at %d\n", i);
            fail = 1;
            break;
        }
        *(void **)p = head;
        head = p;
        allocated++;
    }
    if (!fail && allocated == STRESS_COUNT) {
        printf("stress-alloc OK\n");
    }
    while (head) {
        void *next = *(void **)head;
        free(head);
        head = next;
    }
    if (!fail) printf("stress-free OK\n");

    // --- 2: набор живых аллокаций разного размера, уникальный паттерн ---
    void *ptrs[LIVE_COUNT];
    size_t sizes[LIVE_COUNT];
    for (int i = 0; i < LIVE_COUNT; i++) {
        size_t sz = 16 + ((size_t)i * 37) % 500;
        sizes[i] = sz;
        ptrs[i] = malloc(sz);
        if (!ptrs[i]) {
            printf("live-alloc FAIL at %d\n", i);
            fail = 1;
            break;
        }
        memset(ptrs[i], i + 1, sz);
    }

    int corrupt = 0;
    for (int i = 0; i < LIVE_COUNT && !corrupt; i++) {
        unsigned char *p = (unsigned char *)ptrs[i];
        for (size_t j = 0; j < sizes[i]; j++) {
            if (p[j] != (unsigned char)(i + 1)) { corrupt = 1; break; }
        }
    }
    if (corrupt) {
        printf("live-pattern-check FAIL\n");
        fail = 1;
    } else {
        printf("live-pattern-check OK\n");
    }

    // --- 3: освобождаем чётные, выделяем заново — нечётные не должны пострадать ---
    for (int i = 0; i < LIVE_COUNT; i += 2) {
        free(ptrs[i]);
        ptrs[i] = NULL;
    }
    for (int i = 0; i < LIVE_COUNT; i += 2) {
        size_t sz = 8 + (size_t)i;
        void *p = malloc(sz);
        if (!p) {
            printf("realloc-after-free FAIL at %d\n", i);
            fail = 1;
            break;
        }
        memset(p, 0xAA, sz);
        ptrs[i] = p;
    }

    corrupt = 0;
    for (int i = 1; i < LIVE_COUNT && !corrupt; i += 2) {
        unsigned char *p = (unsigned char *)ptrs[i];
        for (size_t j = 0; j < sizes[i]; j++) {
            if (p[j] != (unsigned char)(i + 1)) { corrupt = 1; break; }
        }
    }
    if (corrupt) {
        printf("odd-survivors-intact FAIL\n");
        fail = 1;
    } else {
        printf("odd-survivors-intact OK\n");
    }

    printf(fail ? "malloc_test FAILED\n" : "all tests done\n");
    return fail;
}
