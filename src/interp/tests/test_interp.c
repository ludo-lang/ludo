#include "ludo_interp.h"
#include "test.h"

#include <string.h>

static void check_constants(void) {
    static const ludo_image_binding images[] = {{"rock", 7u}};
    static const ludo_storage_binding slots[] = {{"save1", 3u}};
    ludo_host_constants constants = {0};
    ludo_host_constants empty = {0};

    /* The stub's configuration is complete except for the bindings, which are
       the program's and which the caller supplies (#134). */
    ludo_stub_constants(&constants);
    LUDO_CHECK(ludo_constants_check(&constants) == LUDO_HOST_OK);
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

/* ch8 P15's vector, verbatim: set_render_scale(x) then render_scale() returns,
   for x in 1.0, 0.3, 0.25, 0.0, -1.0 and 4.0, respectively 1.0, 0.3125, 0.25,
   0.25, 0.25 and 1.0 -- no fault for any input, every result a multiple of 1/16
   in [0.25, 1.0]. The property is a runner's, so it is checked on the shared
   quantiser rather than on any one host. */
static void check_render_scale_grid(void) {
    static const float argument[] = {1.0f, 0.3f, 0.25f, 0.0f, -1.0f, 4.0f};
    static const float expected[] = {1.0f, 0.3125f, 0.25f, 0.25f, 0.25f, 1.0f};
    size_t i;

    for (i = 0; i < sizeof(argument) / sizeof(argument[0]); i++) {
        float got = ludo_quantise_render_scale(argument[i]);
        LUDO_CHECK(got == expected[i]);
        LUDO_CHECK(got >= LUDO_RENDER_SCALE_MIN && got <= LUDO_RENDER_SCALE_MAX);
        LUDO_CHECK(got * (float)LUDO_RENDER_SCALE_STEPS ==
                   (float)(int)(got * (float)LUDO_RENDER_SCALE_STEPS));
    }

    /* Thirteen legal values, and every one of them survives a round trip: a
       quantiser that collapsed the grid would still pass the vector above. */
    for (i = 0; i < 13u; i++) {
        float legal = LUDO_RENDER_SCALE_MIN + (float)i / (float)LUDO_RENDER_SCALE_STEPS;
        LUDO_CHECK(ludo_quantise_render_scale(legal) == legal);
    }
    LUDO_CHECK(LUDO_RENDER_SCALE_MIN + 12.0f / (float)LUDO_RENDER_SCALE_STEPS ==
               LUDO_RENDER_SCALE_MAX);
}

/* #134: what the stub answers, and the union of the tables being the coverage
   rather than any single run. */
static void check_stub_tables(void) {
    ludo_stub_state state;
    ludo_host host;
    size_t i;

    /* quiet: deterministic, and both conditional branches dead. That is the
       reading #134 refused to adopt on its own, so it is asserted rather than
       assumed. */
    ludo_stub_host(&state, &host, &ludo_stub_answers_quiet);
    LUDO_CHECK(ludo_host_check(&host) == LUDO_HOST_OK);
    LUDO_CHECK(!ludo_input_button_pressed(ludo_stub_frame(&state), LUDO_BUTTON_ONE, 0));
    /* ch6 7.11's floor. Not 0.0: below the grid is a value no conforming host
       can report, and the dead branch does not need an illegal answer. */
    LUDO_CHECK(ludo_stub_frame(&state)->render_scale == LUDO_RENDER_SCALE_MIN);
    LUDO_CHECK(!(ludo_stub_frame(&state)->render_scale > 0.5f));
    LUDO_CHECK(host.storage_write(host.context, 1u, NULL, 0u) == LUDO_HOST_OK);

    /* active: every branch reference.ludo takes on a host answer is live. */
    ludo_stub_host(&state, &host, &ludo_stub_answers_active);
    LUDO_CHECK(ludo_input_button_pressed(ludo_stub_frame(&state), LUDO_BUTTON_ONE, 0));
    LUDO_CHECK(ludo_input_direction_left(ludo_stub_frame(&state), 0).x == 1.0f);
    LUDO_CHECK(ludo_stub_frame(&state)->render_scale > 0.5f);
    LUDO_CHECK(host.measure_text(host.context, NULL).advance > 0.0f);
    LUDO_CHECK(host.storage_write(host.context, 1u, NULL, 0u) == LUDO_HOST_OK);

    /* ch6 7.11: the getter reports the QUANTISED value, never the argument. A
       one-frame run cannot see the difference, and an argument already on the
       grid cannot either -- so this passes one that is not. */
    host.set_render_scale(host.context, 0.3f);
    LUDO_CHECK(ludo_stub_frame(&state)->render_scale == 0.3125f);
    LUDO_CHECK(!(ludo_stub_frame(&state)->render_scale > 0.5f));

    /* oversize: the one failure ch6 8.10 lets a program see, which is a branch
       point and not a special case. */
    ludo_stub_host(&state, &host, &ludo_stub_answers_oversize);
    LUDO_CHECK(host.storage_write(host.context, 1u, NULL, 0u) == LUDO_HOST_ERR_OVERSIZE);

    /* Every table wires a complete host and names itself, because a coverage
       report that says "run 2" is a report nobody reads. */
    for (i = 0; i < (size_t)LUDO_STUB_TABLE_COUNT; i++) {
        ludo_stub_host(&state, &host, ludo_stub_tables[i]);
        LUDO_CHECK(ludo_host_check(&host) == LUDO_HOST_OK);
        LUDO_CHECK(ludo_stub_tables[i]->name != NULL);
    }
}

/* The two counters a spec clause forces, and nothing more: minted handles, and
   a cursor that advances per frame. */
static void check_stub_state(void) {
    ludo_stub_state state;
    ludo_host host;
    ludo_voice_id first;
    ludo_voice_id second;
    uint64_t at_zero;

    ludo_stub_host(&state, &host, &ludo_stub_answers_active);

    first = host.play(host.context, NULL);
    second = host.play(host.context, NULL);
    LUDO_CHECK(first != LUDO_HANDLE_NONE);
    LUDO_CHECK(second != LUDO_HANDLE_NONE);
    LUDO_CHECK(first != second);

    at_zero = host.audio_cursor(host.context);
    ludo_stub_next_frame(&state);
    LUDO_CHECK(host.audio_cursor(host.context) > at_zero);

    /* A NULL state reads idle rather than faulting, the same way an unplugged
       slot does. Zero here is the absence of an answer, not a render scale a
       host reported, which is why it is not on ch6 7.11's grid. */
    LUDO_CHECK(ludo_stub_frame(NULL)->render_scale == 0.0f);
    ludo_stub_next_frame(NULL);
    ludo_stub_host(NULL, &host, &ludo_stub_answers_quiet);
    ludo_stub_constants(NULL);
}

LUDO_TEST_MAIN({
    ludo_host host;
    ludo_host zeroed = {0};
    ludo_stub_state state;

    LUDO_CHECK(strcmp(ludo_interp_version(), "") != 0);

    /* A complete vtable passes; a partial one is rejected at wiring, not
       mid-frame, and NULL never means no-op. */
    ludo_stub_host(&state, &host, &ludo_stub_answers_active);
    LUDO_CHECK(ludo_host_check(&host) == LUDO_HOST_OK);
    host.measure_text = NULL;
    LUDO_CHECK(ludo_host_check(&host) == LUDO_HOST_ERR_INCOMPLETE);
    LUDO_CHECK(ludo_host_check(&zeroed) == LUDO_HOST_ERR_INCOMPLETE);
    LUDO_CHECK(ludo_host_check(NULL) == LUDO_HOST_ERR_INCOMPLETE);

    /* A host mints handles, and LUDO_HANDLE_NONE is never one it mints. */
    ludo_stub_host(&state, &host, &ludo_stub_answers_active);
    LUDO_CHECK(host.play(host.context, NULL) != LUDO_HANDLE_NONE);

    check_constants();
    check_latch();
    check_render_scale_grid();
    check_stub_tables();
    check_stub_state();
})
