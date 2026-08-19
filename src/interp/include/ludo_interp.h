/* The interpreter's umbrella header.
 *
 * interp/ declares the host interface and platform/ implements it (#96). This
 * file is that declaration (#133), and it is the whole of it: #130 fixes one
 * umbrella header per library, so the host surface does not get a second one.
 *
 * The shape, in five sentences:
 *
 *   1. driver/ owns the frame loop. Nothing here starts, presents or pumps;
 *      the evaluator is called once per frame and returns. A library that owns
 *      the loop owns the process, and ADR-0020 forbids that.
 *   2. Ambient per-frame reads are DATA, not entry points. ch6 6.8.1 mandates
 *      that input is latched once before the frame entry runs, so
 *      $.input.direction() answers out of ludo_frame_input rather than calling
 *      the host; the same holds for $.time.now and $.video.render_scale.
 *      Constants (canvas size, default font, debug image, sample rate, the
 *      resolved asset and storage ids) are negotiated once, at host creation.
 *   3. The vtable is flat. The five facades are a spec grouping (ch6 1.6), not
 *      an implementation one.
 *   4. Only POD mirrors cross. platform/ compiles without knowing that an
 *      interpreter value exists; the evaluator marshals at the boundary.
 *   5. Handles are uint32_t, minted by the host, and 0 is never valid.
 *
 * What is deliberately absent: any target other than the screen, the five
 * mandated drawing calls reference.ludo does not make (ch6 4.4.1 mandates ten),
 * clips, push/free_space, the pointer and keyboard escape hatches, fullscreen,
 * and storage read. Each is an additive change to this header; none of them is
 * on the way to running reference.ludo, which is what this map is for.
 */
#ifndef LUDO_INTERP_H
#define LUDO_INTERP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

const char *ludo_interp_version(void);

/* ---------------------------------------------------------------------- */
/* Handles                                                                 */
/* ---------------------------------------------------------------------- */

/* Host-minted, opaque to the evaluator, and comparable. ch5 8.3 has the runner
   resolve and decode every declared asset before top level runs, so by the time
   the evaluator exists these ids already do. */
typedef uint32_t ludo_image_id;
typedef uint32_t ludo_font_id;
typedef uint32_t ludo_storage_id;
typedef uint32_t ludo_voice_id;
typedef uint32_t ludo_target_id;

/* Reserved on every handle type. A stub host has a spelling before anything
   argues about what it should return (#134). */
#define LUDO_HANDLE_NONE 0u

/* ---------------------------------------------------------------------- */
/* Value mirrors (ch6 3, ch6 4.3, ch6 4.5)                                 */
/* ---------------------------------------------------------------------- */

typedef struct {
    float x;
    float y;
} ludo_vec2;

typedef struct {
    ludo_vec2 position;
    ludo_vec2 size;
} ludo_rect;

/* ch6 4.3.1: Paint is a sum of exactly two variants, and a pattern is not a
   third one -- it is Mapping.tile. */
typedef enum { LUDO_MAPPING_STRETCH, LUDO_MAPPING_TILE } ludo_mapping;

typedef enum { LUDO_PAINT_COLOR, LUDO_PAINT_TEXTURE } ludo_paint_kind;

typedef struct {
    ludo_image_id image;
    ludo_mapping mapping;
    ludo_vec2 offset;
} ludo_texture;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} ludo_color;

typedef struct {
    ludo_paint_kind kind;
    ludo_color color;     /* kind == LUDO_PAINT_COLOR */
    ludo_texture texture; /* kind == LUDO_PAINT_TEXTURE */
} ludo_paint;

/* ch6 4.4.8: closed at three, with no none and no opaque member. */
typedef enum { LUDO_BLEND_ALPHA, LUDO_BLEND_ADDITIVE, LUDO_BLEND_MULTIPLY } ludo_blend;

/* ch6 4.5.1: one descriptor type per call, never a shared one whose live
   fields depend on the receiving call. ch6 4.5.5: strokes repeat their fill
   descriptor's fields flat rather than nesting. */
typedef struct {
    ludo_vec2 position;
    ludo_vec2 size;
    ludo_paint paint;
    ludo_blend blend;
} ludo_rect_desc;

typedef struct {
    ludo_vec2 position;
    ludo_vec2 size;
    float thickness;
    ludo_paint paint;
    ludo_blend blend;
} ludo_rect_stroke_desc;

typedef struct {
    ludo_vec2 position;
    ludo_vec2 size;
    ludo_paint paint;
    ludo_blend blend;
} ludo_ellipse_desc;

typedef struct {
    ludo_image_id image;
    ludo_vec2 position;
    ludo_vec2 size;
    ludo_vec2 anchor;
    ludo_vec2 rotation_anchor;
    float angle; /* Radians */
    bool flip_horizontal;
    bool flip_vertical;
    ludo_paint paint;
    ludo_blend blend;
} ludo_sprite_desc;

/* ch6 4.8.7: text is []u8, not a string -- what a program draws is not
   necessarily a literal. The bytes are the evaluator's and are valid only for
   the duration of the call. */
typedef struct {
    const uint8_t *text;
    size_t text_len;
    ludo_font_id font;
    float font_size;
    ludo_vec2 position;
    ludo_vec2 anchor;
    ludo_paint paint;
    ludo_blend blend;
} ludo_text_desc;

/* ch6 4.8.3: exactly two quantities, in logical units, kerned (4.8.4). */
typedef struct {
    float advance;
    ludo_rect bounds;
} ludo_text_metrics;

/* ch6 5.4.3: closed. */
typedef enum {
    LUDO_WAVEFORM_SINE,
    LUDO_WAVEFORM_SQUARE,
    LUDO_WAVEFORM_TRIANGLE,
    LUDO_WAVEFORM_SAW,
    LUDO_WAVEFORM_NOISE
} ludo_waveform;

typedef struct {
    ludo_waveform waveform;
    float frequency;
    float amplitude;
    float pan;
    bool looping;
} ludo_voice_desc;

/* ch6 5.2.5: one call with a patch whose absent fields mean unchanged, which is
   what the has_ flags carry across a language with no optionals. */
typedef struct {
    bool has_frequency;
    float frequency;
    bool has_amplitude;
    float amplitude;
    bool has_pan;
    float pan;
} ludo_voice_patch;

/* ---------------------------------------------------------------------- */
/* Failure                                                                 */
/* ---------------------------------------------------------------------- */

/* Host failures only. Program errors are diagnostics, on frontend/'s channel
   (#130), and this header has no errno (#131). Every entry point that cannot
   fail returns void or its value directly, so a call site never checks
   something the spec says cannot happen. */
typedef enum {
    LUDO_HOST_OK,
    LUDO_HOST_ERR_OVERSIZE,    /* ch6 8.10: the one storage failure a program sees */
    LUDO_HOST_ERR_UNAVAILABLE, /* the device or slot is gone */
    LUDO_HOST_ERR_INCOMPLETE   /* ludo_host_check: a vtable member is NULL */
} ludo_host_status;

/* ---------------------------------------------------------------------- */
/* Negotiated once, at host creation                                       */
/* ---------------------------------------------------------------------- */

/* Reads with no per-frame variation. ch5 9.2 folds canvas_size at compile time
   and it is repeated here because platform/ has to letterbox against it; ch6
   4.8.6 identifies the default font by name, version and hash, which is the
   host's business and reaches the evaluator as an id.

   The declared assets and storage slots resolve here too: ch5 8.4 makes a
   missing asset a fault at top level, so driver/ resolves them before the host
   is built and startup failure never enters this header. */
typedef struct {
    const char *name; /* as written in the program's extern declaration */
    ludo_image_id id;
} ludo_image_binding;

typedef struct {
    const char *name;
    ludo_storage_id id;
} ludo_storage_binding;

typedef struct {
    ludo_vec2 canvas_size;
    ludo_font_id default_font;
    ludo_image_id debug_image;
    ludo_target_id screen;
    int32_t sample_rate; /* ch6 5.2.1: a device fact, fixed for the run */

    const ludo_image_binding *images;
    size_t image_count;
    const ludo_storage_binding *storage_slots;
    size_t storage_slot_count;
} ludo_host_constants;

/* ---------------------------------------------------------------------- */
/* Latched once per frame, before the frame entry runs                     */
/* ---------------------------------------------------------------------- */

/* ch6 6.2.1: twelve ordinals, fixed by prominence (6.2.3), never hardware
   nouns. Held as bit positions in the masks below -- which is not the integer
   index 6.2.5 bans from the facade, because nothing here is the facade. */
typedef enum {
    LUDO_BUTTON_ONE,
    LUDO_BUTTON_TWO,
    LUDO_BUTTON_THREE,
    LUDO_BUTTON_FOUR,
    LUDO_BUTTON_FIVE,
    LUDO_BUTTON_SIX,
    LUDO_BUTTON_SEVEN,
    LUDO_BUTTON_EIGHT,
    LUDO_BUTTON_NINE,
    LUDO_BUTTON_TEN,
    LUDO_BUTTON_ELEVEN,
    LUDO_BUTTON_TWELVE,
    LUDO_BUTTON_COUNT
} ludo_button;

/* ch6 6.4.4: slots, not devices, fixed at four. */
#define LUDO_PLAYER_COUNT 4

/* ch6 6.4.5, 6.4.6: an absent device is present and idle, so every slot is
   always readable and unplugging does not renumber. */
typedef struct {
    ludo_vec2 direction_left;
    ludo_vec2 direction_right;
    uint16_t down;     /* level */
    uint16_t pressed;  /* edge, exactly this frame */
    uint16_t released; /* edge, exactly this frame */
    bool connected;    /* 6.4.8: has produced input */
} ludo_player_input;

/* ch6 6.8.1: latched once by driver/ before the frame entry, so two reads in
   one frame cannot disagree. A function pointer for direction() would be a spec
   violation waiting to be written, which is why this is a struct.

   6.4.2 puts the edge state on the runner: driver/ computes pressed and
   released against the previous frame, and platform/ reports levels. */
typedef struct {
    ludo_player_input players[LUDO_PLAYER_COUNT];

    /* ch6 7.11: the quantised grid value the host actually applied, never the
       argument a previous frame passed to set_render_scale. */
    float render_scale;

    /* $.time.now, as Seconds. Carried at double width because the host measures a
       monotonic clock and the evaluator narrows when it mints the value; a float
       seconds counter loses its frame resolution after a few hours of uptime. */
    double now_seconds;
} ludo_frame_input;

/* ---------------------------------------------------------------------- */
/* The host vtable                                                         */
/* ---------------------------------------------------------------------- */

/* Flat, and every member is required: ludo_host_check rejects a partial vtable
   at wiring time rather than mid-frame, so "the empty host" is something a
   caller states rather than something it gets by omitting a member (#134). */
typedef struct ludo_host {
    void *context; /* platform/'s own state; the evaluator never reads it */

    /* $.graphics -- the five calls reference.ludo makes. ch6 4.2.6: the target is
       a parameter and there is no $.graphics.screen() to fetch it. */
    void (*fill_rect)(void *context, ludo_target_id target, const ludo_rect_desc *desc);
    void (*fill_ellipse)(void *context, ludo_target_id target, const ludo_ellipse_desc *desc);
    void (*stroke_rect)(void *context, ludo_target_id target, const ludo_rect_stroke_desc *desc);
    void (*fill_text)(void *context, ludo_target_id target, const ludo_text_desc *desc);
    void (*fill_sprite)(void *context, ludo_target_id target, const ludo_sprite_desc *desc);

    /* ch6 4.8.1: pure, takes no target, and takes the same descriptor fill_text
       does. It needs real font metrics, so it is a query answered synchronously
       with a value the program branches on -- one of the three the latch could
       not remove. */
    ludo_text_metrics (*measure_text)(void *context, const ludo_text_desc *desc);

    /* $.audio. ch6 5.1.3: play returns a handle and never absence -- there is no
       voice cap, no stealing policy and no exhaustion to report. */
    ludo_voice_id (*play)(void *context, const ludo_voice_desc *desc);
    void (*stop)(void *context, ludo_voice_id voice);
    void (*set_voice)(void *context, ludo_voice_id voice, const ludo_voice_patch *patch);

    /* ch6 5.2.6: SampleFrames, and converting it takes the device rate as an
       argument so a stale-rate division is unspellable. The second query the
       latch could not remove: it advances within a frame. */
    uint64_t (*cursor)(void *context);

    /* $.video. The getter is in ludo_frame_input; only the write is here. */
    void (*set_render_scale)(void *context, float scale);

    /* $.storage. ch6 8.8: never blocks and reports no completion. ch6 8.10:
       oversize is the one failure the program can see, which is why this is the
       only entry point that returns a status. */
    ludo_host_status (*storage_write)(void *context, ludo_storage_id slot, const uint8_t *bytes,
                                      size_t len);
} ludo_host;

/* LUDO_HOST_ERR_INCOMPLETE if any member is NULL, LUDO_HOST_OK otherwise. The
   caller runs this once, at wiring. */
ludo_host_status ludo_host_check(const ludo_host *host);

/* The stub host: fully populated, explicitly named, and reached on purpose.
   #134 decides what its members return and defines it; declaring it here is
   what keeps that a live decision rather than a side effect of a NULL member. */
void ludo_null_host(ludo_host *out);

#endif /* LUDO_INTERP_H */
