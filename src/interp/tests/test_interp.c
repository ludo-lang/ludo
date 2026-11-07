#include "ludo_interp.h"
#include "test.h"

#include <stdio.h>
#include <stdlib.h>
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

/* ---- Band 1 (#143): reference.ludo's chapter-1 surface checks and runs ---- */

/* Lines 66-202, by declaration. The checker must call each of them CHECKED --
   never PENDING -- and the evaluator must reach every executable node in them.
   The program itself calls none of the functions (they have no call site after
   line 202), so this harness is what runs them; #139's report will say so. */
static const char *const band1_declarations[] = {
    "MAX_ROCKS", "save1",  "rock",  "Rock",      "Shape",       "RockId", "FieldError",  "overlaps",
    "advance",   "extent", "first", "radius_of", "lead_radius", "is_big", "count_where",
};

static char *read_text(const char *path, size_t *length) {
    FILE *file = fopen(path, "rb");
    char *text = NULL;
    if (file == NULL) {
        return NULL;
    }
    if (fseek(file, 0, SEEK_END) == 0) {
        long size = ftell(file);
        if (size > 0 && fseek(file, 0, SEEK_SET) == 0) {
            text = malloc((size_t)size); /* ludo-allow-malloc */
            if (text != NULL && fread(text, 1, (size_t)size, file) != (size_t)size) {
                free(text); /* ludo-allow-malloc */
                text = NULL;
            }
            *length = (size_t)size;
        }
    }
    (void)fclose(file);
    return text;
}

static ludo_value rock_at(ludo_interp *in, float x, float y, float radius, bool defaulted) {
    static const char *const names[] = {"pos", "vel", "radius"};
    ludo_value fields[3];
    ludo_value out = {0};
    fields[0] = ludo_value_vec2(in, x, y);
    fields[1] = ludo_value_vec2(in, 0.5f, -1.0f);
    fields[2] = ludo_value_f32(radius);
    LUDO_CHECK(ludo_interp_make_struct(in, "Rock", names, fields, defaulted ? 2u : 3u, &out) ==
               LUDO_INTERP_OK);
    return out;
}

static float as_f32(const ludo_value *value) {
    return value->kind == LUDO_VALUE_FLOAT ? (float)value->as.number : -1.0f;
}

static void run_band1(ludo_interp *in) {
    ludo_value result = {0};
    ludo_value args[2];

    /* Field defaults run in the evaluator (ch1 5.3): radius is 4.0. */
    ludo_value a = rock_at(in, 0.0f, 0.0f, 0.0f, true);
    ludo_value near = rock_at(in, 3.0f, 4.0f, 0.0f, true);
    ludo_value far = rock_at(in, 30.0f, 0.0f, 0.0f, true);
    LUDO_CHECK(as_f32(&a.items[2]) == 4.0f);

    args[0] = a;
    args[1] = near;
    LUDO_CHECK(ludo_interp_call(in, "overlaps", args, 2, &result) == LUDO_INTERP_OK);
    LUDO_CHECK(result.kind == LUDO_VALUE_BOOL && result.as.boolean);
    args[1] = far;
    LUDO_CHECK(ludo_interp_call(in, "overlaps", args, 2, &result) == LUDO_INTERP_OK);
    LUDO_CHECK(result.kind == LUDO_VALUE_BOOL && !result.as.boolean);

    /* A ! parameter writes the caller's place, through the mark. */
    ludo_value moving = rock_at(in, 1.0f, 2.0f, 0.0f, true);
    args[0] = ludo_value_ref(&moving);
    LUDO_CHECK(ludo_interp_call(in, "advance", args, 1, &result) == LUDO_INTERP_OK);
    LUDO_CHECK(as_f32(&moving.items[0].items[0]) == 1.5f);
    LUDO_CHECK(as_f32(&moving.items[0].items[1]) == 1.0f);
    /* Passing it by value is refused rather than silently copied. */
    args[0] = moving;
    LUDO_CHECK(ludo_interp_call(in, "advance", args, 1, &result) == LUDO_INTERP_BAD_ARGUMENT);

    ludo_value shape;
    LUDO_CHECK(ludo_interp_make_variant(in, "Shape", "point", NULL, 0, &shape) == LUDO_INTERP_OK);
    LUDO_CHECK(ludo_interp_call(in, "extent", &shape, 1, &result) == LUDO_INTERP_OK);
    LUDO_CHECK(as_f32(&result) == 0.0f);
    args[0] = ludo_value_f32(3.0f);
    LUDO_CHECK(ludo_interp_make_variant(in, "Shape", "circle", args, 1, &shape) == LUDO_INTERP_OK);
    LUDO_CHECK(ludo_interp_call(in, "extent", &shape, 1, &result) == LUDO_INTERP_OK);
    LUDO_CHECK(as_f32(&result) == 3.0f);
    args[0] = ludo_value_vec2(in, 1.0f, 1.0f);
    args[1] = ludo_value_vec2(in, 4.0f, 2.0f);
    LUDO_CHECK(ludo_interp_make_variant(in, "Shape", "box", args, 2, &shape) == LUDO_INTERP_OK);
    LUDO_CHECK(ludo_interp_call(in, "extent", &shape, 1, &result) == LUDO_INTERP_OK);
    LUDO_CHECK(as_f32(&result) == 3.0f);

    /* first[T] through both of its callers: absent, then present. */
    ludo_value rocks[3];
    ludo_value view;
    LUDO_CHECK(ludo_interp_make_view(in, NULL, 0, &view) == LUDO_INTERP_OK);
    LUDO_CHECK(ludo_interp_call(in, "radius_of", &view, 1, &result) == LUDO_INTERP_OK);
    LUDO_CHECK(as_f32(&result) == 0.0f);
    LUDO_CHECK(ludo_interp_call(in, "lead_radius", &view, 1, &result) == LUDO_INTERP_OK);
    LUDO_CHECK(as_f32(&result) == 0.0f);
    rocks[0] = rock_at(in, 0.0f, 0.0f, 9.0f, false);
    rocks[1] = rock_at(in, 0.0f, 0.0f, 4.0f, false);
    rocks[2] = rock_at(in, 0.0f, 0.0f, 10.0f, false);
    LUDO_CHECK(ludo_interp_make_view(in, rocks, 3, &view) == LUDO_INTERP_OK);
    LUDO_CHECK(ludo_interp_call(in, "radius_of", &view, 1, &result) == LUDO_INTERP_OK);
    LUDO_CHECK(as_f32(&result) == 9.0f);
    LUDO_CHECK(ludo_interp_call(in, "lead_radius", &view, 1, &result) == LUDO_INTERP_OK);
    LUDO_CHECK(as_f32(&result) == 9.0f);

    /* A function value passed by name (ch2 7.1, 7.3), and n += 1 if keep(x)
       both taken and not. */
    args[0] = view;
    LUDO_CHECK(ludo_interp_function(in, "is_big", &args[1]) == LUDO_INTERP_OK);
    LUDO_CHECK(ludo_interp_call(in, "count_where", args, 2, &result) == LUDO_INTERP_OK);
    LUDO_CHECK(result.kind == LUDO_VALUE_INT && result.as.bits == 2u);

    LUDO_CHECK(ludo_interp_global(in, "MAX_ROCKS", &result) == LUDO_INTERP_OK);
    LUDO_CHECK(result.kind == LUDO_VALUE_INT && result.as.bits == 64u &&
               result.type == LUDO_TYPE_ID_OF(LUDO_TYPE_USIZE));
    LUDO_CHECK(ludo_interp_global(in, "rock", &result) == LUDO_INTERP_OK);
    LUDO_CHECK(result.kind == LUDO_VALUE_HANDLE && result.as.handle == 7u);
    LUDO_CHECK(ludo_interp_global(in, "save1", &result) == LUDO_INTERP_OK);
    LUDO_CHECK(result.kind == LUDO_VALUE_HANDLE && result.as.handle == 3u);

    /* A later band's declaration is refused, not guessed at. */
    LUDO_CHECK(ludo_interp_call(in, "best", &view, 1, &result) == LUDO_INTERP_UNCHECKED);
}

/* The exempt list the report reads (#139), so the band tests and the report
   cannot disagree about which nodes count. */
static bool load_exempt(ludo_coverage_exempt *exempt) {
    size_t length = 0;
    size_t bad_line = 0;
    char *text = read_text("src/interp/coverage.exempt", &length);
    bool ok = text != NULL && ludo_coverage_exempt_parse(text, length, exempt, &bad_line);
    free(text); /* ludo-allow-malloc */
    return ok;
}

static void print_region(void *context, ludo_node_id id, size_t unreached) {
    const ludo_ast *ast = context;
    (void)fprintf(stderr, "unreached %s at byte %zu (%zu nodes)\n",
                  ludo_ast_kind_name(ast->nodes[id].kind), ast->nodes[id].span.offset, unreached);
}

static void check_exempt_parse(void) {
    ludo_coverage_exempt exempt;
    size_t bad = 0;
    static const char good[] = "# comment\n\nsubtree named-type  a type\n  node struct shape\r\n";
    LUDO_CHECK(ludo_coverage_exempt_parse(good, sizeof good - 1, &exempt, &bad));
    LUDO_CHECK(exempt.kinds[LUDO_AST_NAMED_TYPE] == LUDO_EXEMPT_SUBTREE);
    LUDO_CHECK(exempt.kinds[LUDO_AST_STRUCT] == LUDO_EXEMPT_NODE);
    LUDO_CHECK(exempt.kinds[LUDO_AST_FN] == LUDO_EXEMPT_NONE);

    /* Each of these is refused, and names its line. */
    static const char *const bad_lists[] = {
        "node struct\n",           /* no reason */
        "node structure a typo\n", /* no such kind */
        "skip struct a mode that is not one\n",
        "node struct one\nnode struct two\n", /* two reasons, one stale */
    };
    static const size_t bad_lines[] = {1, 1, 1, 2};
    for (size_t i = 0; i < sizeof bad_lists / sizeof bad_lists[0]; i++) {
        LUDO_CHECK(!ludo_coverage_exempt_parse(bad_lists[i], strlen(bad_lists[i]), &exempt, &bad));
        LUDO_CHECK(bad == bad_lines[i]);
    }

    /* The committed list parses. */
    LUDO_CHECK(load_exempt(&exempt));
}

typedef struct regions {
    ludo_node_id ids[4];
    size_t counts[4];
    size_t count;
} regions;

static void collect_region(void *context, ludo_node_id id, size_t unreached) {
    regions *r = context;
    if (r->count < 4) {
        r->ids[r->count] = id;
        r->counts[r->count] = unreached;
    }
    r->count++;
}

static void check_regions(void) {
    /* module { fn a: named-type { ident }   fn b { block { ident } } } */
    static const ludo_ast_node nodes[] = {
        {0},
        {.kind = LUDO_AST_MODULE, .first_child = 2},
        {.kind = LUDO_AST_FN, .first_child = 3, .next_sibling = 5},
        {.kind = LUDO_AST_NAMED_TYPE, .first_child = 4},
        {.kind = LUDO_AST_IDENT},
        {.kind = LUDO_AST_FN, .first_child = 6},
        {.kind = LUDO_AST_BLOCK, .first_child = 7},
        {.kind = LUDO_AST_IDENT},
    };
    const ludo_ast ast = {nodes, sizeof nodes / sizeof nodes[0], 1};
    ludo_coverage_exempt exempt = {0};
    ludo_node_id stack[sizeof nodes / sizeof nodes[0]];
    exempt.kinds[LUDO_AST_MODULE] = LUDO_EXEMPT_NODE;
    exempt.kinds[LUDO_AST_NAMED_TYPE] = LUDO_EXEMPT_SUBTREE;

    /* fn a ran; its type never runs and is exempt. fn b never ran, and is one
       region of three nodes, not three lines. */
    bool executed[8] = {false, false, true, false, false, false, false, false};
    regions r = {0};
    LUDO_CHECK(ludo_coverage_unreached(&ast, executed, &exempt, 1, stack, collect_region, &r) ==
               3u);
    LUDO_CHECK(r.count == 1 && r.ids[0] == 5 && r.counts[0] == 3);

    /* A reached node inside an unreached region is not counted, and does not
       split the region. */
    executed[6] = true;
    r = (regions){0};
    LUDO_CHECK(ludo_coverage_unreached(&ast, executed, &exempt, 1, stack, collect_region, &r) ==
               2u);
    LUDO_CHECK(r.count == 1 && r.ids[0] == 5 && r.counts[0] == 2);

    /* Everything ran: no region. A walk rooted at a node does not wander into
       its siblings. */
    memset(executed, 1, sizeof executed);
    r = (regions){0};
    LUDO_CHECK(ludo_coverage_unreached(&ast, executed, &exempt, 1, stack, collect_region, &r) ==
               0u);
    LUDO_CHECK(r.count == 0);
    executed[5] = false;
    LUDO_CHECK(ludo_coverage_unreached(&ast, executed, &exempt, 2, stack, NULL, NULL) == 0u);
}

static void check_band1(void) {
    size_t length = 0;
    char *source = read_text("docs/spec/reference/reference.ludo", &length);
    ludo_session *session = NULL;
    const ludo_parsed_file *file = NULL;
    const ludo_checked_file *checked = NULL;
    bool union_bits[1u << 16] = {0};
    LUDO_CHECK(source != NULL);
    if (source == NULL) {
        return;
    }
    LUDO_CHECK(ludo_session_create(&session) == LUDO_FRONTEND_OK);
    LUDO_CHECK(ludo_parse_file(session, "reference.ludo", source, length, &file) ==
               LUDO_FRONTEND_OK);
    LUDO_CHECK(ludo_check_file(session, file, &checked) == LUDO_FRONTEND_OK);
    LUDO_CHECK(checked->diagnostic_count == 0);
    LUDO_CHECK(file->ast.count <= sizeof union_bits / sizeof union_bits[0]);

    for (size_t i = 0; i < sizeof band1_declarations / sizeof band1_declarations[0]; i++) {
        const char *name = band1_declarations[i];
        ludo_node_id decl = ludo_checked_lookup(checked, name, strlen(name));
        LUDO_CHECK(decl != 0 && checked->facts[decl].state == LUDO_CHECK_CHECKED);
    }

    /* ch2 5.1: one instance of first, keyed by (first, Rock). */
    ludo_node_id first_decl = ludo_checked_lookup(checked, "first", 5);
    ludo_node_id rock_decl = ludo_checked_lookup(checked, "Rock", 4);
    LUDO_CHECK(checked->instance_count == 1);
    LUDO_CHECK(checked->instance_count == 1 && checked->instances[0].decl == first_decl &&
               checked->instances[0].arg_count == 1 &&
               checked->instances[0].args[0] == checked->facts[rock_decl].type);

    /* #134: every table, and the union of their bits is the coverage. */
    for (size_t t = 0; t < LUDO_STUB_TABLE_COUNT; t++) {
        static const ludo_image_binding images[] = {{"rock", 7u}};
        static const ludo_storage_binding slots[] = {{"save1", 3u}};
        ludo_stub_state state;
        ludo_host host;
        ludo_host_constants constants;
        ludo_interp *in = NULL;
        ludo_stub_host(&state, &host, ludo_stub_tables[t]);
        ludo_stub_constants(&constants);
        constants.images = images;
        constants.image_count = 1u;
        constants.storage_slots = slots;
        constants.storage_slot_count = 1u;
        LUDO_CHECK(ludo_interp_create(checked, &host, &constants, &in) == LUDO_INTERP_OK);
        if (in == NULL) {
            continue;
        }
        run_band1(in);
        /* The frame is band 6's (#148): the headless run's loop calls it and
           is refused, rather than running an unchecked body. */
        LUDO_CHECK(ludo_interp_frame(in, ludo_stub_frame(&state)) == LUDO_INTERP_UNCHECKED);
        const bool *bits = ludo_interp_executed(in);
        for (size_t n = 0; n < file->ast.count && n < sizeof union_bits; n++) {
            union_bits[n] = union_bits[n] || bits[n];
        }
        ludo_interp_destroy(in);
    }

    ludo_coverage_exempt exempt;
    static ludo_node_id stack[1u << 16];
    LUDO_CHECK(load_exempt(&exempt));
    for (size_t i = 0; i < sizeof band1_declarations / sizeof band1_declarations[0]; i++) {
        const char *name = band1_declarations[i];
        ludo_node_id decl = ludo_checked_lookup(checked, name, strlen(name));
        if (decl != 0) {
            LUDO_CHECK(ludo_coverage_unreached(&file->ast, union_bits, &exempt, decl, stack,
                                               print_region, (void *)&file->ast) == 0);
        }
    }

    /* ch5 8.4: a declared asset the host cannot resolve faults at top level. */
    {
        ludo_stub_state state;
        ludo_host host;
        ludo_host_constants constants;
        ludo_interp *in = NULL;
        ludo_stub_host(&state, &host, &ludo_stub_answers_quiet);
        ludo_stub_constants(&constants);
        LUDO_CHECK(ludo_interp_create(checked, &host, &constants, &in) == LUDO_INTERP_FAULT);
        LUDO_CHECK(in != NULL && ludo_interp_fault(in) == LUDO_FAULT_MISSING_ASSET);
        ludo_interp_destroy(in);
    }

    ludo_session_destroy(session);
    free(source); /* ludo-allow-malloc */
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
    check_exempt_parse();
    check_regions();
    check_band1();
})
