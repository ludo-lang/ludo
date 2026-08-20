#include "ludo_interp.h"

#include <string.h>

const char *ludo_interp_version(void) { return "0.0.0-prototype"; }

/* Every member is required (#133). A NULL member would otherwise mean "no-op",
   which puts a branch on every draw call and pre-decides #134 in favour of
   zeros-and-defaults without arguing for it.

   The list is mirrored by LUDO_HOST_ENTRY_POINT_COUNT's assertion in the
   header: adding a member without touching this function breaks the build. */
ludo_host_status ludo_host_check(const ludo_host *host) {
    if (host == NULL) {
        return LUDO_HOST_ERR_INCOMPLETE;
    }
    if (host->fill_rect == NULL || host->fill_ellipse == NULL || host->stroke_rect == NULL ||
        host->fill_text == NULL || host->fill_sprite == NULL || host->measure_text == NULL ||
        host->play == NULL || host->stop == NULL || host->set_voice == NULL ||
        host->audio_cursor == NULL || host->set_render_scale == NULL ||
        host->storage_write == NULL) {
        return LUDO_HOST_ERR_INCOMPLETE;
    }
    return LUDO_HOST_OK;
}

ludo_host_status ludo_constants_check(const ludo_host_constants *constants) {
    if (constants == NULL) {
        return LUDO_HOST_ERR_INCOMPLETE;
    }
    if (constants->screen == LUDO_HANDLE_NONE || constants->default_font == LUDO_HANDLE_NONE) {
        return LUDO_HOST_ERR_INCOMPLETE;
    }
    if (constants->sample_rate <= 0) {
        return LUDO_HOST_ERR_INCOMPLETE;
    }
    if ((constants->image_count > 0 && constants->images == NULL) ||
        (constants->storage_slot_count > 0 && constants->storage_slots == NULL)) {
        return LUDO_HOST_ERR_INCOMPLETE;
    }
    return LUDO_HOST_OK;
}

/* Linear scans. The binding tables hold one entry per declared asset in one
   program, and ch5 8.1 fixes every name at compile time, so the count is small
   and known before the program runs. */
ludo_image_id ludo_image_by_name(const ludo_host_constants *constants, const char *name) {
    if (constants == NULL || name == NULL) {
        return LUDO_HANDLE_NONE;
    }
    for (size_t i = 0; i < constants->image_count; i++) {
        if (strcmp(constants->images[i].name, name) == 0) {
            return constants->images[i].id;
        }
    }
    return LUDO_HANDLE_NONE;
}

ludo_storage_id ludo_storage_by_name(const ludo_host_constants *constants, const char *name) {
    if (constants == NULL || name == NULL) {
        return LUDO_HANDLE_NONE;
    }
    for (size_t i = 0; i < constants->storage_slot_count; i++) {
        if (strcmp(constants->storage_slots[i].name, name) == 0) {
            return constants->storage_slots[i].id;
        }
    }
    return LUDO_HANDLE_NONE;
}

/* ch6 7.11 and 7.12, and ch8 P15's vector. Clamp first, then round: the grid is
   defined inside the range, so rounding an out-of-range argument first would ask
   what the nearest sixteenth to -1.0 is, which the clause does not answer.

   0.5f then truncating is the round-half-away-from-zero the vector wants, and
   the value is non-negative by the time it is reached, so no sign case. */
float ludo_quantise_render_scale(float scale) {
    float steps;

    if (!(scale >= LUDO_RENDER_SCALE_MIN)) {
        /* Written as a negated >= so that a NaN argument lands here rather than
           travelling on: ch6 7.12 says clamp, never fault. */
        return LUDO_RENDER_SCALE_MIN;
    }
    if (scale >= LUDO_RENDER_SCALE_MAX) {
        return LUDO_RENDER_SCALE_MAX;
    }

    steps = (float)(int)(scale * (float)LUDO_RENDER_SCALE_STEPS + 0.5f);
    return steps / (float)LUDO_RENDER_SCALE_STEPS;
}

/* ch6 6.4.5: an absent device is present and idle. A player index outside the
   four slots is the same observation, so it reads idle rather than faulting. */
static const ludo_player_input *slot(const ludo_frame_input *input, int player) {
    static const ludo_player_input idle = {0};
    if (input == NULL || player < 0 || player >= LUDO_PLAYER_COUNT) {
        return &idle;
    }
    return &input->players[player];
}

/* The enum's underlying type is the implementation's choice, so the range check
   is written against int rather than against the enum. */
static bool held(uint16_t mask, ludo_button button) {
    int ordinal = (int)button;
    if (ordinal < 0 || ordinal >= (int)LUDO_BUTTON_COUNT) {
        return false;
    }
    return (mask & (uint16_t)(1u << (unsigned)ordinal)) != 0u;
}

ludo_vec2 ludo_input_direction_left(const ludo_frame_input *input, int player) {
    return slot(input, player)->direction_left;
}

ludo_vec2 ludo_input_direction_right(const ludo_frame_input *input, int player) {
    return slot(input, player)->direction_right;
}

bool ludo_input_button_down(const ludo_frame_input *input, ludo_button button, int player) {
    return held(slot(input, player)->down, button);
}

bool ludo_input_button_pressed(const ludo_frame_input *input, ludo_button button, int player) {
    return held(slot(input, player)->pressed, button);
}

bool ludo_input_button_released(const ludo_frame_input *input, ludo_button button, int player) {
    return held(slot(input, player)->released, button);
}
