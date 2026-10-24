#include "ludo_frontend.h"

#include <string.h>

typedef struct json_writer {
    char *buffer;
    size_t length;
} json_writer;

static void put(json_writer *writer, char byte) {
    if (writer->buffer != NULL) {
        writer->buffer[writer->length] = byte;
    }
    writer->length++;
}

static void text(json_writer *writer, const char *value) {
    while (*value != '\0') {
        put(writer, *value++);
    }
}

static void number(json_writer *writer, size_t value) {
    char digits[3 * sizeof(size_t)];
    size_t count = 0;
    do {
        digits[count++] = (char)('0' + value % 10);
        value /= 10;
    } while (value != 0);
    while (count != 0) {
        put(writer, digits[--count]);
    }
}

static void quoted(json_writer *writer, const char *value) {
    static const char hex[] = "0123456789abcdef";
    put(writer, '"');
    for (size_t i = 0; value[i] != '\0'; i++) {
        unsigned char byte = (unsigned char)value[i];
        if (byte == '"' || byte == '\\') {
            put(writer, '\\');
            put(writer, (char)byte);
        } else if (byte < 0x20) {
            text(writer, "\\u00");
            put(writer, hex[byte >> 4]);
            put(writer, hex[byte & 15]);
        } else {
            put(writer, (char)byte);
        }
    }
    put(writer, '"');
}

static void envelope(json_writer *writer, const ludo_diagnostic *diagnostic) {
    text(writer, "{\"version\":1,\"producer\":\"compile\",\"severity\":\"error\",\"code\":");
    quoted(writer, diagnostic->code);
    text(writer, ",\"primary\":{\"file\":");
    number(writer, diagnostic->primary.file);
    text(writer, ",\"offset\":");
    number(writer, diagnostic->primary.offset);
    text(writer, ",\"length\":");
    number(writer, diagnostic->primary.length);
    text(writer, "},\"message\":");
    quoted(writer, diagnostic->message);
    put(writer, '}');
}

size_t ludo_diagnostic_json(const ludo_diagnostic *diagnostic, char *buffer, size_t capacity) {
    json_writer writer = {0};
    envelope(&writer, diagnostic);
    size_t length = writer.length;
    if (buffer != NULL && capacity >= length) {
        writer = (json_writer){.buffer = buffer};
        envelope(&writer, diagnostic);
    }
    return length;
}
