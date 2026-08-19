/* The CLI. Owns the filesystem, the terminal and the process; one consumer of
   frontend/ among several (ADR-0020). Today it only proves the wiring links. */
#include "ludo_frontend.h"
#include "ludo_interp.h"

#include <stdio.h>

int main(void) {
    (void)printf("ludo %s (interp %s)\n", ludo_frontend_version(), ludo_interp_version());
    return 0;
}
