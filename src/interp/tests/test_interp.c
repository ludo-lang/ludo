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

static void populate_host(ludo_host *host) {
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
    host->audio_cursor = stub_cursor;
    host->set_render_scale = stub_set_render_scale;
    host->storage_write = stub_storage_write;
}

static void check_constants(void) {
    static const ludo_image_binding images[] = {{"rock", 7u}};
    static const ludo_storage_binding slots[] = {{"save1", 3u}};
    ludo_host_constants constants = {0};
    ludo_host_constants empty = {0};

    constants.canvas_size.x = 1280.0f;
    constants.canvas_size.y = 720.0f;
    constants.style = LUDO_STYLE_CRISP;
    constants.default_font = 1u;
    constants.debug_image = 2u;
    constants.screen = 1u;
    constants.sample_rate = 48000;
    constants.images = images;
    constants.image_count = 1u;
    constants.storage_slots = slots;
    constants.storage_slot_count = 1u;

    LUDO_CHECK(ludo_constants_check(&constants) == LUDO_HOST_OK);
    LUDO_CHECK(ludo_constants_check(&empty) == LUDO_HOST_ERR_INCOMPLETE);
    LUDO_CHECK(ludo_constants_check(NULL) == LUDO_HOST_ERR_INCOMPLETE);

    /* A count with no array is the mistake worth catching: it reads as bindings
       that are simply absent right up to the first dereference. */
    constants.images = NULL;
    LUDO_CHECK(ludo_constants_check(&constants) == LUDO_HOST_ERR_INCOMPLETE);
    constants.images = images;

    constants.sample_rate = 0;
    LUDO_CHECK(ludo_constants_check(&constants) == LUDO_HOST_ERR_INCOMPLETE);
    constants.sample_rate = 48000;

    LUDO_CHECK(ludo_image_by_name(&constants, "rock") == 7u);
    LUDO_CHECK(ludo_image_by_name(&constants, "missing") == LUDO_HANDLE_NONE);
    LUDO_CHECK(ludo_storage_by_name(&constants, "save1") == 3u);
    LUDO_CHECK(ludo_storage_by_name(&constants, "save2") == LUDO_HANDLE_NONE);
}

static void check_latch(void) {
    ludo_frame_input input = {0};

    input.players[0].direction_left.x = 1.0f;
    input.players[0].down = (uint16_t)(1u << LUDO_BUTTON_ONE);
    input.players[0].pressed = (uint16_t)(1u << LUDO_BUTTON_ONE);
    input.players[0].connected = true;

    LUDO_CHECK(ludo_input_direction_left(&input, 0).x == 1.0f);
    LUDO_CHECK(ludo_input_button_down(&input, LUDO_BUTTON_ONE, 0));
    LUDO_CHECK(ludo_input_button_pressed(&input, LUDO_BUTTON_ONE, 0));
    LUDO_CHECK(!ludo_input_button_released(&input, LUDO_BUTTON_ONE, 0));
    LUDO_CHECK(!ludo_input_button_down(&input, LUDO_BUTTON_TWELVE, 0));

    /* ch6 6.4.5: an unplugged pad's slot is present and idle, and 6.4.6 says it
       does not renumber the ones above it. An index past the four slots reads
       the same way rather than faulting. */
    LUDO_CHECK(!ludo_input_button_down(&input, LUDO_BUTTON_ONE, 3));
    LUDO_CHECK(ludo_input_direction_left(&input, 3).x == 0.0f);
    LUDO_CHECK(!ludo_input_button_down(&input, LUDO_BUTTON_ONE, 99));
    LUDO_CHECK(!ludo_input_button_down(NULL, LUDO_BUTTON_ONE, 0));
}

LUDO_TEST_MAIN({
    ludo_host host;
    ludo_host zeroed = {0};

    LUDO_CHECK(strcmp(ludo_interp_version(), "") != 0);

    /* A complete vtable passes; a partial one is rejected at wiring, not
       mid-frame, and NULL never means no-op. */
    populate_host(&host);
    LUDO_CHECK(ludo_host_check(&host) == LUDO_HOST_OK);
    host.measure_text = NULL;
    LUDO_CHECK(ludo_host_check(&host) == LUDO_HOST_ERR_INCOMPLETE);
    LUDO_CHECK(ludo_host_check(&zeroed) == LUDO_HOST_ERR_INCOMPLETE);
    LUDO_CHECK(ludo_host_check(NULL) == LUDO_HOST_ERR_INCOMPLETE);

    /* A host mints handles, and LUDO_HANDLE_NONE is never one it mints. */
    populate_host(&host);
    LUDO_CHECK(host.play(NULL, NULL) != LUDO_HANDLE_NONE);

    check_constants();
    check_latch();
})
