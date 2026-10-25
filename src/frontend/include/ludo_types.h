/* The type table and the checker's side tables (#130, #143).

   Types are interned in one table per checked file, so two type ids are equal
   exactly when the types are. The primitives sit at fixed ids, which is what
   lets interp/ and a test mint a primitive value without holding the table.

   Every side table is indexed by the file's node id (#130: types are a side
   table keyed by node index, so the tree stays the one thing a formatter, an
   oracle and the interpreter all read). Band 4 turns these into a program-wide
   table keyed by (source id, node id); with one file, the source id is the
   file's. */
#ifndef LUDO_TYPES_H
#define LUDO_TYPES_H

#include <stddef.h>
#include <stdint.h>

#include "ludo_ast.h"

typedef uint32_t ludo_type_id;

typedef enum ludo_type_kind {
    LUDO_TYPE_NONE, /* slot 0, and the type of a node that has none */
    /* A construct a later band checks. It satisfies every check silently, so
       the parts of the program a band has not reached never report an error,
       and the declaration holding it is marked PENDING rather than CHECKED. */
    LUDO_TYPE_PENDING,
    /* Already reported. Satisfies every check silently, so one mistake is
       reported once. */
    LUDO_TYPE_INVALID,
    LUDO_TYPE_UNIT, /* the result of a function with no return type */
    LUDO_TYPE_BOOL,
    LUDO_TYPE_I8,
    LUDO_TYPE_I16,
    LUDO_TYPE_I32,
    LUDO_TYPE_I64,
    LUDO_TYPE_U8,
    LUDO_TYPE_U16,
    LUDO_TYPE_U32,
    LUDO_TYPE_U64,
    LUDO_TYPE_USIZE,
    LUDO_TYPE_F32,
    LUDO_TYPE_F64,
    LUDO_TYPE_CHAR,
    LUDO_TYPE_STRING,
    LUDO_TYPE_VEC2, /* ch6 3.2: blessed, two f32 components x and y */

    LUDO_TYPE_STRUCT,   /* decl is the type-decl */
    LUDO_TYPE_ENUM,     /* decl is the type-decl */
    LUDO_TYPE_DISTINCT, /* decl is the type-decl; inner is the base */
    LUDO_TYPE_PARAM,    /* decl is the generic-param, opaque in the body (ch2 5.2) */
    LUDO_TYPE_VIEW,     /* []inner */
    LUDO_TYPE_OPTIONAL, /* ?inner: variant 0 is none, variant 1 is some(inner) */
    LUDO_TYPE_FN,       /* params; inner is the result, UNIT when none is written */
    LUDO_TYPE_EXTERN,   /* ch1 5.14: an extern item; extern_class says which */
    LUDO_TYPE_MODULE    /* the name a `use` binds; band 4 gives it members */
} ludo_type_kind;

/* Fixed ids: id N is the primitive of kind N, for every kind up to VEC2. */
#define LUDO_TYPE_ID_OF(kind) ((ludo_type_id)(kind))
#define LUDO_TYPE_FIXED_COUNT ((size_t)LUDO_TYPE_VEC2 + 1u)

typedef enum ludo_extern_class {
    LUDO_EXTERN_IMAGE,
    LUDO_EXTERN_FONT,
    LUDO_EXTERN_SHADER,
    LUDO_EXTERN_STORAGE
} ludo_extern_class;

typedef struct ludo_type {
    ludo_type_kind kind;
    ludo_node_id decl;
    ludo_type_id inner;
    const ludo_type_id *params; /* FN only */
    size_t param_count;
    ludo_extern_class extern_class; /* EXTERN only */
} ludo_type;

/* One entry in the instantiation cache (ch2 5.1, 5.3). The cache is keyed by
   (generic declaration, type arguments), and a call to a generic records the
   entry it instantiates. Checking a call needs only the signature (ch2 5.3);
   the entry is what codegen monomorphises and what the evaluator reports. */
typedef struct ludo_instance {
    ludo_node_id decl;
    const ludo_type_id *args; /* one per generic-param, in order */
    size_t arg_count;
    ludo_type_id signature; /* the FN type with the arguments substituted */
} ludo_instance;

/* Per top-level declaration. Checking is band-local but declaration collection
   is whole-file (#143): every top-level name is known before any body is
   checked, and a body that reaches a construct no band has checked yet is
   PENDING, never an error. */
typedef enum ludo_check_state {
    LUDO_CHECK_NOT_A_DECLARATION,
    LUDO_CHECK_CHECKED,
    LUDO_CHECK_PENDING
} ludo_check_state;

/* What a node names, as the name table answers it. */
typedef enum ludo_ref_kind {
    LUDO_REF_NONE,
    LUDO_REF_DECL,      /* target: a top-level declaration */
    LUDO_REF_LOCAL,     /* target: the param or binding node that defines it */
    LUDO_REF_BINDS,     /* this node defines a local (a binding ident, a binding pattern) */
    LUDO_REF_FIELD,     /* target: the field node; index: its position */
    LUDO_REF_COMPONENT, /* a vec2 component; index: 0 for x, 1 for y */
    LUDO_REF_VARIANT,   /* target: the variant node, 0 for none/some; index: its tag */
    LUDO_REF_GENERIC,   /* target: the generic-param */
    LUDO_REF_VEC2_NEW   /* $.vec2, the positional constructor (ch6 3.5) */
} ludo_ref_kind;

/* One record per node. Absent fields are 0. */
typedef struct ludo_node_fact {
    ludo_type_id type; /* of an expression, a type expression or a declaration */
    ludo_ref_kind ref;
    ludo_node_id target;
    uint32_t index;
    uint32_t instance;      /* call to a generic: 1 + its instance's index */
    ludo_check_state state; /* top-level declarations only */
} ludo_node_fact;

/* Defined in ludo_frontend.h, beside the call that produces it. */
typedef struct ludo_checked_file ludo_checked_file;

#endif /* LUDO_TYPES_H */
