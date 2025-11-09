#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define HEAP_SIZE (1024 * 64)
#define ALIGN 8
#define ALIGN_UP(x) (((x) + (ALIGN - 1)) & ~(ALIGN - 1))

typedef struct block_header {
    size_t size;
    int free;
    struct block_header* next;
} block_header;

static uint8_t heap[HEAP_SIZE];
static block_header* heap_start = NULL;

static inline void* header_to_payload(block_header* h) {
    return (void*)(h + 1);
}

static inline block_header* payload_to_header(void* p) {
    return ((block_header*)p) - 1;
}

void heap_init() {
    if (heap_start) return;
    heap_start = (block_header*)heap;
    heap_start->size = HEAP_SIZE - sizeof(block_header);
    heap_start->free = 1;
    heap_start->next = NULL;
}

block_header* find_fit(size_t size) {
    block_header* cur = heap_start;
    while (cur) {
        if (cur->free && cur->size >= size) return cur;
        cur = cur->next;
    }
    return NULL;
}

void split_block(block_header* b, size_t size) {
    size_t rem = b->size - size;
    if (rem <= sizeof(block_header) + ALIGN) return;

    block_header* newb = (block_header*)((uint8_t*)header_to_payload(b) + size);
    newb->size = rem - sizeof(block_header);
    newb->free = 1;
    newb->next = b->next;

    b->size = size;
    b->next = newb;
}

void coalecse() {
    block_header* cur = heap_start;
    while (cur && cur->next) {
        if (cur->free && cur->next->free) {
            cur->size = cur->size + sizeof(block_header) + cur->next->size;
            cur->next = cur->next->next;
        } else {
            cur = cur->next;
        }
    }
}

void* malloc_my(size_t size) {
    if (size == 0) return NULL;
    if (!heap_start) heap_init();

    size_t req = ALIGN_UP(size);
    block_header* b = find_fit(req);
    if (!b) return NULL;

    split_block(b, req);
    b->free = 0;
    return header_to_payload(b);
}

void free_my(void* ptr) {
    if (!ptr) return;
    block_header* b = payload_to_header(ptr);
    b->free = 1;
    coalecse();
}

void dump_heap() {
    printf("Heap map:\n");
    block_header* cur = heap_start;
    size_t idx = 0;
    while (cur) {
        printf("    block %zu: hdr=%p size%zu free=%d next=%p\n", idx, (void*)cur, cur->size, cur->free, (void*)cur->next);
        cur = cur->next;
        idx += 1;
    }
    printf("\n");
}

// Example test
int main() {
    heap_init();
    dump_heap();

    char* a = malloc_my(16);
    printf("alloc a -> %p\n", (void*)a);
    dump_heap();

    char* b = malloc_my(100);
    printf("alloc b -> %p\n", (void*)b);
    dump_heap();

    free_my(a);
    printf("free a\n");
    dump_heap();

    char* c = malloc_my(8);
    printf("alloc c -> %p\n", (void*)c);
    dump_heap();

    free_my(b);
    printf("free b\n");
    dump_heap();

    free_my(c);
    printf("free c\n");
    dump_heap();

    void* big = malloc_my(HEAP_SIZE / 2);
    printf("alloc big -> %p\n", big);
    dump_heap();

    free_my(big);
    return 0;
}