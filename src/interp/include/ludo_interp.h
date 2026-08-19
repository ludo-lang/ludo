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
 * What is deliberately absent, named so that an omission is never mistaken for
 * an oversight: any target other than the screen; the five mandated drawing
 * calls reference.ludo does not make (ch6 4.4.1 mandates ten); ch6 5.2.1's
 * stop_all, get and underruns, and ch6 5.4.4's Clip with its loop region and
 * direction; push and free_space (ch6 5.5); the pointer, keyboard and scroll
 * escape hatches (ch6 6.4.10 to 6.4.12); fullscreen (ch6 7); and storage read
 * (ch6 8.4). Each is an additive change to this header; none of them is on the
 * way to running reference.ludo, which is what this map is for.
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

/* ch6 4.2.2: a closed pair carried as ONE token, not two independent fields.
   crisp is no anti-aliasing and nearest sampling at every scale (4.2.3); smooth
   is anti-aliasing and linear sampling. crisp is the default, and is first here
   so that the default is the zero value. */
typedef enum { LUDO_STYLE_CRISP, LUDO_STYLE_SMOOTH } ludo_style;

/* ch6 5.4.3: closed. */
typedef enum {
    LUDO_WAVEFORM_SINE,
    LUDO_WAVEFORM_SQUARE,
    LUDO_WAVEFORM_TRIANGLE,
    LUDO_WAVEFORM_SAW,
    LUDO_WAVEFORM_NOISE
} ludo_waveform;

/* ch6 5.4.1: the amplitude envelope is ADSR -- attack time, decay time, sustain
   level, release time. ch6 5.3.7 makes the release the note-off, which is why a
   voice with one runs it rather than stop's ramp. */
typedef struct {
    float attack;  /* seconds */
    float decay;   /* seconds */
    float sustain; /* level, 0..1 */
    float release; /* seconds */
} ludo_adsr;

/* ch6 5.4.2: a two-number decay-to-target, explicitly NOT a second ADSR. The
   clause used to say three and list "start offset, time, linear curve", which
   is two numbers and a mandated shape; transcribing it here is what found that,
   and the clause was repaired rather than this struct grown (4b7f216). The
   curve is linear by mandate, so it is not a field. */
typedef struct {
    float start_offset; /* Hz, added to frequency at note-on */
    float time;         /* seconds to reach the target */
} ludo_pitch_envelope;

/* ch6 5.4.3: a voice carries frequency, amplitude, an amplitude envelope, a
   pitch envelope, and one-shot or loop. */
typedef struct {
    ludo_waveform waveform;
    float frequency;
    float amplitude;
    float pan;
    ludo_adsr amplitude_envelope;
    ludo_pitch_envelope pitch_envelope;
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
   something the spec says cannot happen.

   There is deliberately no device-unavailable status. ch6 8.11: IO failure --
   disk full, quota denied, storage evicted -- happens after the call returns
   and the program never learns of it, so there is no status channel for it to
   arrive on. An enumerator no entry point can return would pre-authorise a
   second failure mode the spec refuses. */
typedef enum {
    LUDO_HOST_OK,
    LUDO_HOST_ERR_OVERSIZE,  /* ch6 8.10: the one storage failure a program sees */
    LUDO_HOST_ERR_INCOMPLETE /* a vtable member or a constant is missing */
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

    /* $.graphics.set_canvas(<style>). ch5 9.3 makes it a top-level statement
       and 9.4 makes it immutable for the process's life -- a second call is
       rejected -- so it is negotiated once and never an entry point. ch6 7.15
       makes it the host's business: crisp stays nearest and smooth stays linear
       for how draws sample their textures, at whatever rate. */
    ludo_style style;

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
    uint64_t (*audio_cursor)(void *context);

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

/* ludo_host_check re-lists every member by hand, and so does any test that
   wires a complete host, so adding an entry point means edits the compiler
   cannot demand. This assertion is that demand: the struct is one context
   pointer plus this many function pointers, so a new member breaks the build
   here and the author is sent to the places that mirror the list. */
#define LUDO_HOST_ENTRY_POINT_COUNT 12
_Static_assert(sizeof(ludo_host) ==
                   sizeof(void *) + LUDO_HOST_ENTRY_POINT_COUNT * sizeof(void (*)(void)),
               "a ludo_host member was added or removed: update "
               "LUDO_HOST_ENTRY_POINT_COUNT, ludo_host_check, and any test host");

/* ---------------------------------------------------------------------- */
/* The stub host (#134)                                                    */
/* ---------------------------------------------------------------------- */

/* #96 bought one evaluator, two consumers: #49's hole-finder passes a host with
   no window, the runner passes platform/'s SDL3 one. #134 decides what that
   first host answers, because reference.ludo branches on what it says.

   It is called a STUB host and not a null one. Null names the behaviour #134
   rejected -- returning nothing, or refusing -- and it turned out that the
   reference program has exactly three host-answer branch points:
   $.input.button_pressed(.one) gating spawn, $.video.render_scale() > 0.5
   gating set_render_scale, and save1.write's oversize rescue tail. A scripted
   input trace is authoring machinery for three of them; refusing forfeits #49's
   "executed at least once" for most of ch6. So the answers are a plain table of
   constants, and the headless run varies the table.

   The tables are committed, hand-written and few, for the reason #131's fuzz
   corpus is committed: a table generated at run time is a thing nobody can read
   in a diff. Their union is the coverage -- no single run reaches every
   construct, and the oversize run deliberately reaches less than the others,
   because its rescue tail returns before the frame ends. */
typedef struct {
    const char *name; /* what a coverage report calls this run */

    /* What driver/ latches before the frame entry. set_render_scale writes back
       into this copy, so a second frame reads what the first one applied --
       which is what makes ch6 7.11's "reports the applied value, never the
       argument" observable rather than asserted. */
    ludo_frame_input frame;

    /* ch6 4.8.1: pure, and the program positions two sprites off its advance.
       Zero is a legal answer and a degenerate one, so a table that wants the
       drawing calls to differ says so. */
    ludo_text_metrics text_metrics;

    /* The cursor advances within a frame in a real host; here it advances by a
       fixed step per frame, which is enough for ch6 5.2.6's conversion to be
       exercised without modelling an audio engine -- the tier #32 ruled out,
       and it does not get back in through the test double. */
    uint64_t audio_cursor_start;
    uint64_t audio_cursor_step;

    /* ch6 8.10's oversize is the only failure the program can see, so it is a
       branch point like the other two rather than a special case. */
    ludo_host_status storage_write_status;
} ludo_stub_answers;

/* Everything zero: no button held, render_scale 0.0, degenerate metrics, writes
   succeed. Deterministic, and both conditional branches are dead -- which is
   exactly the reading #134 refused to adopt on its own. */
extern const ludo_stub_answers ludo_stub_answers_quiet;

/* The same program with every branch live: button one pressed, render_scale
   above the threshold, real metrics, a cursor that moves. */
extern const ludo_stub_answers ludo_stub_answers_active;

/* Active, except that storage_write reports oversize, so the rescue tail runs
   and everything after it does not. */
extern const ludo_stub_answers ludo_stub_answers_oversize;

#define LUDO_STUB_TABLE_COUNT 3
extern const ludo_stub_answers *const ludo_stub_tables[LUDO_STUB_TABLE_COUNT];

/* The stub's own state. Stateless answers, with the two counters a spec clause
   forces: handles are host-minted and 0 is never valid (so something must
   mint), and the cursor advances per frame (so something must count). Not a
   registry of live voices -- stop and set_voice are recorded and discarded. */
typedef struct {
    ludo_stub_answers answers;
    uint64_t frame; /* frames advanced since wiring */
    uint64_t calls; /* every vtable entry point, counted once */
    uint32_t next_handle;
} ludo_stub_state;

/* Wires state and host together from a table. The host's context is the state,
   so the two have the same lifetime and the caller keeps both.

   A no-op on a NULL state or host: nothing here can fail in a way the caller
   could act on, and this header has no errno (#131). */
void ludo_stub_host(ludo_stub_state *state, ludo_host *host, const ludo_stub_answers *answers);

/* The stub's creation-time configuration, which is NOT part of the varying
   table: a canvas size and a font id have no branch to flip, so putting them in
   the coverage matrix would enumerate combinations that do nothing.

   Bindings are left empty because they come from the program, not from the
   stub: ch5 8.3 has driver/ resolve every declared asset before top level, and
   only driver/ knows what was declared. The caller fills them in and runs
   ludo_constants_check. */
void ludo_stub_constants(ludo_host_constants *constants);

/* The latch driver/ hands the evaluator for the current frame. */
const ludo_frame_input *ludo_stub_frame(const ludo_stub_state *state);

/* Ends the frame. #49's headless run is a for loop around the evaluator (#133),
   and it needs more than one turn: one frame cannot tell a persist that
   survives from one that is re-initialised, and reference.ludo declares
   persist tick. */
void ludo_stub_next_frame(ludo_stub_state *state);

/* ---------------------------------------------------------------------- */
/* Reading the negotiated constants and the latch                          */
/* ---------------------------------------------------------------------- */

/* LUDO_HOST_ERR_INCOMPLETE if the screen or the default font is LUDO_HANDLE_NONE,
   if sample_rate is not positive, or if a binding count has no array. Run once,
   at wiring, beside ludo_host_check. */
ludo_host_status ludo_constants_check(const ludo_host_constants *constants);

/* LUDO_HANDLE_NONE when no binding carries that name, which is how a lookup
   reports absence without a status: ch5 8.4 already faulted at top level for a
   missing asset, so by here the name either resolves or the program never ran. */
ludo_image_id ludo_image_by_name(const ludo_host_constants *constants, const char *name);
ludo_storage_id ludo_storage_by_name(const ludo_host_constants *constants, const char *name);

/* The latch, read. These are what $.input.direction and the three button
   queries evaluate to; they take the snapshot rather than the host because ch6
   6.8.1 latched it before the frame entry ran.

   ch6 6.4.4 fixes four slots and 6.4.5 makes an absent device present and idle,
   so an out-of-range player reads as idle rather than faulting -- a slot that
   does not exist and a pad nobody plugged in are the same observation. */
ludo_vec2 ludo_input_direction_left(const ludo_frame_input *input, int player);
ludo_vec2 ludo_input_direction_right(const ludo_frame_input *input, int player);
bool ludo_input_button_down(const ludo_frame_input *input, ludo_button button, int player);
bool ludo_input_button_pressed(const ludo_frame_input *input, ludo_button button, int player);
bool ludo_input_button_released(const ludo_frame_input *input, ludo_button button, int player);

#endif /* LUDO_INTERP_H */
