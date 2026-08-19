/* The interpreter's umbrella header. interp/ declares the host interface and
   platform/ implements it (#96); the surface itself is #133's, so this holds
   only the smoke target that proves the toolchain. */
#ifndef LUDO_INTERP_H
#define LUDO_INTERP_H

const char *ludo_interp_version(void);

#endif /* LUDO_INTERP_H */
