/* The tree-walking evaluator (#143 is band 1).

   It reads the checker's facts and never re-derives them: what a name refers
   to, which field a member selects and which variant a pattern names are all
   answers the checker already wrote down. It runs only CHECKED declarations.

   Memory is one arena for the interpreter's life. Aggregates are copied when
   a value is read out of a place, so a value never aliases a place; a view
   aliases what it reads, which is what a view is (ch3 6). */
#include "frontend/arena.h"
#include "ludo_interp.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

typedef struct slot {
    ludo_node_id def;
    ludo_value value;
} slot;

typedef struct frame {
    slot *slots;
    size_t count;
    size_t capacity;
} frame;

typedef enum flow { FLOW_NORMAL, FLOW_RETURN, FLOW_BREAK, FLOW_CONTINUE, FLOW_STOP } flow;

struct ludo_interp {
    ludo_arena arena;
    const ludo_checked_file *program;
    const ludo_parsed_file *file;
    const ludo_ast *ast;
    const ludo_node_fact *facts;
    const ludo_host *host;
    const ludo_host_constants *constants;
    bool *executed;
    ludo_value *globals; /* per declaration node: a constant's or extern's value */
    ludo_value returned;
    ludo_type_id result_type;  /* the running function's declared result */
    ludo_interp_status status; /* why the last FLOW_STOP stopped */
    ludo_fault fault;
};

/* ---- Small helpers --------------------------------------------------------- */

static const ludo_ast_node *at(const ludo_interp *in, ludo_node_id id) {
    return &in->ast->nodes[id];
}

static ludo_ast_kind kind_of(const ludo_interp *in, ludo_node_id id) {
    return id == 0 ? LUDO_AST_ERROR : at(in, id)->kind;
}

static ludo_node_id first(const ludo_interp *in, ludo_node_id id) {
    return id == 0 ? 0 : at(in, id)->first_child;
}

static ludo_node_id next(const ludo_interp *in, ludo_node_id id) {
    return at(in, id)->next_sibling;
}

static ludo_node_id child_of_kind(const ludo_interp *in, ludo_node_id id, ludo_ast_kind kind) {
    for (ludo_node_id it = first(in, id); it != 0; it = next(in, it)) {
        if (kind_of(in, it) == kind) {
            return it;
        }
    }
    return 0;
}

static const ludo_type *type_at(const ludo_interp *in, ludo_type_id id) {
    return &in->program->types[id];
}

static ludo_type_kind tkind(const ludo_interp *in, ludo_type_id id) {
    return in->program->types[id].kind;
}

static ludo_token_kind token_kind(const ludo_interp *in, ludo_node_id id) {
    size_t token = at(in, id)->token;
    return token < in->file->token_count ? in->file->tokens[token].kind : LUDO_TOK_EOF;
}

static flow stop(ludo_interp *in, ludo_interp_status status) {
    in->status = status;
    return FLOW_STOP;
}

static flow fault(ludo_interp *in, ludo_fault why) {
    in->fault = why;
    return stop(in, LUDO_INTERP_FAULT);
}

static ludo_value *items(ludo_interp *in, size_t count) {
    if (count == 0) {
        return NULL;
    }
    if (count > SIZE_MAX / sizeof(ludo_value)) {
        return NULL;
    }
    ludo_value *made = ludo_arena_alloc(&in->arena, count * sizeof *made);
    if (made != NULL) {
        memset(made, 0, count * sizeof *made);
    }
    return made;
}

/* Reading a value out of a place copies what it owns, so writing the copy
   never writes the place. A view's items are borrowed and stay shared. */
static bool copy(ludo_interp *in, const ludo_value *from, ludo_value *to) {
    *to = *from;
    if (from->kind != LUDO_VALUE_AGGREGATE || from->count == 0) {
        return true;
    }
    to->items = items(in, from->count);
    if (to->items == NULL) {
        return false;
    }
    for (size_t i = 0; i < from->count; i++) {
        if (!copy(in, &from->items[i], &to->items[i])) {
            return false;
        }
    }
    return true;
}

static ludo_value unit(void) { return (ludo_value){.kind = LUDO_VALUE_UNIT}; }

static ludo_value boolean(bool value) {
    return (ludo_value){
        .kind = LUDO_VALUE_BOOL, .type = LUDO_TYPE_ID_OF(LUDO_TYPE_BOOL), .as.boolean = value};
}

static ludo_value number(ludo_type_id type, double value) {
    return (ludo_value){.kind = LUDO_VALUE_FLOAT, .type = type, .as.number = value};
}

static ludo_value integer(ludo_type_id type, uint64_t bits) {
    return (ludo_value){.kind = LUDO_VALUE_INT, .type = type, .as.bits = bits};
}

ludo_value ludo_value_f32(float value) { return number(LUDO_TYPE_ID_OF(LUDO_TYPE_F32), value); }

ludo_value ludo_value_ref(ludo_value *place) {
    return (ludo_value){.kind = LUDO_VALUE_REF, .type = place->type, .as.ref = place};
}

static bool make_vec2(ludo_interp *in, float x, float y, ludo_value *out) {
    ludo_value *parts = items(in, 2);
    if (parts == NULL) {
        return false;
    }
    parts[0] = ludo_value_f32(x);
    parts[1] = ludo_value_f32(y);
    *out = (ludo_value){.kind = LUDO_VALUE_AGGREGATE,
                        .type = LUDO_TYPE_ID_OF(LUDO_TYPE_VEC2),
                        .count = 2,
                        .items = parts};
    return true;
}

ludo_value ludo_value_vec2(ludo_interp *interp, float x, float y) {
    ludo_value out = unit();
    (void)make_vec2(interp, x, y, &out);
    return out;
}

/* ---- Frames ----------------------------------------------------------------- */

static slot *find(frame *f, ludo_node_id def) {
    for (size_t i = f->count; i != 0; i--) {
        if (f->slots[i - 1].def == def) {
            return &f->slots[i - 1];
        }
    }
    return NULL;
}

/* Node ids are unique per file, so a definition keys its slot directly; a
   loop that binds the same node again overwrites it. */
static bool bind(ludo_interp *in, frame *f, ludo_node_id def, ludo_value value) {
    in->executed[def] = true;
    slot *existing = find(f, def);
    if (existing != NULL) {
        existing->value = value;
        return true;
    }
    if (f->count == f->capacity) {
        size_t capacity = f->capacity == 0 ? 16 : f->capacity * 2;
        slot *grown = ludo_arena_alloc(&in->arena, capacity * sizeof *grown);
        if (grown == NULL) {
            return false;
        }
        if (f->count != 0) {
            memcpy(grown, f->slots, f->count * sizeof *grown);
        }
        f->slots = grown;
        f->capacity = capacity;
    }
    f->slots[f->count++] = (slot){def, value};
    return true;
}

/* ---- Arithmetic --------------------------------------------------------------
   ch2 8.3: integer overflow is checked always, in every layer; ch2 2.5: float
   arithmetic is evaluated exactly as written, at the type's own width. */

static unsigned width(ludo_type_kind kind) {
    static const unsigned bits[] = {8, 16, 32, 64, 8, 16, 32, 64, 64};
    return bits[(size_t)kind - (size_t)LUDO_TYPE_I8];
}

static bool is_signed(ludo_type_kind kind) { return kind >= LUDO_TYPE_I8 && kind <= LUDO_TYPE_I64; }

static bool fits(ludo_type_kind kind, int64_t value_signed, uint64_t value_unsigned) {
    unsigned w = width(kind);
    if (is_signed(kind)) {
        if (w == 64) {
            return true;
        }
        int64_t limit = (int64_t)1 << (w - 1);
        return value_signed >= -limit && value_signed < limit;
    }
    return w == 64 || value_unsigned < ((uint64_t)1 << w);
}

static flow int_op(ludo_interp *in, ludo_token_kind op, ludo_type_id type, uint64_t a, uint64_t b,
                   ludo_value *out) {
    ludo_type_kind kind = tkind(in, type);
    bool overflow = false;
    if (is_signed(kind)) {
        int64_t x = (int64_t)a;
        int64_t y = (int64_t)b;
        int64_t r = 0;
        if ((op == LUDO_TOK_SLASH || op == LUDO_TOK_PERCENT) && y == 0) {
            return fault(in, LUDO_FAULT_DIVISION_BY_ZERO);
        }
        if (op == LUDO_TOK_PLUS) {
            overflow = __builtin_add_overflow(x, y, &r);
        } else if (op == LUDO_TOK_MINUS) {
            overflow = __builtin_sub_overflow(x, y, &r);
        } else if (op == LUDO_TOK_STAR) {
            overflow = __builtin_mul_overflow(x, y, &r);
        } else if (op == LUDO_TOK_SLASH || op == LUDO_TOK_PERCENT) {
            overflow = x == INT64_MIN && y == -1;
            r = overflow ? 0 : (op == LUDO_TOK_SLASH ? x / y : x % y);
        } else if (op == LUDO_TOK_AMP) {
            r = x & y;
        } else if (op == LUDO_TOK_PIPE) {
            r = x | y;
        } else if (op == LUDO_TOK_TILDE) {
            r = x ^ y;
        } else {
            /* Shifts: a count outside the width is a fault, never a wrap. */
            overflow = y < 0 || y >= (int64_t)width(kind);
            r = overflow ? 0
                         : (op == LUDO_TOK_SHL ? (int64_t)((uint64_t)x << (unsigned)y)
                                               : x >> (unsigned)y);
        }
        if (overflow || !fits(kind, r, 0)) {
            return fault(in, LUDO_FAULT_OVERFLOW);
        }
        *out = integer(type, (uint64_t)r);
        return FLOW_NORMAL;
    }
    uint64_t r = 0;
    if ((op == LUDO_TOK_SLASH || op == LUDO_TOK_PERCENT) && b == 0) {
        return fault(in, LUDO_FAULT_DIVISION_BY_ZERO);
    }
    if (op == LUDO_TOK_PLUS) {
        overflow = __builtin_add_overflow(a, b, &r);
    } else if (op == LUDO_TOK_MINUS) {
        overflow = __builtin_sub_overflow(a, b, &r);
    } else if (op == LUDO_TOK_STAR) {
        overflow = __builtin_mul_overflow(a, b, &r);
    } else if (op == LUDO_TOK_SLASH) {
        r = a / b;
    } else if (op == LUDO_TOK_PERCENT) {
        r = a % b;
    } else if (op == LUDO_TOK_AMP) {
        r = a & b;
    } else if (op == LUDO_TOK_PIPE) {
        r = a | b;
    } else if (op == LUDO_TOK_TILDE) {
        r = a ^ b;
    } else {
        overflow = b >= width(kind);
        r = overflow ? 0 : (op == LUDO_TOK_SHL ? a << b : a >> b);
    }
    if (overflow || !fits(kind, 0, r)) {
        return fault(in, LUDO_FAULT_OVERFLOW);
    }
    *out = integer(type, r);
    return FLOW_NORMAL;
}

static double float_op(ludo_token_kind op, bool narrow, double a, double b) {
    if (narrow) {
        float x = (float)a;
        float y = (float)b;
        float r = op == LUDO_TOK_PLUS    ? x + y
                  : op == LUDO_TOK_MINUS ? x - y
                  : op == LUDO_TOK_STAR  ? x * y
                  : op == LUDO_TOK_SLASH ? x / y
                                         : fmodf(x, y);
        return r;
    }
    return op == LUDO_TOK_PLUS    ? a + b
           : op == LUDO_TOK_MINUS ? a - b
           : op == LUDO_TOK_STAR  ? a * b
           : op == LUDO_TOK_SLASH ? a / b
                                  : fmod(a, b);
}

/* The representation of a value of type, for arithmetic: a distinct type
   computes as its base (ch2 3.1, 2.9.5). */
static ludo_type_id base_of(const ludo_interp *in, ludo_type_id type) {
    while (tkind(in, type) == LUDO_TYPE_DISTINCT) {
        type = type_at(in, type)->inner;
    }
    return type;
}

static flow arithmetic(ludo_interp *in, ludo_token_kind op, ludo_type_id type, const ludo_value *a,
                       const ludo_value *b, ludo_value *out) {
    ludo_type_id base = base_of(in, type);
    ludo_type_kind kind = tkind(in, base);
    if (kind == LUDO_TYPE_VEC2) {
        /* ch2 16.1: + and - elementwise, * by a scalar, and nothing else. */
        float x;
        float y;
        if (b->kind == LUDO_VALUE_FLOAT) {
            x = (float)a->items[0].as.number * (float)b->as.number;
            y = (float)a->items[1].as.number * (float)b->as.number;
        } else {
            x = (float)float_op(op, true, a->items[0].as.number, b->items[0].as.number);
            y = (float)float_op(op, true, a->items[1].as.number, b->items[1].as.number);
        }
        return make_vec2(in, x, y, out) ? FLOW_NORMAL : stop(in, LUDO_INTERP_OUT_OF_MEMORY);
    }
    if (kind == LUDO_TYPE_F32 || kind == LUDO_TYPE_F64) {
        *out = number(type, float_op(op, kind == LUDO_TYPE_F32, a->as.number, b->as.number));
        return FLOW_NORMAL;
    }
    flow result = int_op(in, op, base, a->as.bits, b->as.bits, out);
    out->type = type;
    return result;
}

static bool compare(const ludo_value *a, const ludo_value *b, ludo_token_kind op, bool is_float,
                    bool is_signed_int) {
    int order;
    if (a->kind == LUDO_VALUE_BOOL) {
        order = (int)a->as.boolean - (int)b->as.boolean;
    } else if (is_float) {
        if (op == LUDO_TOK_BANG_EQ) {
            return !(a->as.number == b->as.number);
        }
        /* IEEE: every ordered comparison with a NaN is false (ch2 2.5). */
        return op == LUDO_TOK_EQ_EQ   ? a->as.number == b->as.number
               : op == LUDO_TOK_LT    ? a->as.number < b->as.number
               : op == LUDO_TOK_LT_EQ ? a->as.number <= b->as.number
               : op == LUDO_TOK_GT    ? a->as.number > b->as.number
                                      : a->as.number >= b->as.number;
    } else if (is_signed_int) {
        int64_t x = (int64_t)a->as.bits;
        int64_t y = (int64_t)b->as.bits;
        order = x < y ? -1 : x > y ? 1 : 0;
    } else {
        order = a->as.bits < b->as.bits ? -1 : a->as.bits > b->as.bits ? 1 : 0;
    }
    return op == LUDO_TOK_EQ_EQ     ? order == 0
           : op == LUDO_TOK_BANG_EQ ? order != 0
           : op == LUDO_TOK_LT      ? order < 0
           : op == LUDO_TOK_LT_EQ   ? order <= 0
           : op == LUDO_TOK_GT      ? order > 0
                                    : order >= 0;
}

/* ---- Expressions -------------------------------------------------------------- */

static flow eval(ludo_interp *in, frame *f, ludo_node_id id, ludo_value *out);
static flow exec_block(ludo_interp *in, frame *f, ludo_node_id block);
static flow call(ludo_interp *in, ludo_node_id decl, const ludo_value *args, size_t count,
                 ludo_value *out);

static flow eval_literal(ludo_interp *in, ludo_node_id id, ludo_value *out) {
    ludo_token_kind token = token_kind(in, id);
    ludo_type_id type = in->facts[id].type;
    if (token == LUDO_TOK_KW_TRUE || token == LUDO_TOK_KW_FALSE) {
        *out = boolean(token == LUDO_TOK_KW_TRUE);
        return FLOW_NORMAL;
    }
    ludo_type_id base = base_of(in, type);
    if (token == LUDO_TOK_FLOAT || tkind(in, base) == LUDO_TYPE_F32 ||
        tkind(in, base) == LUDO_TYPE_F64) {
        double value;
        uint64_t whole = 0;
        if (token == LUDO_TOK_FLOAT) {
            value = ludo_literal_float(in->file, id);
        } else {
            (void)ludo_literal_integer(in->file, id, &whole);
            value = (double)whole;
        }
        *out = number(type, tkind(in, base) == LUDO_TYPE_F32 ? (double)(float)value : value);
        return FLOW_NORMAL;
    }
    if (token == LUDO_TOK_INT) {
        uint64_t value = 0;
        (void)ludo_literal_integer(in->file, id, &value);
        *out = integer(type, value);
        return FLOW_NORMAL;
    }
    return stop(in, LUDO_INTERP_UNCHECKED); /* string literals are band 6's */
}

static flow optional(ludo_interp *in, ludo_type_id type, const ludo_value *present,
                     ludo_value *out) {
    if (present == NULL) {
        *out = (ludo_value){.kind = LUDO_VALUE_AGGREGATE, .type = type, .tag = 0};
        return FLOW_NORMAL;
    }
    ludo_value *payload = items(in, 1);
    if (payload == NULL) {
        return stop(in, LUDO_INTERP_OUT_OF_MEMORY);
    }
    payload[0] = *present;
    *out = (ludo_value){
        .kind = LUDO_VALUE_AGGREGATE, .type = type, .tag = 1, .count = 1, .items = payload};
    return FLOW_NORMAL;
}

static size_t optional_depth(const ludo_interp *in, ludo_type_id type) {
    size_t depth = 0;
    while (type != 0 && tkind(in, type) == LUDO_TYPE_OPTIONAL) {
        type = type_at(in, type)->inner;
        depth++;
    }
    return depth;
}

/* ch2 9.11: a T used where ?T is expected is present. Injection is by static
   type and never flattens, so the count of ?s between the two static types is
   how many times the value is wrapped -- never the value's own type, which in
   a generic body names the parameter rather than its argument. */
static flow coerce(ludo_interp *in, ludo_type_id to, ludo_type_id from, ludo_value *value) {
    size_t want = optional_depth(in, to);
    size_t have = optional_depth(in, from);
    /* Innermost layer first: the w-th wrap has the type of `to` with w - 1 of
       its outer ?s removed. */
    for (size_t w = want > have ? want - have : 0; w != 0; w--) {
        ludo_type_id layer = to;
        for (size_t i = 1; i < w; i++) {
            layer = type_at(in, layer)->inner;
        }
        ludo_value inner = *value;
        flow result = optional(in, layer, &inner, value);
        if (result != FLOW_NORMAL) {
            return result;
        }
    }
    return FLOW_NORMAL;
}

static flow eval_ident(ludo_interp *in, frame *f, ludo_node_id id, ludo_value *out) {
    const ludo_node_fact *fact = &in->facts[id];
    if (fact->ref == LUDO_REF_LOCAL) {
        slot *s = find(f, fact->target);
        if (s == NULL) {
            return stop(in, LUDO_INTERP_UNCHECKED);
        }
        const ludo_value *value = s->value.kind == LUDO_VALUE_REF ? s->value.as.ref : &s->value;
        return copy(in, value, out) ? FLOW_NORMAL : stop(in, LUDO_INTERP_OUT_OF_MEMORY);
    }
    if (fact->ref == LUDO_REF_VARIANT) {
        return optional(in, fact->type, NULL, out); /* none */
    }
    if (fact->ref == LUDO_REF_DECL && kind_of(in, fact->target) == LUDO_AST_FN) {
        *out = (ludo_value){.kind = LUDO_VALUE_FN, .type = fact->type, .as.fn = fact->target};
        return FLOW_NORMAL;
    }
    if (fact->ref == LUDO_REF_DECL) {
        return copy(in, &in->globals[fact->target], out) ? FLOW_NORMAL
                                                         : stop(in, LUDO_INTERP_OUT_OF_MEMORY);
    }
    return stop(in, LUDO_INTERP_UNCHECKED);
}

/* The place an expression names: a binding, through a mark, a field or a
   component. A ! parameter's slot holds a REF, followed to the caller's. */
static ludo_value *place(ludo_interp *in, frame *f, ludo_node_id id) {
    in->executed[id] = true;
    ludo_ast_kind kind = kind_of(in, id);
    if (kind == LUDO_AST_MARK || kind == LUDO_AST_GROUP) {
        return place(in, f, first(in, id));
    }
    if (kind == LUDO_AST_IDENT && in->facts[id].ref == LUDO_REF_LOCAL) {
        slot *s = find(f, in->facts[id].target);
        if (s == NULL) {
            return NULL;
        }
        return s->value.kind == LUDO_VALUE_REF ? s->value.as.ref : &s->value;
    }
    if (kind == LUDO_AST_MEMBER &&
        (in->facts[id].ref == LUDO_REF_FIELD || in->facts[id].ref == LUDO_REF_COMPONENT)) {
        ludo_value *receiver = place(in, f, first(in, id));
        if (receiver == NULL || receiver->kind != LUDO_VALUE_AGGREGATE ||
            in->facts[id].index >= receiver->count) {
            return NULL;
        }
        return &receiver->items[in->facts[id].index];
    }
    return NULL;
}

static flow eval_args(ludo_interp *in, frame *f, ludo_node_id first_arg, ludo_node_id params,
                      ludo_value **args, size_t *count) {
    size_t n = 0;
    for (ludo_node_id it = first_arg; it != 0; it = next(in, it)) {
        n++;
    }
    *count = n;
    *args = n == 0 ? NULL : items(in, n);
    if (n != 0 && *args == NULL) {
        return stop(in, LUDO_INTERP_OUT_OF_MEMORY);
    }
    ludo_node_id param = first(in, params);
    size_t i = 0;
    for (ludo_node_id it = first_arg; it != 0; it = next(in, it), i++) {
        bool by_place = param != 0 && kind_of(in, first(in, param)) == LUDO_AST_MUTABLE_TYPE;
        if (by_place) {
            /* ch1 7.10: the caller's marked place, written in place. */
            ludo_value *target = place(in, f, it);
            if (target == NULL) {
                return stop(in, LUDO_INTERP_UNCHECKED);
            }
            in->executed[it] = true;
            (*args)[i] = ludo_value_ref(target);
        } else {
            flow result = eval(in, f, it, &(*args)[i]);
            if (result == FLOW_NORMAL && param != 0) {
                result = coerce(in, in->facts[param].type, in->facts[it].type, &(*args)[i]);
            }
            if (result != FLOW_NORMAL) {
                return result;
            }
        }
        param = param != 0 ? next(in, param) : 0;
    }
    return FLOW_NORMAL;
}

static flow eval_call(ludo_interp *in, frame *f, ludo_node_id id, ludo_value *out) {
    ludo_node_id callee = first(in, id);
    ludo_node_id args_node = next(in, callee);
    const ludo_node_fact *fact = &in->facts[callee];
    ludo_value *args = NULL;
    size_t count = 0;
    in->executed[callee] = true;
    if (fact->ref == LUDO_REF_VEC2_NEW) {
        in->executed[first(in, callee)] = true;
        flow result = eval_args(in, f, args_node, 0, &args, &count);
        if (result != FLOW_NORMAL) {
            return result;
        }
        return make_vec2(in, (float)args[0].as.number, (float)args[1].as.number, out)
                   ? FLOW_NORMAL
                   : stop(in, LUDO_INTERP_OUT_OF_MEMORY);
    }
    if (fact->ref == LUDO_REF_VARIANT) {
        flow result = eval_args(in, f, args_node, 0, &args, &count);
        if (result != FLOW_NORMAL) {
            return result;
        }
        if (kind_of(in, callee) == LUDO_AST_MEMBER) {
            in->executed[first(in, callee)] = true;
            /* ch2 4.6.1: Type.variant(payload). */
            *out = (ludo_value){.kind = LUDO_VALUE_AGGREGATE,
                                .type = in->facts[id].type,
                                .tag = fact->index,
                                .count = count,
                                .items = args};
            return FLOW_NORMAL;
        }
        return optional(in, in->facts[id].type, &args[0], out); /* some(x) */
    }
    ludo_node_id decl = 0;
    if (fact->ref == LUDO_REF_DECL && kind_of(in, fact->target) == LUDO_AST_FN) {
        decl = fact->target;
    } else if (fact->ref == LUDO_REF_LOCAL) {
        /* ch2 7.1: a function value is a pointer to a named declaration. */
        ludo_value pointer;
        flow result = eval_ident(in, f, callee, &pointer);
        if (result != FLOW_NORMAL) {
            return result;
        }
        decl = pointer.as.fn;
    } else {
        return stop(in, LUDO_INTERP_UNCHECKED);
    }
    flow result =
        eval_args(in, f, args_node, child_of_kind(in, decl, LUDO_AST_PARAMS), &args, &count);
    if (result != FLOW_NORMAL) {
        return result;
    }
    return call(in, decl, args, count, out);
}

static flow eval_member(ludo_interp *in, frame *f, ludo_node_id id, ludo_value *out) {
    const ludo_node_fact *fact = &in->facts[id];
    ludo_node_id receiver = first(in, id);
    if (fact->ref == LUDO_REF_VARIANT) {
        in->executed[receiver] = true;
        *out = (ludo_value){.kind = LUDO_VALUE_AGGREGATE, .type = fact->type, .tag = fact->index};
        return FLOW_NORMAL;
    }
    ludo_value value;
    flow result = eval(in, f, receiver, &value);
    if (result != FLOW_NORMAL) {
        return result;
    }
    bool chained = kind_of(in, id) == LUDO_AST_OPTIONAL_MEMBER ||
                   (tkind(in, in->facts[receiver].type) == LUDO_TYPE_OPTIONAL);
    if (chained) {
        /* ch2 9.8: ?. short-circuits, and the whole chain is optional. */
        if (value.tag == 0) {
            return optional(in, fact->type, NULL, out);
        }
        ludo_value link = value.items[0].items[fact->index];
        return optional(in, fact->type, &link, out);
    }
    if (value.kind != LUDO_VALUE_AGGREGATE || fact->index >= value.count) {
        return stop(in, LUDO_INTERP_UNCHECKED);
    }
    *out = value.items[fact->index];
    return FLOW_NORMAL;
}

static flow eval_binary(ludo_interp *in, frame *f, ludo_node_id id, ludo_value *out) {
    ludo_node_id left = first(in, id);
    ludo_node_id right = next(in, left);
    ludo_token_kind op = token_kind(in, id);
    ludo_value a;
    ludo_value b;
    flow result = eval(in, f, left, &a);
    if (result != FLOW_NORMAL) {
        return result;
    }
    if (op == LUDO_TOK_KW_AND || op == LUDO_TOK_KW_OR) {
        if (a.kind == LUDO_VALUE_AGGREGATE) {
            /* ch2 9.8: unwrap-or, selected by the left operand's static type. */
            if (a.tag == 1) {
                *out = a.items[0];
                return FLOW_NORMAL;
            }
            return eval(in, f, right, out);
        }
        if (a.as.boolean == (op == LUDO_TOK_KW_OR)) {
            *out = a;
            return FLOW_NORMAL;
        }
        return eval(in, f, right, out);
    }
    result = eval(in, f, right, &b);
    if (result != FLOW_NORMAL) {
        return result;
    }
    if (op == LUDO_TOK_EQ_EQ || op == LUDO_TOK_BANG_EQ || op == LUDO_TOK_LT ||
        op == LUDO_TOK_LT_EQ || op == LUDO_TOK_GT || op == LUDO_TOK_GT_EQ) {
        ludo_type_kind kind = tkind(in, base_of(in, in->facts[left].type));
        *out = boolean(
            compare(&a, &b, op, kind == LUDO_TYPE_F32 || kind == LUDO_TYPE_F64, is_signed(kind)));
        return FLOW_NORMAL;
    }
    return arithmetic(in, op, in->facts[id].type, &a, &b, out);
}

static flow eval_unary(ludo_interp *in, frame *f, ludo_node_id id, ludo_value *out) {
    ludo_value value;
    flow result = eval(in, f, first(in, id), &value);
    if (result != FLOW_NORMAL) {
        return result;
    }
    ludo_token_kind op = token_kind(in, id);
    ludo_type_id type = in->facts[id].type;
    if (op == LUDO_TOK_KW_NOT) {
        *out = boolean(!value.as.boolean);
        return FLOW_NORMAL;
    }
    if (op == LUDO_TOK_TILDE) {
        ludo_type_kind kind = tkind(in, base_of(in, type));
        uint64_t bits = ~value.as.bits;
        if (!is_signed(kind) && width(kind) < 64) {
            bits &= ((uint64_t)1 << width(kind)) - 1;
        }
        *out = integer(type, bits);
        return FLOW_NORMAL;
    }
    ludo_value zero = value;
    if (value.kind == LUDO_VALUE_FLOAT) {
        *out = number(type, -value.as.number);
        return FLOW_NORMAL;
    }
    if (value.kind == LUDO_VALUE_AGGREGATE) {
        return make_vec2(in, -(float)value.items[0].as.number, -(float)value.items[1].as.number,
                         out)
                   ? FLOW_NORMAL
                   : stop(in, LUDO_INTERP_OUT_OF_MEMORY);
    }
    zero.as.bits = 0;
    return arithmetic(in, LUDO_TOK_MINUS, type, &zero, &value, out);
}

static flow eval_kind(ludo_interp *in, frame *f, ludo_node_id id, ludo_value *out) {
    switch (kind_of(in, id)) {
    case LUDO_AST_LITERAL:
        return eval_literal(in, id, out);
    case LUDO_AST_IDENT:
        return eval_ident(in, f, id, out);
    case LUDO_AST_GROUP:
    case LUDO_AST_MARK:
        return eval(in, f, first(in, id), out);
    case LUDO_AST_BINARY:
        return eval_binary(in, f, id, out);
    case LUDO_AST_UNARY:
        return eval_unary(in, f, id, out);
    case LUDO_AST_MEMBER:
    case LUDO_AST_OPTIONAL_MEMBER:
        return eval_member(in, f, id, out);
    case LUDO_AST_CALL:
        return eval_call(in, f, id, out);
    case LUDO_AST_ERROR:
    case LUDO_AST_CAST:
    case LUDO_AST_BRACKET:
    case LUDO_AST_DEREF:
    case LUDO_AST_AGGREGATE:
    case LUDO_AST_RESCUE:
    case LUDO_AST_ROOT:
    case LUDO_AST_DISCARD:
    case LUDO_AST_NAMED_ITEM:
    case LUDO_AST_KEYED_ITEM:
    case LUDO_AST_HANDLER:
    case LUDO_AST_MODULE:
    case LUDO_AST_EXPLICIT:
    case LUDO_AST_LIBRARY:
    case LUDO_AST_USE:
    case LUDO_AST_ATTRIBUTE:
    case LUDO_AST_TYPE_DECL:
    case LUDO_AST_CONST:
    case LUDO_AST_FN:
    case LUDO_AST_EXTERN_FN:
    case LUDO_AST_EXTERN_ITEM:
    case LUDO_AST_PERSIST:
    case LUDO_AST_FRAME:
    case LUDO_AST_IMPL:
    case LUDO_AST_GENERIC_PARAMS:
    case LUDO_AST_GENERIC_PARAM:
    case LUDO_AST_CONSTRAINTS:
    case LUDO_AST_PARAMS:
    case LUDO_AST_PARAM:
    case LUDO_AST_RETURNS:
    case LUDO_AST_STRUCT:
    case LUDO_AST_FIELD:
    case LUDO_AST_ENUM:
    case LUDO_AST_VARIANT:
    case LUDO_AST_DISTINCT:
    case LUDO_AST_INTERFACE:
    case LUDO_AST_FN_TYPE:
    case LUDO_AST_NAMED_TYPE:
    case LUDO_AST_GENERIC_ARGS:
    case LUDO_AST_OPTIONAL_TYPE:
    case LUDO_AST_POINTER_TYPE:
    case LUDO_AST_VIEW_TYPE:
    case LUDO_AST_ARRAY_TYPE:
    case LUDO_AST_MUTABLE_TYPE:
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
        /* Unreachable in a CHECKED declaration: a later band's construct
           makes its declaration PENDING, and PENDING is never run. */
        return stop(in, LUDO_INTERP_UNCHECKED);
    }
    return stop(in, LUDO_INTERP_UNCHECKED);
}

static flow eval(ludo_interp *in, frame *f, ludo_node_id id, ludo_value *out) {
    in->executed[id] = true;
    return eval_kind(in, f, id, out);
}

/* ---- Statements ----------------------------------------------------------------- */

static flow exec(ludo_interp *in, frame *f, ludo_node_id id);

/* Binds what a pattern binds; false when the value is not this pattern. */
static bool matches(ludo_interp *in, frame *f, ludo_node_id pattern, const ludo_value *value,
                    bool *exhausted) {
    in->executed[pattern] = true;
    const ludo_node_fact *fact = &in->facts[pattern];
    ludo_ast_kind kind = kind_of(in, pattern);
    if (kind == LUDO_AST_PATTERN_DISCARD) {
        return true;
    }
    if (fact->ref == LUDO_REF_BINDS) {
        ludo_value bound;
        if (!copy(in, value, &bound) || !bind(in, f, pattern, bound)) {
            *exhausted = true;
            return false;
        }
        return true;
    }
    if (fact->ref != LUDO_REF_VARIANT || value->tag != fact->index) {
        return false;
    }
    size_t i = 0;
    for (ludo_node_id it = first(in, pattern); it != 0; it = next(in, it), i++) {
        if (i >= value->count || !matches(in, f, it, &value->items[i], exhausted)) {
            return false;
        }
    }
    return true;
}

static flow exec_match(ludo_interp *in, frame *f, ludo_node_id id) {
    ludo_node_id scrutinee = first(in, id);
    ludo_value value;
    flow result = eval(in, f, scrutinee, &value);
    if (result != FLOW_NORMAL) {
        return result;
    }
    for (ludo_node_id arm = next(in, scrutinee); arm != 0; arm = next(in, arm)) {
        bool exhausted = false;
        in->executed[arm] = true;
        if (matches(in, f, first(in, arm), &value, &exhausted)) {
            return exec_block(in, f, next(in, first(in, arm)));
        }
        if (exhausted) {
            return stop(in, LUDO_INTERP_OUT_OF_MEMORY);
        }
    }
    /* ch2 4.3: exhaustive by checking, so no arm is a checker bug. */
    return stop(in, LUDO_INTERP_UNCHECKED);
}

static flow exec_for(ludo_interp *in, frame *f, ludo_node_id id) {
    ludo_node_id bindings = first(in, id);
    ludo_node_id subject = next(in, bindings);
    ludo_node_id block = next(in, subject);
    ludo_value view;
    in->executed[bindings] = true;
    flow result = eval(in, f, subject, &view);
    if (result != FLOW_NORMAL) {
        return result;
    }
    /* The subject is evaluated once; each turn binds a copy of the element. */
    for (size_t i = 0; i < view.count; i++) {
        ludo_value element;
        if (!copy(in, &view.items[i], &element) || !bind(in, f, first(in, bindings), element)) {
            return stop(in, LUDO_INTERP_OUT_OF_MEMORY);
        }
        result = exec_block(in, f, block);
        if (result == FLOW_BREAK) {
            break;
        }
        if (result != FLOW_NORMAL && result != FLOW_CONTINUE) {
            return result;
        }
    }
    return FLOW_NORMAL;
}

static flow truth(ludo_interp *in, frame *f, ludo_node_id condition, bool *value) {
    ludo_value result;
    flow outcome = eval(in, f, condition, &result);
    *value = outcome == FLOW_NORMAL && result.as.boolean;
    return outcome;
}

static flow exec_assign(ludo_interp *in, frame *f, ludo_node_id id) {
    ludo_node_id target = first(in, id);
    ludo_node_id value_node = next(in, target);
    ludo_value value;
    flow result = eval(in, f, value_node, &value);
    if (result != FLOW_NORMAL) {
        return result;
    }
    ludo_value *slot_value = place(in, f, target);
    if (slot_value == NULL) {
        return stop(in, LUDO_INTERP_UNCHECKED);
    }
    if (kind_of(in, id) == LUDO_AST_COMPOUND_ASSIGN) {
        /* ch1 6.5: pure sugar over a = a op b. */
        static const ludo_token_kind ops[] = {LUDO_TOK_PLUS, LUDO_TOK_MINUS, LUDO_TOK_STAR,
                                              LUDO_TOK_SLASH, LUDO_TOK_PERCENT};
        size_t index = (size_t)token_kind(in, id) - (size_t)LUDO_TOK_PLUS_EQ;
        ludo_value combined;
        result = arithmetic(in, ops[index], in->facts[target].type, slot_value, &value, &combined);
        if (result != FLOW_NORMAL) {
            return result;
        }
        *slot_value = combined;
        return FLOW_NORMAL;
    }
    result = coerce(in, in->facts[target].type, in->facts[value_node].type, &value);
    if (result == FLOW_NORMAL) {
        *slot_value = value;
    }
    return result;
}

static flow exec_kind(ludo_interp *in, frame *f, ludo_node_id id) {
    switch (kind_of(in, id)) {
    case LUDO_AST_LET: {
        ludo_node_id bindings = first(in, id);
        ludo_node_id init = next(in, bindings);
        ludo_type_id declared = 0;
        if (next(in, init) != 0) {
            declared = in->facts[init].type;
            init = next(in, init);
        }
        ludo_value value;
        in->executed[bindings] = true;
        flow result = eval(in, f, init, &value);
        if (result == FLOW_NORMAL) {
            result = coerce(in, declared, in->facts[init].type, &value);
        }
        if (result != FLOW_NORMAL) {
            return result;
        }
        return bind(in, f, first(in, bindings), value) ? FLOW_NORMAL
                                                       : stop(in, LUDO_INTERP_OUT_OF_MEMORY);
    }
    case LUDO_AST_ASSIGN:
    case LUDO_AST_COMPOUND_ASSIGN:
        return exec_assign(in, f, id);
    case LUDO_AST_EXPR_STMT: {
        ludo_value ignored;
        return eval(in, f, first(in, id), &ignored);
    }
    case LUDO_AST_TAIL:
    case LUDO_AST_RETURN: {
        ludo_node_id value = first(in, id);
        if (value == 0) {
            in->returned = unit();
            return FLOW_RETURN;
        }
        flow result = eval(in, f, value, &in->returned);
        if (result == FLOW_NORMAL) {
            result = coerce(in, in->result_type, in->facts[value].type, &in->returned);
        }
        return result == FLOW_NORMAL ? FLOW_RETURN : result;
    }
    case LUDO_AST_MODIFIER: {
        ludo_node_id statement = first(in, id);
        bool holds = false;
        flow result = truth(in, f, next(in, statement), &holds);
        if (result != FLOW_NORMAL) {
            return result;
        }
        /* ch1 6.13: `if` runs it when true, `unless` when false. */
        return holds == (token_kind(in, id) == LUDO_TOK_KW_IF) ? exec(in, f, statement)
                                                               : FLOW_NORMAL;
    }
    case LUDO_AST_IF:
        for (ludo_node_id branch = first(in, id); branch != 0; branch = next(in, branch)) {
            ludo_node_id head = first(in, branch);
            in->executed[branch] = true;
            if (kind_of(in, head) == LUDO_AST_BLOCK) {
                return exec_block(in, f, head);
            }
            bool holds = false;
            flow result = truth(in, f, head, &holds);
            if (result != FLOW_NORMAL) {
                return result;
            }
            if (holds) {
                return exec_block(in, f, next(in, head));
            }
        }
        return FLOW_NORMAL;
    case LUDO_AST_WHILE: {
        ludo_node_id condition = first(in, id);
        for (;;) {
            bool holds = false;
            flow result = truth(in, f, condition, &holds);
            if (result != FLOW_NORMAL) {
                return result;
            }
            if (!holds) {
                return FLOW_NORMAL;
            }
            result = exec_block(in, f, next(in, condition));
            if (result == FLOW_BREAK) {
                return FLOW_NORMAL;
            }
            if (result != FLOW_NORMAL && result != FLOW_CONTINUE) {
                return result;
            }
        }
    }
    case LUDO_AST_FOR:
        return exec_for(in, f, id);
    case LUDO_AST_MATCH:
        return exec_match(in, f, id);
    case LUDO_AST_BREAK:
        return FLOW_BREAK;
    case LUDO_AST_CONTINUE:
        return FLOW_CONTINUE;
    case LUDO_AST_ERROR:
    case LUDO_AST_DEFER:
    case LUDO_AST_UNSAFE:
    case LUDO_AST_BLOCK:
    case LUDO_AST_MODULE:
    case LUDO_AST_EXPLICIT:
    case LUDO_AST_LIBRARY:
    case LUDO_AST_USE:
    case LUDO_AST_ATTRIBUTE:
    case LUDO_AST_TYPE_DECL:
    case LUDO_AST_CONST:
    case LUDO_AST_FN:
    case LUDO_AST_EXTERN_FN:
    case LUDO_AST_EXTERN_ITEM:
    case LUDO_AST_PERSIST:
    case LUDO_AST_FRAME:
    case LUDO_AST_IMPL:
    case LUDO_AST_GENERIC_PARAMS:
    case LUDO_AST_GENERIC_PARAM:
    case LUDO_AST_CONSTRAINTS:
    case LUDO_AST_PARAMS:
    case LUDO_AST_PARAM:
    case LUDO_AST_RETURNS:
    case LUDO_AST_STRUCT:
    case LUDO_AST_FIELD:
    case LUDO_AST_ENUM:
    case LUDO_AST_VARIANT:
    case LUDO_AST_DISTINCT:
    case LUDO_AST_INTERFACE:
    case LUDO_AST_FN_TYPE:
    case LUDO_AST_NAMED_TYPE:
    case LUDO_AST_GENERIC_ARGS:
    case LUDO_AST_OPTIONAL_TYPE:
    case LUDO_AST_POINTER_TYPE:
    case LUDO_AST_VIEW_TYPE:
    case LUDO_AST_ARRAY_TYPE:
    case LUDO_AST_MUTABLE_TYPE:
    case LUDO_AST_FALLIBLE_TYPE:
    case LUDO_AST_BINDINGS:
    case LUDO_AST_BRANCH:
    case LUDO_AST_BIND_CONDITION:
    case LUDO_AST_ARM:
    case LUDO_AST_PATTERN_IDENT:
    case LUDO_AST_PATTERN_VARIANT:
    case LUDO_AST_PATTERN_STRUCT:
    case LUDO_AST_PATTERN_FIELD:
    case LUDO_AST_PATTERN_LITERAL:
    case LUDO_AST_PATTERN_DISCARD:
    case LUDO_AST_IDENT:
    case LUDO_AST_ROOT:
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
        return stop(in, LUDO_INTERP_UNCHECKED); /* see eval_kind */
    }
    return stop(in, LUDO_INTERP_UNCHECKED);
}

static flow exec(ludo_interp *in, frame *f, ludo_node_id id) {
    in->executed[id] = true;
    return exec_kind(in, f, id);
}

static flow exec_block(ludo_interp *in, frame *f, ludo_node_id block) {
    in->executed[block] = true;
    for (ludo_node_id it = first(in, block); it != 0; it = next(in, it)) {
        flow result = exec(in, f, it);
        if (result != FLOW_NORMAL) {
            return result;
        }
    }
    return FLOW_NORMAL;
}

/* A fresh frame per call: every function is top-level and nothing captures
   (ch2 7.2, 7.3), so a body sees its parameters and its own bindings only. */
static flow call(ludo_interp *in, ludo_node_id decl, const ludo_value *args, size_t count,
                 ludo_value *out) {
    if (in->facts[decl].state != LUDO_CHECK_CHECKED) {
        return stop(in, LUDO_INTERP_UNCHECKED);
    }
    frame f = {0};
    in->executed[decl] = true;
    ludo_node_id params = child_of_kind(in, decl, LUDO_AST_PARAMS);
    size_t i = 0;
    for (ludo_node_id it = first(in, params); it != 0; it = next(in, it), i++) {
        bool by_place = kind_of(in, first(in, it)) == LUDO_AST_MUTABLE_TYPE;
        if (i >= count || (by_place != (args[i].kind == LUDO_VALUE_REF))) {
            return stop(in, LUDO_INTERP_BAD_ARGUMENT);
        }
        if (!bind(in, &f, it, args[i])) {
            return stop(in, LUDO_INTERP_OUT_OF_MEMORY);
        }
    }
    if (i != count) {
        return stop(in, LUDO_INTERP_BAD_ARGUMENT);
    }
    ludo_type_id caller_result = in->result_type;
    in->result_type = type_at(in, in->facts[decl].type)->inner;
    in->returned = unit();
    flow result = exec_block(in, &f, child_of_kind(in, decl, LUDO_AST_BLOCK));
    in->result_type = caller_result;
    if (result == FLOW_STOP) {
        return result;
    }
    *out = in->returned;
    return FLOW_NORMAL;
}

/* ---- Loading -------------------------------------------------------------------- */

static flow load_extern(ludo_interp *in, ludo_node_id id) {
    size_t length = 0;
    const char *text = ludo_node_text(in->file, id, &length);
    char *name = ludo_arena_alloc(&in->arena, length + 1);
    if (name == NULL) {
        return stop(in, LUDO_INTERP_OUT_OF_MEMORY);
    }
    memcpy(name, text, length);
    name[length] = '\0';
    ludo_extern_class class_id = type_at(in, in->facts[id].type)->extern_class;
    uint32_t handle = LUDO_HANDLE_NONE;
    if (class_id == LUDO_EXTERN_IMAGE) {
        handle = ludo_image_by_name(in->constants, name);
    } else if (class_id == LUDO_EXTERN_STORAGE) {
        handle = ludo_storage_by_name(in->constants, name);
    } else {
        return stop(in, LUDO_INTERP_UNCHECKED); /* fonts and shaders are band 6's */
    }
    if (handle == LUDO_HANDLE_NONE) {
        return fault(in, LUDO_FAULT_MISSING_ASSET); /* ch5 8.4 */
    }
    in->executed[id] = true;
    in->executed[first(in, id)] = true;
    if (next(in, first(in, id)) != 0) {
        in->executed[next(in, first(in, id))] = true;
    }
    in->globals[id] =
        (ludo_value){.kind = LUDO_VALUE_HANDLE, .type = in->facts[id].type, .as.handle = handle};
    return FLOW_NORMAL;
}

/* ch5 8.3: every declared asset resolves before top level runs; constants
   fold. Declaration order, because a constant may name an earlier one. */
static flow load(ludo_interp *in) {
    frame f = {0};
    for (ludo_node_id it = first(in, in->ast->root); it != 0; it = next(in, it)) {
        if (in->facts[it].state != LUDO_CHECK_CHECKED) {
            continue;
        }
        flow result = FLOW_NORMAL;
        if (kind_of(in, it) == LUDO_AST_CONST) {
            ludo_node_id init = first(in, it);
            if (next(in, init) != 0) {
                init = next(in, init);
            }
            in->executed[it] = true;
            result = eval(in, &f, init, &in->globals[it]);
        } else if (kind_of(in, it) == LUDO_AST_EXTERN_ITEM) {
            result = load_extern(in, it);
        }
        if (result != FLOW_NORMAL) {
            return result;
        }
    }
    return FLOW_NORMAL;
}

/* ---- The public surface ------------------------------------------------------- */

ludo_interp_status ludo_interp_create(const ludo_checked_file *program, const ludo_host *host,
                                      const ludo_host_constants *constants, ludo_interp **out) {
    *out = NULL;
    if (ludo_host_check(host) != LUDO_HOST_OK || ludo_constants_check(constants) != LUDO_HOST_OK) {
        return LUDO_INTERP_BAD_ARGUMENT;
    }
    ludo_arena arena = {0};
    ludo_interp *in = ludo_arena_alloc(&arena, sizeof *in);
    size_t nodes = program->file->ast.count;
    if (in == NULL) {
        ludo_arena_destroy(&arena);
        return LUDO_INTERP_OUT_OF_MEMORY;
    }
    *in = (ludo_interp){.program = program,
                        .file = program->file,
                        .ast = &program->file->ast,
                        .facts = program->facts,
                        .host = host,
                        .constants = constants};
    in->executed = ludo_arena_alloc(&arena, nodes * sizeof *in->executed);
    in->globals = ludo_arena_alloc(&arena, nodes * sizeof *in->globals);
    in->arena = arena;
    if (in->executed == NULL || in->globals == NULL) {
        ludo_arena_destroy(&in->arena);
        return LUDO_INTERP_OUT_OF_MEMORY;
    }
    memset(in->executed, 0, nodes * sizeof *in->executed);
    memset(in->globals, 0, nodes * sizeof *in->globals);
    if (load(in) == FLOW_STOP) {
        ludo_interp_status status = in->status;
        *out = in; /* a fault keeps the interpreter readable: fault and bits */
        return status;
    }
    *out = in;
    return LUDO_INTERP_OK;
}

void ludo_interp_destroy(ludo_interp *interp) {
    if (interp == NULL) {
        return;
    }
    /* The interpreter lives in its own arena. */
    ludo_arena arena = interp->arena;
    ludo_arena_destroy(&arena);
}

ludo_interp_status ludo_interp_call(ludo_interp *interp, const char *name, const ludo_value *args,
                                    size_t count, ludo_value *result) {
    ludo_node_id decl = ludo_checked_lookup(interp->program, name, strlen(name));
    if (kind_of(interp, decl) != LUDO_AST_FN) {
        return LUDO_INTERP_UNCHECKED;
    }
    interp->status = LUDO_INTERP_OK;
    if (interp->facts[decl].state != LUDO_CHECK_CHECKED) {
        return LUDO_INTERP_UNCHECKED;
    }
    /* A caller outside the program has no static types to offer, so its
       arguments are injected by their own (ch2 9.11). */
    ludo_value *coerced = items(interp, count);
    if (count != 0 && coerced == NULL) {
        return LUDO_INTERP_OUT_OF_MEMORY;
    }
    ludo_node_id param = first(interp, child_of_kind(interp, decl, LUDO_AST_PARAMS));
    for (size_t i = 0; i < count; i++) {
        coerced[i] = args[i];
        if (param != 0 && args[i].kind != LUDO_VALUE_REF &&
            coerce(interp, interp->facts[param].type, args[i].type, &coerced[i]) == FLOW_STOP) {
            return interp->status;
        }
        param = param != 0 ? next(interp, param) : 0;
    }
    return call(interp, decl, coerced, count, result) == FLOW_STOP ? interp->status
                                                                   : LUDO_INTERP_OK;
}

ludo_interp_status ludo_interp_global(ludo_interp *interp, const char *name, ludo_value *out) {
    ludo_node_id decl = ludo_checked_lookup(interp->program, name, strlen(name));
    ludo_ast_kind kind = kind_of(interp, decl);
    if ((kind != LUDO_AST_CONST && kind != LUDO_AST_EXTERN_ITEM) ||
        interp->facts[decl].state != LUDO_CHECK_CHECKED) {
        return LUDO_INTERP_UNCHECKED;
    }
    *out = interp->globals[decl];
    return LUDO_INTERP_OK;
}

static ludo_node_id type_body(ludo_interp *in, const char *type_name, ludo_ast_kind kind) {
    ludo_node_id decl = ludo_checked_lookup(in->program, type_name, strlen(type_name));
    if (kind_of(in, decl) != LUDO_AST_TYPE_DECL || in->facts[decl].state != LUDO_CHECK_CHECKED) {
        return 0;
    }
    return child_of_kind(in, decl, kind);
}

static bool node_named(const ludo_interp *in, ludo_node_id id, const char *name) {
    size_t length = 0;
    const char *text = ludo_node_text(in->file, id, &length);
    return length == strlen(name) && memcmp(text, name, length) == 0;
}

ludo_interp_status ludo_interp_make_struct(ludo_interp *interp, const char *type_name,
                                           const char *const *fields, const ludo_value *values,
                                           size_t count, ludo_value *out) {
    ludo_node_id body = type_body(interp, type_name, LUDO_AST_STRUCT);
    if (body == 0) {
        return LUDO_INTERP_UNCHECKED;
    }
    size_t total = 0;
    for (ludo_node_id it = first(interp, body); it != 0; it = next(interp, it)) {
        total++;
    }
    ludo_value *made = items(interp, total);
    if (total != 0 && made == NULL) {
        return LUDO_INTERP_OUT_OF_MEMORY;
    }
    size_t used = 0;
    size_t index = 0;
    for (ludo_node_id field = first(interp, body); field != 0;
         field = next(interp, field), index++) {
        bool given = false;
        for (size_t i = 0; i < count; i++) {
            if (node_named(interp, field, fields[i])) {
                made[index] = values[i];
                given = true;
                used++;
            }
        }
        ludo_node_id fallback = next(interp, first(interp, field));
        if (!given && fallback == 0) {
            return LUDO_INTERP_BAD_ARGUMENT; /* ch2 9.6: no implicit zero */
        }
        if (!given) {
            frame f = {0};
            interp->status = LUDO_INTERP_OK;
            if (eval(interp, &f, fallback, &made[index]) == FLOW_STOP) {
                return interp->status;
            }
        }
    }
    if (used != count) {
        return LUDO_INTERP_BAD_ARGUMENT;
    }
    ludo_node_id decl = ludo_checked_lookup(interp->program, type_name, strlen(type_name));
    *out = (ludo_value){.kind = LUDO_VALUE_AGGREGATE,
                        .type = interp->facts[decl].type,
                        .count = total,
                        .items = made};
    return LUDO_INTERP_OK;
}

ludo_interp_status ludo_interp_make_variant(ludo_interp *interp, const char *type_name,
                                            const char *variant, const ludo_value *payload,
                                            size_t count, ludo_value *out) {
    ludo_node_id body = type_body(interp, type_name, LUDO_AST_ENUM);
    uint32_t tag = 0;
    for (ludo_node_id it = first(interp, body); it != 0; it = next(interp, it), tag++) {
        if (!node_named(interp, it, variant)) {
            continue;
        }
        size_t arity = 0;
        for (ludo_node_id p = first(interp, it); p != 0; p = next(interp, p)) {
            arity++;
        }
        if (arity != count) {
            return LUDO_INTERP_BAD_ARGUMENT;
        }
        ludo_value *made = items(interp, count);
        if (count != 0 && made == NULL) {
            return LUDO_INTERP_OUT_OF_MEMORY;
        }
        if (count != 0) {
            memcpy(made, payload, count * sizeof *made);
        }
        ludo_node_id decl = ludo_checked_lookup(interp->program, type_name, strlen(type_name));
        *out = (ludo_value){.kind = LUDO_VALUE_AGGREGATE,
                            .type = interp->facts[decl].type,
                            .tag = tag,
                            .count = count,
                            .items = made};
        return LUDO_INTERP_OK;
    }
    return body == 0 ? LUDO_INTERP_UNCHECKED : LUDO_INTERP_BAD_ARGUMENT;
}

ludo_interp_status ludo_interp_make_view(ludo_interp *interp, const ludo_value *values,
                                         size_t count, ludo_value *out) {
    ludo_value *made = items(interp, count);
    if (count != 0 && made == NULL) {
        return LUDO_INTERP_OUT_OF_MEMORY;
    }
    for (size_t i = 0; i < count; i++) {
        if (!copy(interp, &values[i], &made[i])) {
            return LUDO_INTERP_OUT_OF_MEMORY;
        }
    }
    *out = (ludo_value){.kind = LUDO_VALUE_VIEW, .count = count, .items = made};
    return LUDO_INTERP_OK;
}

ludo_interp_status ludo_interp_function(ludo_interp *interp, const char *name, ludo_value *out) {
    ludo_node_id decl = ludo_checked_lookup(interp->program, name, strlen(name));
    if (kind_of(interp, decl) != LUDO_AST_FN) {
        return LUDO_INTERP_UNCHECKED;
    }
    *out = (ludo_value){.kind = LUDO_VALUE_FN, .type = interp->facts[decl].type, .as.fn = decl};
    return LUDO_INTERP_OK;
}

const bool *ludo_interp_executed(const ludo_interp *interp) { return interp->executed; }

ludo_fault ludo_interp_fault(const ludo_interp *interp) { return interp->fault; }
