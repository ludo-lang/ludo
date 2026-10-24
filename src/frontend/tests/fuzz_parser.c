/* Same entry point for search and every-check corpus replay. The parser must
   terminate, retain source bounds, and publish a tree rather than a graph. */
#include "../arena.h"
#include "ludo_frontend.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

static void require(bool ok) {
    if (!ok) {
        abort();
    }
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    ludo_session *session = NULL;
    if (ludo_session_create(&session) != LUDO_FRONTEND_OK) {
        return 0;
    }
    const ludo_parsed_file *file = NULL;
    if (ludo_parse_file(session, "fuzz", (const char *)data, size, &file) == LUDO_FRONTEND_OK) {
        require(file != NULL && file->ast.root != 0 && file->ast.root < file->ast.count);
        require(file->length == size && file->source == (const char *)data);
        ludo_arena arena = {0};
        size_t *parents = ludo_arena_alloc(&arena, file->ast.count * sizeof *parents);
        if (parents == NULL) {
            ludo_session_destroy(session);
            return 0;
        }
        memset(parents, 0, file->ast.count * sizeof *parents);
        size_t errors = 0;
        for (size_t i = 1; i < file->ast.count; i++) {
            const ludo_ast_node *node = ludo_ast_get(&file->ast, i);
            require(node->span.file == file->id);
            require(node->span.offset <= size && node->span.length <= size - node->span.offset);
            require(node->token < file->token_count);
            size_t visits = 0;
            ludo_node_id child = node->first_child;
            ludo_node_id last = 0;
            while (child != 0) {
                require(child < file->ast.count && ++visits < file->ast.count);
                require(++parents[child] == 1);
                last = child;
                child = ludo_ast_get(&file->ast, child)->next_sibling;
            }
            require(last == node->last_child);
            if (node->kind == LUDO_AST_ERROR) {
                errors++;
            }
        }
        for (size_t i = 1; i < file->ast.count; i++) {
            require(parents[i] == (i == file->ast.root ? 0U : 1U));
        }
        /* One-parent counts also admit disconnected cycles; prove reachability
           from the root with the same scratch array used as an explicit stack. */
        size_t pending = 1;
        size_t visited = 0;
        parents[0] = file->ast.root;
        while (pending != 0) {
            ludo_node_id current = parents[--pending];
            require(++visited < file->ast.count);
            ludo_node_id child = ludo_ast_get(&file->ast, current)->first_child;
            while (child != 0) {
                require(pending < file->ast.count);
                parents[pending++] = child;
                child = ludo_ast_get(&file->ast, child)->next_sibling;
            }
        }
        require(visited == file->ast.count - 1);
        require((errors == 0) == (file->diagnostic_count == 0));
        for (size_t i = 0; i < file->diagnostic_count; i++) {
            ludo_span span = file->diagnostics[i].primary;
            require(span.file == file->id);
            require(span.offset <= size && span.length <= size - span.offset);
        }
        ludo_arena_destroy(&arena);
    } else {
        require(file == NULL);
    }
    ludo_session_destroy(session);
    return 0;
}
