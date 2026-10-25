/* The session's private layout, shared by the parser and the checker. Not part
   of the public surface: consumers hold an opaque ludo_session (#130). */
#ifndef LUDO_SESSION_H
#define LUDO_SESSION_H

#include "arena.h"
#include "ludo_frontend.h"

typedef struct parsed_unit {
    ludo_parsed_file file;
    ludo_arena arena;
    struct parsed_unit *next;
} parsed_unit;

struct ludo_session {
    ludo_arena arena;
    parsed_unit *files;
    size_t count;
    /* Every checked file's tables. Checking publishes into its own arena so a
       parse arena stays exactly what the parser wrote. */
    ludo_arena check_arena;
};

#endif /* LUDO_SESSION_H */
