/* The stub host (#134).
 *
 * A complete ludo_host that answers out of a table of constants, so that #49's
 * hole-finder can run reference.ludo with no window and still reach the
 * branches an input-conditional program takes. What it answers, and why it is a
 * table rather than a scripted trace, is argued in ludo_interp.h.
 *
 * It is its own translation unit rather than a few lines in a test file
 * because the hole-finder is a real consumer: the moment the stub takes a
 * table it has an interface, and driver/ links it for a headless run.
 */
#include "ludo_interp.h"

/* ---------------------------------------------------------------------- */
/* The tables                                                              */
/* ---------------------------------------------------------------------- */

/* Designated initialisers throughout: every field these tables leave out is
   deliberately zero, and spelling out a zero would hide which ones are the
   answer and which ones are the absence of a question. */

const ludo_stub_answers ludo_stub_answers_quiet = {
    .name = "quiet",
    .storage_write_status = LUDO_HOST_OK,
};

const ludo_stub_answers ludo_stub_answers_active = {
    .name = "active",
    .frame =
        {
            .players =
                {
                    /* Slot 0 only. ch6 6.4.5 makes the other three present and
                       idle, which is a state worth having in every run rather
                       than a fourth table. */
                    [0] =
                        {
                            .direction_left = {.x = 1.0f, .y = 0.0f},
                            .down = 1u << LUDO_BUTTON_ONE,
                            .pressed = 1u << LUDO_BUTTON_ONE,
                            .connected = true,
                        },
                },

            /* Above reference.ludo's 0.5 threshold, so the frame that reads it
               calls set_render_scale and the next frame reads 0.5 back. */
            .render_scale = 0.75f,
            .now_seconds = 1.5,
        },
    .text_metrics =
        {
            .advance = 24.0f,
            .bounds = {.position = {.x = 0.0f, .y = -12.0f}, .size = {.x = 24.0f, .y = 16.0f}},
        },
    .audio_cursor_step = 800u, /* one frame at 48 kHz, near enough */
    .storage_write_status = LUDO_HOST_OK,
};

/* Deliberately not a fourth independent axis: oversize is active with one field
   changed, because the point is the rescue tail, not a new input state. */
const ludo_stub_answers ludo_stub_answers_oversize = {
    .name = "oversize",
    .frame = {.players = {[0] = {.direction_left = {.x = 1.0f, .y = 0.0f},
                                 .down = 1u << LUDO_BUTTON_ONE,
                                 .pressed = 1u << LUDO_BUTTON_ONE,
                                 .connected = true}},
              .render_scale = 0.75f,
              .now_seconds = 1.5},
    .text_metrics = {.advance = 24.0f,
                     .bounds = {.position = {.x = 0.0f, .y = -12.0f},
                                .size = {.x = 24.0f, .y = 16.0f}}},
    .audio_cursor_step = 800u,
    .storage_write_status = LUDO_HOST_ERR_OVERSIZE,
};

const ludo_stub_answers *const ludo_stub_tables[LUDO_STUB_TABLE_COUNT] = {
    &ludo_stub_answers_quiet,
    &ludo_stub_answers_active,
    &ludo_stub_answers_oversize,
};

/* ---------------------------------------------------------------------- */
/* The entry points                                                        */
/* ---------------------------------------------------------------------- */

/* Every entry point is counted. It is not coverage -- #134 puts that on an
   execution bit per AST node, which needs an AST -- but it is the cheap proof
   that the vtable is wired to the evaluator at all, and a test can assert on
   it before an evaluator exists. */
static ludo_stub_state *state_of(void *context) { return (ludo_stub_state *)context; }

static void count(void *context) {
    ludo_stub_state *state = state_of(context);
    if (state != NULL) {
        state->calls++;
    }
}

static void stub_fill_rect(void *context, ludo_target_id target, const ludo_rect_desc *desc) {
    (void)target;
    (void)desc;
    count(context);
}

static void stub_fill_ellipse(void *context, ludo_target_id target, const ludo_ellipse_desc *desc) {
    (void)target;
    (void)desc;
    count(context);
}

static void stub_stroke_rect(void *context, ludo_target_id target,
                             const ludo_rect_stroke_desc *desc) {
    (void)target;
    (void)desc;
    count(context);
}

static void stub_fill_text(void *context, ludo_target_id target, const ludo_text_desc *desc) {
    (void)target;
    (void)desc;
    count(context);
}

static void stub_fill_sprite(void *context, ludo_target_id target, const ludo_sprite_desc *desc) {
    (void)target;
    (void)desc;
    count(context);
}

static ludo_text_metrics stub_measure_text(void *context, const ludo_text_desc *desc) {
    ludo_stub_state *state = state_of(context);
    ludo_text_metrics none = {0};
    (void)desc;
    count(context);
    if (state == NULL) {
        return none;
    }
    /* ch6 4.8.1 makes it pure and takes the same descriptor fill_text does, so
       a table answer is a constant rather than a measurement: what is measured
       is what is drawn only in the sense that neither one looks at the text. */
    return state->answers.text_metrics;
}

/* ch6 5.1.3: play returns a handle and never absence, and LUDO_HANDLE_NONE is
   never one a host mints. A counter, not a voice registry. */
static ludo_voice_id stub_play(void *context, const ludo_voice_desc *desc) {
    ludo_stub_state *state = state_of(context);
    (void)desc;
    count(context);
    if (state == NULL) {
        return LUDO_HANDLE_NONE + 1u;
    }
    state->next_handle++;
    return state->next_handle;
}

static void stub_stop(void *context, ludo_voice_id voice) {
    (void)voice;
    count(context);
}

static void stub_set_voice(void *context, ludo_voice_id voice, const ludo_voice_patch *patch) {
    (void)voice;
    (void)patch;
    count(context);
}

static uint64_t stub_audio_cursor(void *context) {
    ludo_stub_state *state = state_of(context);
    count(context);
    if (state == NULL) {
        return 0u;
    }
    return state->answers.audio_cursor_start + state->frame * state->answers.audio_cursor_step;
}

/* ch6 7.9: a preference the program may set, and ch6 7.11: the getter reports
   what the host applied. Writing it back into the latch is what lets a two-frame
   run observe that, instead of a comment claiming it. There is no quantisation
   grid here -- a real host has one, and a stub that invented one would be
   asserting a number no clause supplies. */
static void stub_set_render_scale(void *context, float scale) {
    ludo_stub_state *state = state_of(context);
    count(context);
    if (state != NULL) {
        state->answers.frame.render_scale = scale;
    }
}

static ludo_host_status stub_storage_write(void *context, ludo_storage_id slot,
                                           const uint8_t *bytes, size_t len) {
    ludo_stub_state *state = state_of(context);
    (void)slot;
    (void)bytes;
    (void)len;
    count(context);
    if (state == NULL) {
        return LUDO_HOST_OK;
    }
    /* The status is the table's, not a size comparison: ch6 8.10's limit is the
       host's business and a stub that picked a byte count would be legislating
       one. */
    return state->answers.storage_write_status;
}

/* ---------------------------------------------------------------------- */
/* Wiring                                                                  */
/* ---------------------------------------------------------------------- */

void ludo_stub_host(ludo_stub_state *state, ludo_host *host, const ludo_stub_answers *answers) {
    if (state == NULL || host == NULL) {
        return;
    }

    state->answers = (answers != NULL) ? *answers : ludo_stub_answers_quiet;
    state->frame = 0u;
    state->calls = 0u;
    state->next_handle = LUDO_HANDLE_NONE;

    /* The list ludo_host_check re-lists and LUDO_HOST_ENTRY_POINT_COUNT
       guards. A member added to the vtable breaks the static assertion, which
       sends its author here. */
    host->context = state;
    host->fill_rect = stub_fill_rect;
    host->fill_ellipse = stub_fill_ellipse;
    host->stroke_rect = stub_stroke_rect;
    host->fill_text = stub_fill_text;
    host->fill_sprite = stub_fill_sprite;
    host->measure_text = stub_measure_text;
    host->play = stub_play;
    host->stop = stub_stop;
    host->set_voice = stub_set_voice;
    host->audio_cursor = stub_audio_cursor;
    host->set_render_scale = stub_set_render_scale;
    host->storage_write = stub_storage_write;
}

void ludo_stub_constants(ludo_host_constants *constants) {
    if (constants == NULL) {
        return;
    }

    constants->canvas_size.x = 1280.0f;
    constants->canvas_size.y = 720.0f;
    constants->style = LUDO_STYLE_CRISP;
    constants->default_font = 1u;
    constants->debug_image = 2u;
    constants->screen = 3u;
    constants->sample_rate = 48000;

    /* The caller's, from the program's declarations. */
    constants->images = NULL;
    constants->image_count = 0u;
    constants->storage_slots = NULL;
    constants->storage_slot_count = 0u;
}

const ludo_frame_input *ludo_stub_frame(const ludo_stub_state *state) {
    static const ludo_frame_input idle = {0};
    if (state == NULL) {
        return &idle;
    }
    return &state->answers.frame;
}

void ludo_stub_next_frame(ludo_stub_state *state) {
    if (state != NULL) {
        state->frame++;
    }
}
