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

static char *read_reference(size_t *length) {
    FILE *file = fopen("docs/spec/reference/reference.ludo", "rb");
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

/* Type-level nodes are not executed; #139's exempt list is where they go. */
static bool executable(ludo_ast_kind kind) {
    switch (kind) {
    case LUDO_AST_NAMED_TYPE:
    case LUDO_AST_OPTIONAL_TYPE:
    case LUDO_AST_VIEW_TYPE:
    case LUDO_AST_MUTABLE_TYPE:
    case LUDO_AST_FN_TYPE:
    case LUDO_AST_PARAMS:
    case LUDO_AST_RETURNS:
    case LUDO_AST_GENERIC_PARAMS:
    case LUDO_AST_GENERIC_PARAM:
    case LUDO_AST_ROOT:
    case LUDO_AST_STRUCT:
    case LUDO_AST_ENUM:
    case LUDO_AST_DISTINCT:
    case LUDO_AST_FIELD:
    case LUDO_AST_VARIANT:
    case LUDO_AST_TYPE_DECL:
        return false;
    case LUDO_AST_ERROR:
    case LUDO_AST_MODULE:
    case LUDO_AST_EXPLICIT:
    case LUDO_AST_LIBRARY:
    case LUDO_AST_USE:
    case LUDO_AST_ATTRIBUTE:
    case LUDO_AST_CONST:
    case LUDO_AST_FN:
    case LUDO_AST_EXTERN_FN:
    case LUDO_AST_EXTERN_ITEM:
    case LUDO_AST_PERSIST:
    case LUDO_AST_FRAME:
    case LUDO_AST_IMPL:
    case LUDO_AST_CONSTRAINTS:
    case LUDO_AST_PARAM:
    case LUDO_AST_INTERFACE:
    case LUDO_AST_GENERIC_ARGS:
    case LUDO_AST_POINTER_TYPE:
    case LUDO_AST_ARRAY_TYPE:
    case LUDO_AST_FALLIBLE_TYPE:
    case LUDO_AST_BLOCK:
    case LUDO_AST_LET:
    case LUDO_AST_BINDINGS:
    case LUDO_AST_ASSIGN:
    case LUDO_AST_COMPOUND_ASSIGN:
    case LUDO_AST_EXPR_STMT:
    case LUDO_AST_TAIL:
    case LUDO_AST_RETURN:
    case LUDO_AST_BREAK:
    case LUDO_AST_CONTINUE:
    case LUDO_AST_DEFER:
    case LUDO_AST_MODIFIER:
    case LUDO_AST_IF:
    case LUDO_AST_BRANCH:
    case LUDO_AST_WHILE:
    case LUDO_AST_FOR:
    case LUDO_AST_BIND_CONDITION:
    case LUDO_AST_UNSAFE:
    case LUDO_AST_MATCH:
    case LUDO_AST_ARM:
    case LUDO_AST_PATTERN_IDENT:
    case LUDO_AST_PATTERN_VARIANT:
    case LUDO_AST_PATTERN_STRUCT:
    case LUDO_AST_PATTERN_FIELD:
    case LUDO_AST_PATTERN_LITERAL:
    case LUDO_AST_PATTERN_DISCARD:
    case LUDO_AST_IDENT:
    case LUDO_AST_DISCARD:
    case LUDO_AST_LITERAL:
    case LUDO_AST_UNARY:
    case LUDO_AST_BINARY:
    case LUDO_AST_CAST:
    case LUDO_AST_MEMBER:
    case LUDO_AST_OPTIONAL_MEMBER:
    case LUDO_AST_CALL:
    case LUDO_AST_BRACKET:
    case LUDO_AST_MARK:
    case LUDO_AST_DEREF:
    case LUDO_AST_GROUP:
    case LUDO_AST_AGGREGATE:
    case LUDO_AST_NAMED_ITEM:
    case LUDO_AST_KEYED_ITEM:
    case LUDO_AST_RESCUE:
    case LUDO_AST_HANDLER:
        return true;
    }
    return true;
}

/* Every executable node under id was reached. A field default is the one
   expression under a type declaration, so it is walked rather than exempt. */
static size_t unreached(const ludo_ast *ast, const bool *executed, ludo_node_id id) {
    const ludo_ast_node *node = ludo_ast_get(ast, id);
    ludo_ast_kind kind = node->kind;
    if (kind == LUDO_AST_NAMED_TYPE || kind == LUDO_AST_OPTIONAL_TYPE ||
        kind == LUDO_AST_VIEW_TYPE || kind == LUDO_AST_MUTABLE_TYPE || kind == LUDO_AST_FN_TYPE ||
        kind == LUDO_AST_GENERIC_PARAMS) {
        return 0; /* a type expression, names included */
    }
    size_t missing = executable(kind) && !executed[id] ? 1u : 0u;
    if (missing != 0) {
        (void)fprintf(stderr, "unreached %s at byte %zu\n", ludo_ast_kind_name(node->kind),
                      node->span.offset);
    }
    for (ludo_node_id child = node->first_child; child != 0;
         child = ast->nodes[child].next_sibling) {
        missing += unreached(ast, executed, child);
    }
    return missing;
}

static void check_band1(void) {
    size_t length = 0;
    char *source = read_reference(&length);
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
        const bool *bits = ludo_interp_executed(in);
        for (size_t n = 0; n < file->ast.count && n < sizeof union_bits; n++) {
            union_bits[n] = union_bits[n] || bits[n];
        }
        ludo_interp_destroy(in);
    }

    for (size_t i = 0; i < sizeof band1_declarations / sizeof band1_declarations[0]; i++) {
        const char *name = band1_declarations[i];
        ludo_node_id decl = ludo_checked_lookup(checked, name, strlen(name));
        if (decl != 0) {
            LUDO_CHECK(unreached(&file->ast, union_bits, decl) == 0);
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
    check_band1();
})
