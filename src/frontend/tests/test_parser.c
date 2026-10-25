#include "../arena.h"
#include "ludo_frontend.h"
#include "test.h"

#include <stdint.h>
#include <string.h>

static const ludo_parsed_file *parse(ludo_session *session, const char *source, bool valid) {
    const ludo_parsed_file *file = NULL;
    ludo_frontend_status status = ludo_parse_file(session, "test", source, strlen(source), &file);
    LUDO_CHECK(status == LUDO_FRONTEND_OK);
    if (file == NULL) {
        return NULL;
    }
    if (valid && file->diagnostic_count != 0) {
        (void)fprintf(stderr, "unexpected parse errors: %s\n", source);
        for (size_t i = 0; i < file->diagnostic_count; i++) {
            (void)fprintf(stderr, "  %zu: %s\n", file->diagnostics[i].primary.offset,
                          file->diagnostics[i].message);
        }
    }
    LUDO_CHECK((file->diagnostic_count == 0) == valid);
    size_t errors = 0;
    for (size_t i = 1; i < file->ast.count; i++) {
        const ludo_ast_node *node = ludo_ast_get(&file->ast, i);
        LUDO_CHECK(node != NULL);
        LUDO_CHECK(node->span.file == file->id);
        LUDO_CHECK(node->span.offset <= file->length);
        LUDO_CHECK(node->span.length <= file->length - node->span.offset);
        if (node->kind == LUDO_AST_ERROR) {
            errors++;
        }
    }
    LUDO_CHECK((errors == 0) == valid);
    return file;
}

static size_t kind_count(const ludo_parsed_file *file, ludo_ast_kind kind) {
    size_t count = 0;
    for (size_t i = 1; i < file->ast.count; i++) {
        count += ludo_ast_get(&file->ast, i)->kind == kind ? 1U : 0U;
    }
    return count;
}

static ludo_node_id find(const ludo_parsed_file *file, ludo_ast_kind kind, const char *token) {
    for (size_t i = 1; i < file->ast.count; i++) {
        const ludo_ast_node *node = ludo_ast_get(&file->ast, i);
        ludo_token t = file->tokens[node->token];
        if (node->kind == kind && t.length == strlen(token) &&
            memcmp(file->source + t.offset, token, t.length) == 0) {
            return i;
        }
    }
    return 0;
}

static void test_surface(ludo_session *session) {
    static const char *const valid[] = {
        "",
        "-- comment\n",
        "#explicit library demo use \"dep\"",
        "pub #align(16) type Point = struct x: f32 = 1.0 y: f32 end",
        "type Result[T, N: usize] = enum no yes(T, [N]u8) end",
        "type Id = distinct numeric u32 type Opaque = distinct u32 type Alias = Foo",
        "type Marker = interface end type Ranked[T] = interface fn rank(v: T) -> f32 end",
        "type Ordered[T] = interface fn compare[U: Ranked[U]](x: U, y: U) -> int end",
        "impl Rock: Ranked[Rock] + Hash[Rock]",
        "pub const N: usize = 4 const A = 0xFF const B = 0b10 const C = 1e3",
        "const x: T rescue E = g() fn local() x: T rescue E = g() end",
        "fn defaulted(a: T rescue E = g()) end type X = Box[T rescue]",
        "fn elided() -> T rescue state.field = v end",
        "fn elided() -> T rescue state!.field = v end",
        "fn elided() -> T rescue state[0].field = v end",
        "fn elided() -> T rescue n + 1 end",
        "persist n: u64 = 0 persist xs: !List[Rock] persist cb: fn()",
        "extern \"SDL3\" fn poll(event: ^Event) -> bool extern image rock = \"rock.png\"",
        "extern storage save1 extern shader water = \"water.wgsl\"",
        "#vertex #fragment fn draw[T](x: T, y: int = 2) -> int, bool return y, true end",
        "frame(screen: !$.graphics.Target, scratch: !Scratch) clear(screen!) end",
        "fn f(a: ?^[]T, b: ![2+2]u8, c: fn(?T, ^U) -> T rescue E) end",
        "fn f() x, _ := pair() y: int = 1 x = y x += 1 x -= 1 x *= 2 x /= 2 x %= 2 end",
        "fn f() if item := get() then use_item(item) elseif false then stop() else go() end end",
        "fn f() while ready() do continue end for x, _ in xs do break end end",
        "fn f() unsafe do x := &v p!^ = x^ end defer cleanup() return end",
        "fn f() return 1 unless ready() continue if skip() break if stop() end",
        "fn f() match x do A(v, _) then return v end {x=y,} then return y end end end",
        "fn f() match x do 1 then return end \"s\" then return end true then return end _ then "
        "return end end end",
        "fn f() -> int not a or b and c == d | e ~ f & g << h..<i + j * -k as int end",
        "fn f() -> T x?.foo(1, 2)[0]!^ end",
        "fn f() -> T x := {a=1, [key]=2, 3, [_]=0,} Foo[T]{a=x} end",
        "fn f() -> T g() rescue err then return fallback(err) end end",
        "fn f() -> T g() rescue 0 end",
        "fn f() -> T rescue E n := g() rescue return n end",
        "frame() g() rescue return g() rescue break g() rescue continue end",
        "fn f() -> int (1+2)*3 end",
        "fn f() -> fn(int) -> int compare[int] end",
        "fn f() f[T,U](x) f[?T, [4]u8](x) end",
        NULL,
    };
    for (size_t i = 0; valid[i] != NULL; i++) {
        (void)parse(session, valid[i], true);
    }
}

static void test_recovery(ludo_session *session) {
    static const char *const invalid[] = {
        "const = 1 const after = 2",
        "fn broken( end const after = 2",
        "fn f() x := (1 + ) end const after = 2",
        "fn f() if then end end",
        "fn f() match x do end end",
        "fn f() match x do V() then end end end",
        "fn f() match x do {} then end end end",
        "fn f() return (x += 1) end",
        "type X = Box[f().field rescue]",
        "type X = Box[$[0] rescue]",
        "const x = 1..<2..<3",
        "a+b = 1",
        "const x = $ {}",
        "type S = struct f: end",
        "pub impl S: I",
        "type X = enum A() end",
        "#align() type X = int",
        "const x = f(1,)",
        "fn f() if true then 1 + 2 end end",
        "frame() 1+2 end",
        "# explicit",
        "const x = 0x",
        "const x = \"\\q\"",
        "-- bad \xff",
        "@",
        ";",
        "fn f() const nested = 2 end const after = 3",
        NULL,
    };
    for (size_t i = 0; invalid[i] != NULL; i++) {
        (void)parse(session, invalid[i], false);
    }
    const ludo_parsed_file *file = parse(session, "fn bad( end const after = 2", false);
    LUDO_CHECK(find(file, LUDO_AST_CONST, "after") != 0);
    file = parse(session, "fn bad() x := (1 + ) end const after = 2", false);
    LUDO_CHECK(find(file, LUDO_AST_CONST, "after") != 0);
    /* A spaced marker is its own error, not an attribute on the next item. */
    file = parse(session, "# explicit const after = 2", false);
    LUDO_CHECK(file->diagnostic_count == 1);
    LUDO_CHECK(strcmp(file->diagnostics[0].message, "#explicit takes no space after #") == 0);
    LUDO_CHECK(kind_count(file, LUDO_AST_ATTRIBUTE) == 0);
    LUDO_CHECK(find(file, LUDO_AST_CONST, "after") != 0);
    /* ch7 §5.6.4: the prototype is a vendor and never emits a `ludo:` code. */
    file = parse(session, "const = \"\\q\" @", false);
    LUDO_CHECK(file->diagnostic_count >= 3); /* one syntax error, two lex errors */
    for (size_t i = 0; i < file->diagnostic_count; i++) {
        LUDO_CHECK(strncmp(file->diagnostics[i].code, "bootstrap:", 10) == 0);
    }
}

static void test_shape(ludo_session *session) {
    const ludo_parsed_file *file = parse(session, "fn f() -> int 1 + 2 * 3 - 4 end", true);
    ludo_node_id minus = find(file, LUDO_AST_BINARY, "-");
    ludo_node_id plus = ludo_ast_child(&file->ast, minus, 0);
    LUDO_CHECK(plus == find(file, LUDO_AST_BINARY, "+"));
    LUDO_CHECK(ludo_ast_child(&file->ast, plus, 1) == find(file, LUDO_AST_BINARY, "*"));
    LUDO_CHECK(kind_count(file, LUDO_AST_TAIL) == 1);
    file = parse(session, "fn f() -> int a-b-c end", true);
    ludo_node_id tail = find(file, LUDO_AST_TAIL, "a");
    ludo_node_id outer = ludo_ast_child(&file->ast, tail, 0);
    LUDO_CHECK(ludo_ast_get(&file->ast, ludo_ast_child(&file->ast, outer, 0))->kind ==
               LUDO_AST_BINARY);
    file = parse(session, "fn f() -> int n := g() rescue return n end", true);
    LUDO_CHECK(kind_count(file, LUDO_AST_TAIL) == 1);
    ludo_node_id propagation = find(file, LUDO_AST_RETURN, "return");
    LUDO_CHECK(ludo_ast_child(&file->ast, propagation, 0) == 0);
    file = parse(session, "frame() n := 1 if true then go() end n = 2 if ready() end", true);
    LUDO_CHECK(kind_count(file, LUDO_AST_IF) == 1);
    LUDO_CHECK(kind_count(file, LUDO_AST_MODIFIER) == 1);
    file = parse(session, "fn f() -> T g() end", true);
    LUDO_CHECK(kind_count(file, LUDO_AST_TAIL) == 1);
    LUDO_CHECK(kind_count(file, LUDO_AST_EXPR_STMT) == 0);
    file = parse(session, "fn f() -> T Foo{a=1} end", true);
    ludo_node_id aggregate = find(file, LUDO_AST_AGGREGATE, "{");
    LUDO_CHECK(ludo_ast_get(&file->ast, aggregate)->flags == LUDO_AST_PREFIXED);
    file = parse(session, "-- lead\n#explicit const x=1", true);
    LUDO_CHECK(kind_count(file, LUDO_AST_EXPLICIT) == 1);
    LUDO_CHECK(file->tokens[0].kind == LUDO_TOK_COMMENT);
}

/* ch1 §9.7.1 (#152): where `T rescue` ends. Each source has one fallible type,
   and the second field says whether it names its error set. */
static void test_error_set_boundary(ludo_session *session) {
    static const struct {
        const char *source;
        bool named;
    } cases[] = {
        {"fn f() -> T rescue n := g() rescue return n end", false},
        {"fn f() -> T rescue result end", false},
        {"fn f() -> T rescue E end", false},
        {"fn f() -> T rescue E return x end", true},
        {"fn f() -> T rescue E n := g() rescue return n end", true},
        {"fn f() -> T rescue $.io.IOError return x end", true},
        {"fn f() -> T rescue Box[U] return x end", true},
        {"fn f() -> T rescue E, U return x, y end", true},
        {"fn f() -> T rescue E (x) end", false},
        {"fn f() -> T rescue n - 1 end", false},
        {"fn f() -> T rescue return x end", false},
        {"type R = interface fn f() -> T rescue E end", true},
        {"type R = interface fn f() -> T rescue fn g() -> U end", false},
        {"extern \"lib\" fn f() -> T rescue E const after = 1", true},
        {"type S = struct a: T rescue b: U end", false},
        {"type S = struct a: T rescue E b: U end", true},
        {"const x: T rescue E = g()", true},
        {"fn f(a: T rescue E = g()) end", true},
    };
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        const ludo_parsed_file *file = parse(session, cases[i].source, true);
        LUDO_CHECK(kind_count(file, LUDO_AST_FALLIBLE_TYPE) == 1);
        ludo_node_id fallible = find(file, LUDO_AST_FALLIBLE_TYPE, "T");
        if (fallible == 0) {
            fallible = find(file, LUDO_AST_FALLIBLE_TYPE, "!");
        }
        bool named = ludo_ast_child(&file->ast, fallible, 1) != 0;
        if (named != cases[i].named) {
            (void)fprintf(stderr, "error set %s: %s\n", named ? "named" : "elided",
                          cases[i].source);
        }
        LUDO_CHECK(named == cases[i].named);
    }
    /* A cast's target is a PrefixType (ch1 §7.13): `rescue` after it is the
       expression's tail, never part of the type. */
    const ludo_parsed_file *file =
        parse(session, "fn f() -> int x as int rescue fallback end", true);
    LUDO_CHECK(kind_count(file, LUDO_AST_FALLIBLE_TYPE) == 0);
    LUDO_CHECK(kind_count(file, LUDO_AST_RESCUE) == 1);
}

static void test_sessions_and_limits(void) {
    ludo_session *session = NULL;
    LUDO_CHECK(ludo_session_create(&session) == LUDO_FRONTEND_OK);
    const ludo_parsed_file *first = parse(session, "const first = 1", true);
    const ludo_ast_node *original = ludo_ast_get(&first->ast, first->ast.root);
    const ludo_parsed_file *second = parse(session, "const second = 2", true);
    LUDO_CHECK(second->id != first->id);
    LUDO_CHECK(first->ast.nodes != second->ast.nodes);
    LUDO_CHECK(ludo_ast_get(&first->ast, first->ast.root) == original);
    LUDO_CHECK(ludo_ast_get(&first->ast, 0) == NULL);
    LUDO_CHECK(ludo_ast_get(&first->ast, SIZE_MAX) == NULL);
    LUDO_CHECK(ludo_ast_child(&first->ast, 0, 2) == 0);
    static char deep[6000];
    size_t count = 0;
    const char *prefix = "fn f() ";
    memcpy(deep, prefix, strlen(prefix));
    count = strlen(prefix);
    for (size_t i = 0; i < 600; i++) {
        memcpy(deep + count, "defer ", 6);
        count += 6;
    }
    memcpy(deep + count, "g() end", 8);
    count += 7;
    const ludo_parsed_file *failed = second;
    LUDO_CHECK(ludo_parse_file(session, "deep", deep, count, &failed) ==
               LUDO_FRONTEND_RESOURCE_LIMIT);
    LUDO_CHECK(failed == NULL);
    (void)parse(session, "const after = 3", true);
    ludo_session_destroy(session);
    ludo_session_destroy(NULL);
    ludo_arena arena = {0};
    LUDO_CHECK(ludo_arena_alloc(&arena, SIZE_MAX) == NULL);
    void *aligned = ludo_arena_alloc(&arena, sizeof(max_align_t));
    LUDO_CHECK((uintptr_t)aligned % _Alignof(max_align_t) == 0);
    ludo_arena_destroy(&arena);
}

static void test_json(void) {
    ludo_diagnostic diagnostic = {"bootstrap:parse-syntax", "a\"b\\c\n", {7, 13, 0}};
    static const char expected[] =
        "{\"version\":1,\"producer\":\"compile\",\"severity\":\"error\","
        "\"code\":\"bootstrap:parse-syntax\",\"primary\":{\"file\":7,\"offset\":13,\"length\":0},"
        "\"message\":\"a\\\"b\\\\c\\u000a\"}";
    char buffer[512];
    memset(buffer, 'X', sizeof buffer);
    size_t size = ludo_diagnostic_json(&diagnostic, NULL, 0);
    LUDO_CHECK(size == strlen(expected));
    LUDO_CHECK(ludo_diagnostic_json(&diagnostic, buffer, size - 1) == size);
    LUDO_CHECK(buffer[0] == 'X');
    LUDO_CHECK(ludo_diagnostic_json(&diagnostic, buffer, size) == size);
    LUDO_CHECK(memcmp(buffer, expected, size) == 0);
    LUDO_CHECK(buffer[size] == 'X');
}

LUDO_TEST_MAIN({
    ludo_session *session = NULL;
    LUDO_CHECK(ludo_session_create(&session) == LUDO_FRONTEND_OK);
    test_surface(session);
    test_recovery(session);
    test_shape(session);
    test_error_set_boundary(session);
    ludo_session_destroy(session);
    test_sessions_and_limits();
    test_json();
})
