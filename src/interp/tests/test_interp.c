#include "ludo_interp.h"
#include "test.h"

#include <string.h>

static void stub_fill_rect(void *c, ludo_target_id t, const ludo_rect_desc *d) {
    (void)c;
    (void)t;
    (void)d;
}
static void stub_fill_ellipse(void *c, ludo_target_id t, const ludo_ellipse_desc *d) {
    (void)c;
    (void)t;
    (void)d;
}
static void stub_stroke_rect(void *c, ludo_target_id t, const ludo_rect_stroke_desc *d) {
    (void)c;
    (void)t;
    (void)d;
}
static void stub_fill_text(void *c, ludo_target_id t, const ludo_text_desc *d) {
    (void)c;
    (void)t;
    (void)d;
}
static void stub_fill_sprite(void *c, ludo_target_id t, const ludo_sprite_desc *d) {
    (void)c;
    (void)t;
    (void)d;
}
static ludo_text_metrics stub_measure_text(void *c, const ludo_text_desc *d) {
    ludo_text_metrics m = {0};
    (void)c;
    (void)d;
    return m;
}
static ludo_voice_id stub_play(void *c, const ludo_voice_desc *d) {
    (void)c;
    (void)d;
    return 1u;
}
static void stub_stop(void *c, ludo_voice_id v) {
    (void)c;
    (void)v;
}
static void stub_set_voice(void *c, ludo_voice_id v, const ludo_voice_patch *p) {
    (void)c;
    (void)v;
    (void)p;
}
static uint64_t stub_cursor(void *c) {
    (void)c;
    return 0u;
}
static void stub_set_render_scale(void *c, float s) {
    (void)c;
    (void)s;
}
static ludo_host_status stub_storage_write(void *c, ludo_storage_id s, const uint8_t *b, size_t n) {
    (void)c;
    (void)s;
    (void)b;
    return n > 64u ? LUDO_HOST_ERR_OVERSIZE : LUDO_HOST_OK;
}

static void fill(ludo_host *host) {
    host->context = NULL;
    host->fill_rect = stub_fill_rect;
    host->fill_ellipse = stub_fill_ellipse;
    host->stroke_rect = stub_stroke_rect;
    host->fill_text = stub_fill_text;
    host->fill_sprite = stub_fill_sprite;
    host->measure_text = stub_measure_text;
    host->play = stub_play;
    host->stop = stub_stop;
    host->set_voice = stub_set_voice;
    host->cursor = stub_cursor;
    host->set_render_scale = stub_set_render_scale;
    host->storage_write = stub_storage_write;
}

LUDO_TEST_MAIN({
    ludo_host host;
    ludo_host zeroed = {0};

    LUDO_CHECK(strcmp(ludo_interp_version(), "") != 0);

    /* A complete vtable passes; a partial one is rejected at wiring, not
       mid-frame, and NULL never means no-op. */
    fill(&host);
    LUDO_CHECK(ludo_host_check(&host) == LUDO_HOST_OK);
    host.measure_text = NULL;
    LUDO_CHECK(ludo_host_check(&host) == LUDO_HOST_ERR_INCOMPLETE);
    LUDO_CHECK(ludo_host_check(&zeroed) == LUDO_HOST_ERR_INCOMPLETE);
    LUDO_CHECK(ludo_host_check(NULL) == LUDO_HOST_ERR_INCOMPLETE);

    /* LUDO_HANDLE_NONE is reserved on every handle type. */
    LUDO_CHECK(LUDO_HANDLE_NONE == 0u);
    fill(&host);
    LUDO_CHECK(host.play(NULL, NULL) != LUDO_HANDLE_NONE);
})
