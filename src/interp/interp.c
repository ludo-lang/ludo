#include "ludo_interp.h"

const char *ludo_interp_version(void) { return "0.0.0-prototype"; }

/* Every member is required (#133). A NULL member would otherwise mean "no-op",
   which puts a branch on every draw call and pre-decides #134 in favour of
   zeros-and-defaults without arguing for it. */
ludo_host_status ludo_host_check(const ludo_host *host) {
    if (host == NULL) {
        return LUDO_HOST_ERR_INCOMPLETE;
    }
    if (host->fill_rect == NULL || host->fill_ellipse == NULL || host->stroke_rect == NULL ||
        host->fill_text == NULL || host->fill_sprite == NULL || host->measure_text == NULL ||
        host->play == NULL || host->stop == NULL || host->set_voice == NULL ||
        host->cursor == NULL || host->set_render_scale == NULL || host->storage_write == NULL) {
        return LUDO_HOST_ERR_INCOMPLETE;
    }
    return LUDO_HOST_OK;
}

/* ludo_null_host is declared but not defined: what the stub returns is #134's
   decision, and a definition here would make it silently. */
