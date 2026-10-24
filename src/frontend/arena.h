#ifndef LUDO_ARENA_H
#define LUDO_ARENA_H

#include <stddef.h>

typedef struct ludo_arena_block ludo_arena_block;
typedef struct ludo_arena {
    ludo_arena_block *head;
} ludo_arena;

void *ludo_arena_alloc(ludo_arena *arena, size_t size);
void ludo_arena_destroy(ludo_arena *arena);

#endif
