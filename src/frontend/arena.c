/* The one heap-owning component. Every allocation belongs to an arena and
   dies wholesale; returned addresses have max_align_t alignment. */
#include "arena.h"

#include <stdint.h>
#include <stdlib.h>

struct ludo_arena_block {
    struct ludo_arena_block *next;
    size_t used;
    size_t capacity;
    max_align_t data[];
};

void *ludo_arena_alloc(ludo_arena *arena, size_t size) {
    const size_t alignment = _Alignof(max_align_t);
    if (size > SIZE_MAX - (alignment - 1)) {
        return NULL;
    }
    size_t aligned = (size + alignment - 1) / alignment * alignment;
    ludo_arena_block *block = arena->head;
    if (block == NULL || aligned > block->capacity - block->used) {
        size_t capacity = aligned > 8192 ? aligned : 8192;
        if (capacity > SIZE_MAX - sizeof *block) {
            return NULL;
        }
        block = malloc(sizeof *block + capacity); /* ludo-allow-malloc */
        if (block == NULL) {
            return NULL;
        }
        block->next = arena->head;
        block->used = 0;
        block->capacity = capacity;
        arena->head = block;
    }
    void *result = (unsigned char *)block->data + block->used;
    block->used += aligned;
    return result;
}

void ludo_arena_destroy(ludo_arena *arena) {
    ludo_arena_block *block = arena->head;
    while (block != NULL) {
        ludo_arena_block *next = block->next;
        free(block); /* ludo-allow-malloc */
        block = next;
    }
    arena->head = NULL;
}
