#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

typedef struct {
    float frequency_hz;
    float duration_ms;
} Note;

typedef struct Parser Parser;

typedef struct {
    bool (*init_parser)(Parser *parser);
    bool (*get_next_note)(Parser *parser, Note *out);
} ParserVTable;

struct Parser {
    const ParserVTable *vtable;
    void* data;
};

static inline bool init_parser(Parser *parser) {
    return parser->vtable->init_parser(parser);
}

static inline bool get_next_note(Parser *parser, Note *out) {
    return parser->vtable->get_next_note(parser, out);
}

#endif
