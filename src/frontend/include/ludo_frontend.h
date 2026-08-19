/* The frontend's umbrella header. One per library, ludo_ prefix on everything
   exported, no varargs, no errno (#130). Nothing here is the session API yet:
   this is the smoke target that proves the toolchain, not the frontend. */
#ifndef LUDO_FRONTEND_H
#define LUDO_FRONTEND_H

/* Compile-time identity of the library, so the gate has something to link. */
const char *ludo_frontend_version(void);

#endif /* LUDO_FRONTEND_H */
