/*
Copyright (c) 2024-2025 Luke Philipsen

Permission to use, copy, modify, and/or distribute this software for
any purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/* Usage

  Do this:
    #define HOXML_IMPLEMENTATION
  before you include this file in *one* C or C++ file to create the implementation.

  You can define HOXML_DECL with
    #define HOXML_DECL static
  or
    #define HOXML_DECL extern
  to specify hoxml function declarations as static or extern, respectively.
  The default specifier is extern.
*/

#ifndef HOXML_H
    #define HOXML_H

#include <stdlib.h> /* strtoul() */
#include <string.h> /* memcpy(), memset(), NULL, size_t */

#ifndef HOXML_DECL
    #define HOXML_DECL
#endif /* HOXML_DECL */

#ifdef __cplusplus
    extern "C" {
#endif /* __cpluspus */

/***************/
/* Definitions */

/**
 * Error and token codes returned after parsing.
 */
typedef enum {
    HOXML_ERROR_INVALID_INPUT = -9, /**< One or more parameter passed to hoxml was unacceptable. */
    HOXML_ERROR_INTERNAL = -8, /**< There's a bug in hoxml and parsing must halt. */
    HOXML_ERROR_INSUFFICIENT_MEMORY = -7, /**< Initialization or continued parsing require more memory. */
    HOXML_ERROR_UNEXPECTED_EOF = -6, /**< Reached the end of the XML content before the end of the document. */
    HOXML_ERROR_SYNTAX = -5, /**< Syntax error (e.g. "<element<"). */
    HOXML_ERROR_ENCODING = -4, /**< Character encoding error or contradiction. */
    HOXML_ERROR_TAG_MISMATCH = -3, /**< Close tag does not match the open tag (e.g. "<tag>" followed by "</tga>"). */
    HOXML_ERROR_INVALID_DOCUMENT_TYPE_DECLARATION = -2, /**< <!DOCTYPE> declaration not before the root element. */
    HOXML_ERROR_INVALID_DOCUMENT_DECLARATION = -1, /**< <?xml?> declaration not before the root element. */
    HOXML_END_OF_DOCUMENT = 0, /**< The root element has been closed, parsing is done. */
    HOXML_ELEMENT_BEGIN, /**< A new element/tag began and its name is available. */
    HOXML_ELEMENT_END, /**< An element was closed, </tag> or <tag/>, and its name and content are available. */
    HOXML_ATTRIBUTE, /**< An attribute's value, its name, and its element are available. */
    HOXML_PROCESSING_INSTRUCTION_BEGIN, /**< A processing instruction began and its target is available. */
    HOXML_PROCESSING_INSTRUCTION_END /**< A processing instruction ended and its content is available. */
} hoxml_code_t;

/**
 * Holds context and state information needed by hoxml. Some of this information is public and holds the data parsed
 * from XML content (element names, attribute names and values, etc.) but some is private and only makes sense to hoxml.
 */
typedef struct {
    /* Public */
    char* tag; /**< Holds the name of the open or just-closed tag, or processing instruction target. */
    char* attribute; /**< Holds the current attribute's name. */
    char* value; /**< Holds the current attribute's value. */
    char* content; /**< Holds the current element's content. This means all character data found, including spaces. */
    int line; /**< The line currently being parsed. Lines are determined by line feeds and carriage returns. */
    int column; /**< The column, on the current line, of the character last parsed. */
    int depth; /**< The nested level of elements. Assigned with the level in which the element was found. */

    /* Private (for internal use) */
    int is_initialized; /* Set to 1, or true, by hoxml_init() and indicates this context is safe to use */
    const char* xml; /* XML content to be parsed */
    size_t xml_length; /* Length of the XML content to parse */
    int encoding; /* Character encoding of the XML content */
    const char* iterator; /* Pointer to the character in the XML content being parsed */
    char* buffer; /* Memory allocated for hoxml to use */
    size_t buffer_length; /* Amount of memory allocated for hoxml */
    char* reference_start; /* Pointer to a location on the stack where a reference entity string (e.g "&lt;") began */
    char* stack; /* Pointer to the current node in the stack-like structure of elements */
    int state; /* Current parsing state, determines which characters are acceptable and when to return */
    int post_state; /* When not "none" this indicates a post-state that has a cleanup step */
    int return_state; /* State to return to after the processing of a comment or reference has finished */
    int error_return_state; /* State to return to after recovering from an error */
    unsigned long stream; /* Holds the current character, whole or partial. May contain bytes from different strings. */
    size_t stream_length; /* Length of the 'stream' variable in bytes */
    unsigned newline_character; /* The character used to increment the 'line' variable, \r or \n */
} hoxml_context_t;

/**
 * Sets up the hoxml context object to begin parsing. Following this, call hoxml_parse() until
 * HOXML_END_OF_DOCUMENT or one of the error values is returned.
 *
 * @param context Pointer to an allocated hoxml context object. This instance will be modified.
 * @param buffer A pointer to some contiguous block of memory for hoxml to use. This will also be modified, frequently.
 * @param buffer_length The length, in bytes, of the buffer handed to hoxml as the 'buffer' parameter.
 */
HOXML_DECL void hoxml_init(hoxml_context_t* context, void* buffer, size_t buffer_length);

/**
 * Instruct hoxml to use a new buffer. This maintains the current state of parsing meaning that the next call to
 * hoxml_parse() will continue none the wiser.
 * The buffer must have a length greater than the current buffer and both buffers must be allocated at the time this
 * function is called. Once it returns, the original buffer may and should be freed.
 *
 * @param context An initialized hoxml context object.
 * @param buffer A pointer to a new, contiguous block of memory for hoxml to use.
 * @param buffer_length The length, in bytes, of the buffer handed to hoxml as the 'buffer' parameter.
 */
HOXML_DECL void hoxml_realloc(hoxml_context_t* context, void* buffer, size_t buffer_length);

/**
 * Begin or continue parsing the given XML content string.
 * The XML content string does not need to contain the content in its entirety. If hoxml finds a null terminator or
 * parses up to the indicated length of the content, HOXML_ERROR_UNEXPECTED_EOF is returned and parsing will cease.
 * However, this error is recoverable and parsing will continue if the next call to hoxml_parse() passes a new XML
 * content string, using the same pointer or not.
 *
 * @param context An initialized hoxml context object. This should be treated as read-only until parsing is done.
 * @param xml XML content as an encoded string. Supported character encodings include ASCII, UTF-8, and UTF-16(BE|LE).
 * @param xml_length Length of the XML content in bytes.
 * @return A code indicating what information from the XML content is available or an error.
 */
HOXML_DECL hoxml_code_t hoxml_parse(hoxml_context_t* context, const char* xml, size_t xml_length);

#ifdef __cplusplus
    }
#endif /* __cplusplus */

#ifdef HOXML_IMPLEMENTATION

/******************/
/* Implementation */

enum {
    /* Current parser states */
    HOXML_STATE_ERROR_INTERNAL = -8,
    HOXML_STATE_ERROR_INSUFFICIENT_MEMORY = -7,
    HOXML_STATE_ERROR_UNEXPECTED_EOF = -6,
    HOXML_STATE_ERROR_SYNTAX = -5,
    HOXML_STATE_ERROR_ENCODING = -4,
    HOXML_STATE_ERROR_TAG_MISMATCH = -3,
    HOXML_STATE_ERROR_INVALID_DOCUMENT_TYPE_DECLARATION = -2,
    HOXML_STATE_ERROR_INVALID_DOCUMENT_DECLARATION = -1,
    HOXML_STATE_NONE = 0,
    HOXML_STATE_UTF8_BOM1,
    HOXML_STATE_UTF8_BOM2,
    HOXML_STATE_UTF16BE_BOM,
    HOXML_STATE_UTF16LE_BOM,
    HOXML_STATE_TAG_BEGIN,
    HOXML_STATE_TAG_END,
    HOXML_STATE_ELEMENT_NAME1,
    HOXML_STATE_ELEMENT_NAME2,
    HOXML_STATE_ATTRIBUTE_NAME1,
    HOXML_STATE_ATTRIBUTE_NAME2,
    HOXML_STATE_ATTRIBUTE_ASSIGNMENT,
    HOXML_STATE_ATTRIBUTE_VALUE,
    HOXML_STATE_OPEN_TAG,
    HOXML_STATE_COMMENT_CDATA_OR_DTD_BEGIN,
    HOXML_STATE_COMMENT_BEGIN,
    HOXML_STATE_COMMENT,
    HOXML_STATE_COMMENT_END1,
    HOXML_STATE_COMMENT_END2,
    HOXML_STATE_CDATA_BEGIN1,
    HOXML_STATE_CDATA_BEGIN2,
    HOXML_STATE_CDATA_BEGIN3,
    HOXML_STATE_CDATA_BEGIN4,
    HOXML_STATE_CDATA_BEGIN5,
    HOXML_STATE_CDATA_BEGIN6,
    HOXML_STATE_CDATA_CONTENT,
    HOXML_STATE_CDATA_END1,
    HOXML_STATE_CDATA_END2,
    HOXML_STATE_REFERENCE_BEGIN,
    HOXML_STATE_REFERENCE_ENTITY,
    HOXML_STATE_REFERENCE_NUMERIC,
    HOXML_STATE_REFERENCE_HEX,
    HOXML_STATE_PROCESSING_INSTRUCTION_BEGIN,
    HOXML_STATE_PROCESSING_INSTRUCTION_TARGET1,
    HOXML_STATE_PROCESSING_INSTRUCTION_TARGET2,
    HOXML_STATE_PROCESSING_INSTRUCTION_CONTENT,
    HOXML_STATE_PROCESSING_INSTRUCTION_END,
    HOXML_STATE_DTD_BEGIN1,
    HOXML_STATE_DTD_BEGIN2,
    HOXML_STATE_DTD_BEGIN3,
    HOXML_STATE_DTD_BEGIN4,
    HOXML_STATE_DTD_BEGIN5,
    HOXML_STATE_DTD_BEGIN6,
    HOXML_STATE_DTD_BEGIN7,
    HOXML_STATE_DTD_BEGIN8,
    HOXML_STATE_DTD_NAME,
    HOXML_STATE_DTD_CONTENT,
    HOXML_STATE_DTD_OPEN_BRACKET,
    HOXML_STATE_DONE,
    /* Post (i.e. after) parser states indicating actions to take on the next call to hoxml_parse() */
    HOXML_POST_STATE_TAG_END,
    HOXML_POST_STATE_ATTRIBUTE_END,
};

enum {
    HOXML_FLAG_END_TAG = 1, /* The node is a dedicated end tag (not an empty element) */
    HOXML_FLAG_EMPTY_ELEMENT = 2, /* The node is an empty element */
    HOXML_FLAG_PROCESSING_INSTRUCTION = 4, /* The node is a processing instruction */
    HOXML_FLAG_DOUBLE_QUOTE = 8, /* The value string being parsed was opened with a double quote (") */
    HOXML_FLAG_TERMINATED = 16, /* The node's current string (tag, attribute, etc.) is null terminated */
    HOXML_FLAG_BEGUN = 32, /* The "element begun" code was already returned for this node */
    HOXML_FLAG_INCREMENT_DEPTH = 64, /* Context object's depth value should increase by one next hoxml_parse() */
    HOXML_FLAG_DECREMENT_DEPTH = 128 /* Context object's depth value should decrease by one next hoxml_parse() */
};

enum {
    HOXML_ENC_UNKNOWN = 0, /* The character encoding is unknown. UTF-8 is assumed. */
    HOXML_ENC_UTF_8, /* Variable-length encoding (8, 16, 24, or 32 bits) compatible with ASCII */
    HOXML_ENC_UTF_16_LE, /* Variable-length encoding (16 or 32 bits), little-endian variant */
    HOXML_ENC_UTF_16_BE /* Variable-lenght encoding (16 or 32 bits), big-endian variant */
};

enum {
    HOXML_CASE_SENSITIVE = 0, /* Cases must match. 'A' == 'a' -> false. */
    HOXML_CASE_INSENSITIVE /* Cases need not match. 'A' == 'a' -> true. */
};

enum {
    HOXML_REF_TYPE_ENTITY = 0, /* Predefined strings representing known, problematic characters (e.g. '<') */
    HOXML_REF_TYPE_NUMERIC, /* A value of a character given as a decimal number */
    HOXML_REF_TYPE_HEX /* A value of a character given as a hexadecimal number */
};

struct _hoxml_node_t;
typedef struct _hoxml_node_t {
    struct _hoxml_node_t* parent; /* Points to the parent node, or NULL if this is the root */
    char* end; /* Points to the last byte of this node's data */
    int flags; /* May contain any number of the flags defined in hoxml_node_flags */
    char tag; /* Where the tag string will be stored in the buffer, must be defined last */
} hoxml_node_t;

typedef struct {
    unsigned encoded; /* Character as it appeared in the content. In other words, the original, encoded character. */
    unsigned codepoint; /* Unicode codepoint of the character. In other words, the decoded character. */
    size_t bytes; /* Number of eight-bit bytes of the encoded character, in the [1, 4] range */
} hoxml_character_t;

#ifndef UINT32_MAX /* Defined in stdint.h with later revisions of C and C++ but not for some earlier ones */
    #define UINT32_MAX (0xffffffff)
#endif
#define HOXML_STACK ((hoxml_node_t*)context->stack)
#define HOXML_TO_LOWER(c) (c >= 'A' && c <= 'Z' ? c + 32 : c)
#define HOXML_IS_NEW_LINE(c) (c == 0x0A || c == 0x0D)
#define HOXML_IS_WHITESPACE(c) (c == 0x20 || c == 0x09 || HOXML_IS_NEW_LINE(c))
#define HOXML_IS_ASCII_CHAR(c) (c >= 0x21 && c <= 0x7F)
#define HOXML_IS_CHAR_DATA(c) (c != '<' && c != '&')
#define HOXML_IS_ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define HOXML_IS_NUMERIC(c) (c >= '0' && c <= '9')
#define HOXML_IS_NAME_START_CHAR(c) (HOXML_IS_ALPHA(c) || c == ':' || c == '_' || (c >= 0xC0 && c <= 0xD6) || \
    (c >= 0xD8 && c <= 0xF6) || c >= 0xF8)
#define HOXML_IS_NAME_CHAR(c) (HOXML_IS_NAME_START_CHAR(c) || c == '-' || c == '.'|| HOXML_IS_NUMERIC(c))
#define HOXML_IS_HEX_CHAR(c) (HOXML_IS_NUMERIC(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
#define HOXML_IS_VALUE_CHAR_DATA(f, c) (HOXML_IS_CHAR_DATA(c) && ((f & HOXML_FLAG_DOUBLE_QUOTE && c != '"') || \
    c != '\''))

void hoxml_push_stack(hoxml_context_t* context);
void hoxml_pop_stack(hoxml_context_t* context);
void hoxml_append_character(hoxml_context_t* context, hoxml_character_t c);
void hoxml_append_terminator(hoxml_context_t* context);
void hoxml_end_reference(hoxml_context_t* context, int type);
void hoxml_begin_tag(hoxml_context_t* context);
hoxml_code_t hoxml_end_tag(hoxml_context_t* context);
int hoxml_post_state_cleanup(hoxml_context_t* context);
hoxml_character_t hoxml_decode_character(const char* str, size_t str_length, int encoding);
hoxml_character_t hoxml_encode_character(unsigned codepoint, int encoding);
char* hoxml_to_ascii(const char* str, int encoding);
size_t hoxml_strlen(const char* str, int encoding);
int hoxml_strcmp(const char* str1, int encoding1, const char* str2, int encoding2, int sensitivity);
const char* hoxml_strstr(const char* haystack, int haystack_encoding, const char* needle, int needle_encoding,
    int sensitivity);
#ifdef HOXML_DEBUG
    #include <stdio.h> /* printf() */
    #define HOXML_LOG_STATE(s) printf("%s\n", s);
#else
    #define HOXML_LOG_STATE(s)
#endif

HOXML_DECL void hoxml_init(hoxml_context_t* context, void* buffer, size_t buffer_length) {
    if (context == NULL || buffer == NULL || buffer_length <= 0)
        return;

    memset(context, 0, sizeof(hoxml_context_t)); /* Assign all values of the context to zero */
    context->buffer = (char*)buffer; /* Use the provided buffer */
    context->buffer_length = buffer_length; /* Remember the length of the provided buffer */
    context->line = 1; /* This is meant to be human-readable and humans begin counting at one */
    context->is_initialized = 1;
    memset(buffer, 0, buffer_length); /* Fill the buffer with zeroes */
}

HOXML_DECL void hoxml_realloc(hoxml_context_t* context, void* buffer, size_t buffer_length) {
    if (context == NULL || context->is_initialized == 0 || buffer == NULL || buffer_length <= context->buffer_length)
        return;

    /* Reassign the end and parent pointers of each node, beginning at the tail and iterate to the head */
    hoxml_node_t* node = HOXML_STACK;
    while (node != NULL) {
        hoxml_node_t* parent = node->parent;
        node->end = (char*)buffer + (node->end - context->buffer);
        if (node->parent != NULL)
            node->parent = (hoxml_node_t*)((char*)buffer + ((char*)node->parent - context->buffer));
        node = parent;
    }

    /* Use offsets from the original buffer pointer to reassign pointers such that they now point to the new buffer */
    if (context->tag != NULL)
        context->tag = (char*)buffer + (context->tag - context->buffer);
    if (context->attribute != NULL)
        context->attribute = (char*)buffer + (context->attribute - context->buffer);
    if (context->value != NULL)
        context->value = (char*)buffer + (context->value - context->buffer);
    if (context->content != NULL)
        context->content = (char*)buffer + (context->content - context->buffer);
    if (context->reference_start != NULL)
        context->reference_start = (char*)buffer + (context->reference_start - context->buffer);
    if (context->stack != NULL)
        context->stack = (char*)buffer + (context->stack - context->buffer);

    memset(buffer, 0, buffer_length); /* Fill the new buffer with zeroes */
    memcpy(buffer, context->buffer, context->buffer_length); /* Copy the entire, current buffer to the new buffer */
    context->buffer = (char*)buffer;
    context->buffer_length = buffer_length;

    if (context->state == HOXML_STATE_ERROR_INSUFFICIENT_MEMORY) {
        context->state = context->error_return_state;
        context->error_return_state = HOXML_STATE_NONE;
    }
}

HOXML_DECL hoxml_code_t hoxml_parse(hoxml_context_t* context, const char* xml, const size_t xml_length) {
    if (context == NULL || context->is_initialized == 0 || xml == NULL || xml_length == 0)
        return HOXML_ERROR_INVALID_INPUT;

    if (HOXML_STACK != NULL) {
        if (HOXML_STACK->flags & HOXML_FLAG_INCREMENT_DEPTH) { /* If an element began, increasing nesting */
            context->depth += 1;
            HOXML_STACK->flags &= ~HOXML_FLAG_INCREMENT_DEPTH; /* Clear the flag */
        }

        if (HOXML_STACK->flags & HOXML_FLAG_DECREMENT_DEPTH) { /* If an element ended, decreasing nesting */
            context->depth -= 1;
            HOXML_STACK->flags &= ~HOXML_FLAG_DECREMENT_DEPTH; /* Clear the flag */
        }
    }

    switch (context->state) {
        /* Two errors are recoverable: HOXML_ERROR_INSUFFICIENT_MEMORY and HOXML_ERROR_UNEXPECTED_EOF. The former can */
        /* be recovered by assigning a new buffer with hoxml_realloc(). The latter can be recovered by passing a new */
        /* XML content string to hoxml_parse() so we'll check for one before concluding we're still in error. */
        case HOXML_STATE_ERROR_UNEXPECTED_EOF: {
            /* Try to decode a character, or remainder of a character, at the beginning of this hopefully-new string */
            unsigned long stream = context->stream;
            /* Calculate the number of bytes to copy into the 'stream' variable from the hopefully-new string. We */
            /* want 4 bytes, or whatever is available. */
            size_t bytes_to_copy = 4;
            if (bytes_to_copy > xml_length)
                bytes_to_copy = xml_length;
            if (context->stream_length > 0) {
                /* Adjust the number of bytes to copy to account for possible bytes from a previous string */
                bytes_to_copy -= context->stream_length;
                /* Append the new bytes to the previous one(s) */
                memcpy((char*)&stream + context->stream_length, xml, bytes_to_copy);
            }
            else {
                /* Copy to the 'stream' under the assumption that all of it can be overwritten */
                memcpy(&stream, xml, bytes_to_copy);
            }
            hoxml_character_t c = hoxml_decode_character((const char*)&stream, xml_length, context->encoding);
            /* If the character is the equivalent of a null terminator or there was not enough data */
            if (c.codepoint == 0 || c.codepoint == UINT32_MAX)
                return HOXML_ERROR_UNEXPECTED_EOF;
            context->state = context->error_return_state;
            context->error_return_state = HOXML_STATE_NONE;
            /* Note: there is a check for a change in the input pointer a little further down */
        } break;
        case HOXML_STATE_DONE: return HOXML_END_OF_DOCUMENT;
        case HOXML_STATE_ERROR_INTERNAL: return HOXML_ERROR_INTERNAL;
        case HOXML_STATE_ERROR_INSUFFICIENT_MEMORY: return HOXML_ERROR_INSUFFICIENT_MEMORY;
        case HOXML_STATE_ERROR_SYNTAX: return HOXML_ERROR_SYNTAX;
        case HOXML_STATE_ERROR_ENCODING: return HOXML_ERROR_ENCODING;
        case HOXML_STATE_ERROR_TAG_MISMATCH: return HOXML_ERROR_TAG_MISMATCH;
        case HOXML_STATE_ERROR_INVALID_DOCUMENT_DECLARATION: return HOXML_ERROR_INVALID_DOCUMENT_DECLARATION;
    }

    /* A handful of cases leave the context in an intermediary state. This allows the caller to have access to things */
    /* like the tag's name, an attribute's value, etc. but that old data may now need to be cleaned up. */
    if (hoxml_post_state_cleanup(context)) /* If the cleanup process found the document ended */
        return HOXML_END_OF_DOCUMENT;

    /* If the pointer to the XML content string has changed */
    if (context->xml != xml) {
        /* A few variables are now invalid: the pointer to the content, its length, and the iterator */
        context->xml = xml;
        context->xml_length = xml_length;
        context->iterator = xml;
    }

    /* Remember some context variables in case we hit an unexpected EoF and need to undo an iteration */
    const char* previous_iterator = context->iterator;
    size_t previous_stream_length = context->stream_length;
    while (context->state >= HOXML_STATE_NONE && context->state <= HOXML_STATE_DONE) {
        /* About half of the parsing states assume the stack is non-null. */
        /* If parsing is currently in one of those states and the stack (head) pointer is null. */
        if (((context->state >= HOXML_STATE_TAG_BEGIN && context->state <= HOXML_STATE_OPEN_TAG) ||
                (context->state >= HOXML_STATE_REFERENCE_BEGIN && context->state <= HOXML_STATE_REFERENCE_HEX)) &&
                context->stack == NULL) {
            /* Some unforseen bug has led us to a state in which continuing would cause an illegal memory access. */
            /* Parsing must halt. There is no way to recover. */
            context->state = HOXML_STATE_ERROR_INTERNAL;
            return HOXML_ERROR_INTERNAL;
        }

        /* Calculate the number of bytes remaining in the current XML content string */
        size_t bytes_remaining = (size_t)(context->xml_length - (context->iterator - context->xml));
        /* Calculate the number of bytes to copy into the 'stream' variable. We want 4 bytes, or whatever is left. */
        size_t bytes_to_copy = 4;
        if (bytes_to_copy > bytes_remaining)
            bytes_to_copy = bytes_remaining;
        if (context->stream_length > 0) {
            /* Adjust the number of bytes to copy to account for possible bytes from a previous XML content string. */
            /* This will be non-zero in the rare case where content is being given in parts. */
            bytes_to_copy -= context->stream_length;
            /* Append the new bytes to the previous one(s) */
            memcpy((char*)&(context->stream) + context->stream_length, context->iterator, bytes_to_copy);
        }
        else {
            /* Copy to the 'stream' under the assumption that all of it can be overwritten */
            memcpy(&(context->stream), context->iterator, bytes_to_copy);
        }
        hoxml_character_t c = hoxml_decode_character((const char*)&(context->stream), bytes_remaining,
            context->encoding);

        /* If the character is the equivalent of a null terminator or there was not enough data to decode the value */
        if (c.codepoint == 0 || c.codepoint == UINT32_MAX) {
            context->stream_length = bytes_to_copy;
            context->error_return_state = context->state;
            context->state = HOXML_STATE_ERROR_UNEXPECTED_EOF;
            return HOXML_ERROR_UNEXPECTED_EOF;
        } else if (HOXML_IS_NEW_LINE(c.codepoint)) {
            if (context->newline_character == 0) /* If this is the first newline */
                context->newline_character = c.codepoint; /* Remember this as the character to use for increments */
            if (c.codepoint == context->newline_character) /* Avoid incrementing twice for files with \r\n endings */
                context->line++;
            context->column = 0;
        } else
            context->column++;

        /* Iterate up to four bytes into the XML content string. The idea is to jump forward by the number of bytes */
        /* that were just decoded as a single character. The number of bytes varies from one to four bytes depending */
        /* on the character encoding and character's codepoint. We also need to consider the case in which some of */
        /* this character's bytes were carried over from a previous XML content string. Those bytes would have been */
        /* stashed in the context's 'stream' variable where 'stream_length' tells us the number of said bytes. */
        previous_iterator = context->iterator;
        previous_stream_length = context->stream_length;
        context->iterator += c.bytes - context->stream_length;
        context->stream_length = 0;

        #ifdef HOXML_DEBUG
            char debugCodepoint = HOXML_IS_NEW_LINE(c.codepoint) ? ' ' : c.codepoint;
            printf("  %c [%08X] [L%02dC%02d] -> ", debugCodepoint, c.codepoint, context->line, context->column);
        #endif

        switch(context->state) {
        case HOXML_STATE_NONE: /* The first state immediately following initialization, or a document declaration */
            HOXML_LOG_STATE("HOXML_STATE_NONE")
            if (c.codepoint == '<')
                hoxml_begin_tag(context);
            else if (c.encoded == 0xEF) { /* UTF-8 Byte Order Marker (BOM) is [EF] BB BF, as hex bytes */
                context->state = HOXML_STATE_UTF8_BOM1;
                context->column--; /* Don't count this as a column */
            } else if (c.encoded == 0xFE) { /* UTF-16BE BOM is [FE] FF, as hex bytes */
                context->state = HOXML_STATE_UTF16BE_BOM;
                context->column--; /* Don't count this as a column */
            } else if (c.encoded == 0xFF) { /* UTF-16LE BOM is [FF] FE, as hex bytes */
                context->state = HOXML_STATE_UTF16LE_BOM;
                context->column--; /* Don't count this as a column */
            } else if (!HOXML_IS_WHITESPACE(c.codepoint))
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_UTF8_BOM1: /* The first byte of a UTF-8 byte order marker was found */
            HOXML_LOG_STATE("HOXML_STATE_UTF8_BOM1")
            context->column--; /* Don't count this as a column */
            if (c.encoded == 0xBB) /* UTF-8 BOM is EF [BB] BF, as hex bytes */
                context->state = HOXML_STATE_UTF8_BOM2;
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_UTF8_BOM2: /* The second byte of a UTF-8 byte order marker was found */
            HOXML_LOG_STATE("HOXML_STATE_UTF8_BOM2")
            context->column--; /* Don't count this as a column */
            if (c.encoded == 0xBF) { /* UTF-8 BOM is EF BB [BF], as hex bytes */
                context->state = HOXML_STATE_NONE;
                context->encoding = HOXML_ENC_UTF_8;
            } else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_UTF16BE_BOM: /* The first byte of a UTF-16BE byte order marker was found */
            HOXML_LOG_STATE("HOXML_STATE_UTF16BE_BOM")
            context->column--; /* Don't count this as a column */
            if (c.encoded == 0xFF) { /* UTF-16BE BOM is FE [FF], as hex bytes */
                context->state = HOXML_STATE_NONE;
                context->encoding = HOXML_ENC_UTF_16_BE;
            } else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_UTF16LE_BOM: /* The first byte of a UTF-16LE byte order marker was found */
            HOXML_LOG_STATE("HOXML_STATE_UTF16LE_BOM")
            context->column--; /* Don't count this as a column */
            if (c.encoded == 0xFE) { /* UTF-16LE BOM is FF [FE], as hex bytes */
                context->state = HOXML_STATE_NONE;
                context->encoding = HOXML_ENC_UTF_16_LE;
            } else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_TAG_BEGIN: /* A new tag was started (a '<' was found) and a new node has been pushed */
            HOXML_LOG_STATE("HOXML_STATE_TAG_BEGIN")
            if (c.codepoint == '?') { /* "<?" begins a processing instruction */
                context->state = HOXML_STATE_PROCESSING_INSTRUCTION_BEGIN;
                HOXML_STACK->flags |= HOXML_FLAG_PROCESSING_INSTRUCTION; /* Apply the PI flag to this node */
            } else if (c.codepoint == '/') /* "</" begins an end tag */
                HOXML_STACK->flags |= HOXML_FLAG_END_TAG; /* Apply the end tag flag to this node */
            else if (c.codepoint == '!') /* "<!--" = comment, "<![CDATA[" = CDATA, and "<!DOCTYPE" = DTD */
                context->state = HOXML_STATE_COMMENT_CDATA_OR_DTD_BEGIN;
            else if (HOXML_IS_NAME_START_CHAR(c.codepoint)) {
                hoxml_append_character(context, c);
                if (context->state >= HOXML_STATE_NONE) { /* If appending the character was successful */
                    context->state = HOXML_STATE_ELEMENT_NAME1;
                    context->tag = &(HOXML_STACK->tag); /* The tag's name string will begin here */
                }
            } else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_ELEMENT_NAME1: /* A name start character was found after '<' (e.g. the 't' in "<tag>") */
            HOXML_LOG_STATE("HOXML_STATE_ELEMENT_NAME1")
            if (c.codepoint == '>') {
                hoxml_append_terminator(context);
                if (context->state >= HOXML_STATE_NONE) /* If appending the terminator was successful */
                    return hoxml_end_tag(context);
            } else if (c.codepoint == '/') { /* The tag is an empty element, AKA self-closed tag (e.g. "<tag/>") */
                if (HOXML_STACK->flags & HOXML_FLAG_END_TAG) /* If it's also a regular close tag (e.g. "</tag/>") */
                    context->state = HOXML_STATE_ERROR_SYNTAX;
                else {
                    hoxml_append_terminator(context);
                    if (context->state >= HOXML_STATE_NONE) { /* If appending the terminator was successful */
                        HOXML_STACK->flags |= HOXML_FLAG_EMPTY_ELEMENT; /* Apply the empty element flag */
                        return HOXML_ELEMENT_BEGIN;
                    }
                }
            } else if (HOXML_IS_WHITESPACE(c.codepoint)) { /* If whitespace ended the element name (e.g. "<tag ") */
                hoxml_append_terminator(context);
                if (context->state >= HOXML_STATE_NONE) { /* If appending the terminator was successful */
                    context->state = HOXML_STATE_ELEMENT_NAME2;
                    HOXML_STACK->flags |= HOXML_FLAG_BEGUN; /* Indicate "element begun" has already been returned */
                    return HOXML_ELEMENT_BEGIN;
                }
            } else if (HOXML_IS_NAME_CHAR(c.codepoint))
                hoxml_append_character(context, c);
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_ELEMENT_NAME2: /* Whitespace was found after a tag name (e.g. "<tag    >") */
            HOXML_LOG_STATE("HOXML_STATE_ELEMENT_NAME2")
            if (c.codepoint == '>') {
                hoxml_append_terminator(context);
                if (context->state >= HOXML_STATE_NONE) { /* If appending the terminator was successful */
                    /* If "element begun" has already been returned for this element and this element is not */
                    /* self-closing (i.e. the element has the form "<tag ... />" where "..." may be attributes) */
                    if (HOXML_STACK->flags & HOXML_FLAG_BEGUN &&
                            !(HOXML_STACK->flags & HOXML_FLAG_EMPTY_ELEMENT)) {
                        hoxml_end_tag(context); /* Do not return, "element begun" was returned when the name ended */
                        hoxml_post_state_cleanup(context); /* Because hoxml_parse() won't be called, clean up now */
                    } else
                        return hoxml_end_tag(context);
                }
            } else if (c.codepoint == '/') { /* The tag is an empty element, AKA self-closed tag (e.g. "<tag/>") */
                if (HOXML_STACK->flags & HOXML_FLAG_END_TAG) /* If it's also a regular close tag (e.g. "</tag/>") */
                    context->state = HOXML_STATE_ERROR_SYNTAX;
                else
                    HOXML_STACK->flags |= HOXML_FLAG_EMPTY_ELEMENT; /* Apply the empty element flag to this node */
            } else if (HOXML_IS_NAME_START_CHAR(c.codepoint)) { /* First letter of an attribute name */
                hoxml_append_character(context, c);
                if (context->state >= HOXML_STATE_NONE) { /* If appending the character was successful */
                    context->state = HOXML_STATE_ATTRIBUTE_NAME1;
                    context->attribute = HOXML_STACK->end; /* The attribute's name string began here */
                }
            } else if (!HOXML_IS_WHITESPACE(c.codepoint))
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_ATTRIBUTE_NAME1: /* A name start character was found inside a tag after whitespace */
            HOXML_LOG_STATE("HOXML_STATE_ATTRIBUTE_NAME1")
            if (c.codepoint == '=') { /* The name was immediately followed by '=' */
                hoxml_append_terminator(context);
                if (context->state >= HOXML_STATE_NONE) /* If appending the terminator was successful */
                    context->state = HOXML_STATE_ATTRIBUTE_ASSIGNMENT;
            } else if (HOXML_IS_NAME_CHAR(c.codepoint))
                hoxml_append_character(context, c);
            else if (HOXML_IS_WHITESPACE(c.codepoint)) { /* Whitespace after the name, only '=' is allowed next */
                hoxml_append_terminator(context);
                if (context->state >= HOXML_STATE_NONE) /* If appending the terminator was successful */
                    context->state = HOXML_STATE_ATTRIBUTE_NAME2;
            } else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_ATTRIBUTE_NAME2: /* Whitespace was found after an attribute name, look for '=' */
            HOXML_LOG_STATE("HOXML_STATE_ATTRIBUTE_NAME2")
            if (c.codepoint == '=')
                context->state = HOXML_STATE_ATTRIBUTE_ASSIGNMENT;
            else if (!HOXML_IS_WHITESPACE(c.codepoint)) /* Only '=' and whitespace are allowed after an attribute name */
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_ATTRIBUTE_ASSIGNMENT: /* Found a ' =' after an attribute name, look for quotes or whitespace */
            HOXML_LOG_STATE("HOXML_STATE_ATTRIBUTE_ASSIGNMENT")
            if (c.codepoint == '"' || c.codepoint == '\'') {
                context->state = HOXML_STATE_ATTRIBUTE_VALUE;
                if (c.codepoint == '"')
                    HOXML_STACK->flags |= HOXML_FLAG_DOUBLE_QUOTE; /* Apply the double quote flag to this node */
                else
                    HOXML_STACK->flags &= ~HOXML_FLAG_DOUBLE_QUOTE; /* Remove the double quote flag from this node */
                context->value = HOXML_STACK->end + 1; /* The attribute's value string will begin here */
            }
            else if (!HOXML_IS_WHITESPACE(c.codepoint))
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_ATTRIBUTE_VALUE: /* A quotation, single or double, was found after an attribute name and '=' */
            HOXML_LOG_STATE("HOXML_STATE_ATTRIBUTE_VALUE")
            if ((HOXML_STACK->flags & HOXML_FLAG_DOUBLE_QUOTE && c.codepoint == '"') || (!(HOXML_STACK->flags &
                    HOXML_FLAG_DOUBLE_QUOTE) && c.codepoint == '\'')) { /* The quotation marks match, value is done */
                hoxml_append_terminator(context);
                if (context->state >= HOXML_STATE_NONE) { /* If appending the terminator was successful */
                    context->state = HOXML_STATE_ELEMENT_NAME2;
                    context->post_state = HOXML_POST_STATE_ATTRIBUTE_END; /* Clean up some attribute things next call */
                    return HOXML_ATTRIBUTE;
                }
            } else if (c.codepoint == '&') {
                context->state = HOXML_STATE_REFERENCE_BEGIN;
                context->return_state = HOXML_STATE_ATTRIBUTE_VALUE; /* Return to this attribute value state later */
            } else if (HOXML_IS_VALUE_CHAR_DATA(HOXML_STACK->flags, c.codepoint))
                hoxml_append_character(context, c);
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_OPEN_TAG: /* Found a '>' and now inside an open tag, looking for multiple things */
            HOXML_LOG_STATE("HOXML_STATE_OPEN_TAG")
            if (c.codepoint == '<')
                hoxml_begin_tag(context);
            else if (c.codepoint == '&') {
                context->state = HOXML_STATE_REFERENCE_BEGIN;
                context->return_state = HOXML_STATE_OPEN_TAG; /* Return to this open tag state later */
            } else if (HOXML_IS_CHAR_DATA(c.codepoint))
                hoxml_append_character(context, c);
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_COMMENT_CDATA_OR_DTD_BEGIN: /* Found "<!", looking for a '-', '[', or 'D' */
            HOXML_LOG_STATE("HOXML_STATE_COMMENT_CDATA_OR_DTD_BEGIN")
            if (c.codepoint == '-') /* Possible beginning of a comment (i.e. "<!--") */
                context->state = HOXML_STATE_COMMENT_BEGIN;
            else if (c.codepoint == '[') /* Possible beginning of a CDATA section (i.e. "<![CDATA[") */
                context->state = HOXML_STATE_CDATA_BEGIN1;
            else if (c.codepoint == 'D') { /* Possible beginning of a DTD (i.e. "<!DOCTYPE") */
                if (context->return_state != HOXML_STATE_NONE) { /* If this DTD was found after a root element */
                    context->state = HOXML_STATE_ERROR_INVALID_DOCUMENT_TYPE_DECLARATION;
                    return HOXML_ERROR_INVALID_DOCUMENT_TYPE_DECLARATION;
                } else
                    context->state = HOXML_STATE_DTD_BEGIN1;
            } else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_COMMENT_BEGIN: /* Found a '-' was found after "<!", looking for a '-' beginning a comment */
            HOXML_LOG_STATE("HOXML_STATE_COMMENT_BEGIN")
            hoxml_pop_stack(context); /* The preceeding '<' triggered a new node. Undo it. */
            if (c.codepoint == '-')
                context->state = HOXML_STATE_COMMENT;
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_COMMENT: /* Found a second '-' and now in a comment, looking for '-' */
            HOXML_LOG_STATE("HOXML_STATE_COMMENT")
            if (c.codepoint == '-')
                context->state = HOXML_STATE_COMMENT_END1;
            else
                context->state = HOXML_STATE_COMMENT;
            break;
        case HOXML_STATE_COMMENT_END1: /* Found a '-' while in a comment, looking for a second '-' */
            HOXML_LOG_STATE("HOXML_STATE_COMMENT_END1")
            if (c.codepoint == '-')
                context->state = HOXML_STATE_COMMENT_END2;
            else
                context->state = HOXML_STATE_COMMENT;
            break;
        case HOXML_STATE_COMMENT_END2: /* Found a second '-' while in a comment, looking for '>' to end the comment */
            HOXML_LOG_STATE("HOXML_STATE_COMMENT_END2")
            if (c.codepoint == '>')
                context->state = context->return_state; /* Return to the original state at the time '<' was found */
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_CDATA_BEGIN1: /* Found a '[' after "<!", looking for 'C' */
            HOXML_LOG_STATE("HOXML_STATE_CDATA_BEGIN1")
            hoxml_pop_stack(context); /* The preceeding '<' triggered a new node. Undo it. */
            if (c.codepoint == 'C')
                context->state = HOXML_STATE_CDATA_BEGIN2;
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_CDATA_BEGIN2: /* Found a 'C' after "<![", looking for 'D' */
            HOXML_LOG_STATE("HOXML_STATE_CDATA_BEGIN2")
            if (c.codepoint == 'D')
                context->state = HOXML_STATE_CDATA_BEGIN3;
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_CDATA_BEGIN3: /* Found a 'D' after "<![C", looking for 'A' */
            HOXML_LOG_STATE("HOXML_STATE_CDATA_BEGIN3")
            if (c.codepoint == 'A')
                context->state = HOXML_STATE_CDATA_BEGIN4;
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_CDATA_BEGIN4: /* Found an 'A' after "<![CD", looking for 'T' */
            HOXML_LOG_STATE("HOXML_STATE_CDATA_BEGIN4")
            if (c.codepoint == 'T')
                context->state = HOXML_STATE_CDATA_BEGIN5;
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_CDATA_BEGIN5: /* Found a 'T' after "<![CDA", looking for 'A' */
            HOXML_LOG_STATE("HOXML_STATE_CDATA_BEGIN5")
            if (c.codepoint == 'A')
                context->state = HOXML_STATE_CDATA_BEGIN6;
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_CDATA_BEGIN6: /* Found an 'A' after "<![CDAT", looking for '[' */
            HOXML_LOG_STATE("HOXML_STATE_CDATA_BEGIN6")
            if (c.codepoint == '[')
                context->state = HOXML_STATE_CDATA_CONTENT;
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_CDATA_CONTENT: /* Found a '[' after "<![CDATA" and now in a CDATA section, looking for ']' */
            HOXML_LOG_STATE("HOXML_STATE_CDATA_CONTENT")
            hoxml_append_character(context, c);
            if (context->state >= HOXML_STATE_NONE) { /* If appending the character was successful */
                if (c.codepoint == ']')
                    context->state = HOXML_STATE_CDATA_END1;
            } break;
        case HOXML_STATE_CDATA_END1: /* Found a ']' while in a CDATA section, looking for a second ']' */
            HOXML_LOG_STATE("HOXML_STATE_CDATA_END1")
            hoxml_append_character(context, c);
            if (context->state >= HOXML_STATE_NONE) { /* If appending the character was successful */
                if (c.codepoint == ']')
                    context->state = HOXML_STATE_CDATA_END2;
                else
                    context->state = HOXML_STATE_CDATA_CONTENT;
            } break;
        case HOXML_STATE_CDATA_END2: /* Found a second ']' while in a CDATA section, looking for '>' */
            HOXML_LOG_STATE("HOXML_STATE_CDATA_END2")
            if (c.codepoint == '>') {
                context->state = HOXML_STATE_OPEN_TAG;
                /* We couldn't be sure the CDATA section had ended until now so two ']' characters were appended. */
                /* If the document is encoded with UTF-16, four bytes need to be removed. Two bytes otherwise. */
                size_t bytes = context->encoding >= HOXML_ENC_UTF_16_BE ? 4 : 2;
                /* The 'end' pointer is currently pointing at the last byte, the second ']' or its latter half if */
                /* using UTF-16. To remove the "]]" we replace them with zeroes. */
                memset(HOXML_STACK->end - bytes + 1, 0, bytes);
                HOXML_STACK->end -= bytes;
            } else {
                hoxml_append_character(context, c);
                if (context->state >= HOXML_STATE_NONE) /* If appending the character was successful */
                    context->state = HOXML_STATE_CDATA_CONTENT;
            } break;
        case HOXML_STATE_REFERENCE_BEGIN: /* Found an '&' in content or a value, looking for '#', ';', or characters */
            HOXML_LOG_STATE("HOXML_STATE_REFERENCE_BEGIN")
            context->reference_start = HOXML_STACK->end + 1; /* Point to the first byte for comparisons later */
            if (c.codepoint == '#')
                context->state = HOXML_STATE_REFERENCE_NUMERIC;
            /* The predefined entities are "amp", "lt", "gt", "quot", and "apos". Check for just their first letters. */
            else if (c.codepoint == 'a' || c.codepoint == 'g' || c.codepoint == 'l' || c.codepoint == 'q') {
                hoxml_append_character(context, c);
                if (context->state >= HOXML_STATE_NONE) /* If appending the character was successful */
                    context->state = HOXML_STATE_REFERENCE_ENTITY;
            } else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_REFERENCE_ENTITY: /* Looking for "[a]mp", "[l]t", "[g]t", "[q]uot", or "apos" */
            HOXML_LOG_STATE("HOXML_STATE_REFERENCE_ENTITY")
            if (c.codepoint == ';')
                hoxml_end_reference(context, HOXML_REF_TYPE_ENTITY);
            /* Predefined escapes only use a subset of lower case English characters. For now, we'll check for ASCII. */
            else if (HOXML_IS_ASCII_CHAR(c.codepoint))
                hoxml_append_character(context, c);
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_REFERENCE_NUMERIC: /* Found a '#' in a reference, looking for 'x', ';', or chars */
            HOXML_LOG_STATE("HOXML_STATE_REFERENCE_NUMERIC")
            if (c.codepoint == 'x')
                context->state = HOXML_STATE_REFERENCE_HEX;
            else if (c.codepoint == ';')
                hoxml_end_reference(context, HOXML_REF_TYPE_NUMERIC);
            else if (HOXML_IS_NUMERIC(c.codepoint))
                hoxml_append_character(context, c);
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_REFERENCE_HEX: /* Found an 'x' in a reference after '#', looking for chars or ';' */
            HOXML_LOG_STATE("HOXML_STATE_REFERENCE_HEX")
            if (c.codepoint == ';')
                hoxml_end_reference(context, HOXML_REF_TYPE_HEX);
            else if (HOXML_IS_HEX_CHAR(c.codepoint))
                hoxml_append_character(context, c);
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_PROCESSING_INSTRUCTION_BEGIN: /* Found a '?' after a '<' and now in a processing instruction */
            HOXML_LOG_STATE("HOXML_STATE_PROCESSING_INSTRUCTION_BEGIN")
            if (HOXML_IS_NAME_START_CHAR(c.codepoint)) {
                hoxml_append_character(context, c);
                if (context->state >= HOXML_STATE_NONE) { /* If appending the character was successful */
                    context->state = HOXML_STATE_PROCESSING_INSTRUCTION_TARGET1;
                    context->tag = &(HOXML_STACK->tag); /* The processing instruction's target string began here */
                }
            } else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_PROCESSING_INSTRUCTION_TARGET1: /* Found a name char after "<?", looking for more name chars */
            HOXML_LOG_STATE("HOXML_STATE_PROCESSING_INSTRUCTION_TARGET1")
            if (HOXML_IS_WHITESPACE(c.codepoint)) { /* A whitespace marks an end of a target and beginning of content */
                if (hoxml_strcmp(&(HOXML_STACK->tag), context->encoding, "xml", HOXML_ENC_UNKNOWN,
                        HOXML_CASE_INSENSITIVE) && HOXML_STACK->parent != NULL) {
                    /* The document declaration (e.g. <?xml encoding="UTF-8"?>) must come before the first element */
                    context->state = HOXML_STATE_ERROR_INVALID_DOCUMENT_DECLARATION;
                    return HOXML_ERROR_INVALID_DOCUMENT_DECLARATION;
                }
                hoxml_append_terminator(context);
                if (context->state >= HOXML_STATE_NONE) { /* If appending the terminator was successful */
                    context->state = HOXML_STATE_PROCESSING_INSTRUCTION_CONTENT;
                    return HOXML_PROCESSING_INSTRUCTION_BEGIN;
                }
            } else if (c.codepoint == '?') { /* A '?' (or "?>") marks the end of the target and PI */
                hoxml_append_terminator(context);
                if (context->state >= HOXML_STATE_NONE) /* If appending the terminator was successful */
                    context->state = HOXML_STATE_PROCESSING_INSTRUCTION_END;
            } else if (HOXML_IS_NAME_CHAR(c.codepoint))
                hoxml_append_character(context, c);
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_PROCESSING_INSTRUCTION_CONTENT: /* Found space after a PI name, looking for '?' or chars */
            HOXML_LOG_STATE("HOXML_STATE_PROCESSING_INSTRUCTION_CONTENT")
            if (c.codepoint == '?') { /* "?>" marks the end of a processing instruction */
                const char* declaration;
                if ((declaration = hoxml_strstr(context->content, context->encoding, "encoding=", HOXML_ENC_UNKNOWN,
                        HOXML_CASE_SENSITIVE)) != NULL) {
                    const char* encoding;
                    if ((encoding = hoxml_strstr(declaration, context->encoding, "\"", HOXML_ENC_UNKNOWN,
                            HOXML_CASE_SENSITIVE)) != NULL || (encoding = hoxml_strstr(declaration, context->encoding,
                            "'", HOXML_ENC_UNKNOWN, HOXML_CASE_SENSITIVE)) != NULL) {
                        switch (context->encoding) {
                        case HOXML_ENC_UNKNOWN: /* The document did not begin with a byte order marker (BOM) */
                            if (hoxml_strcmp(encoding, context->encoding, "\"UTF-8\"", HOXML_ENC_UNKNOWN,
                                    HOXML_CASE_INSENSITIVE) != 0 || hoxml_strcmp(encoding, context->encoding,
                                    "'UTF-8'", HOXML_ENC_UNKNOWN, HOXML_CASE_INSENSITIVE) != 0) {
                                context->encoding = HOXML_ENC_UTF_8;
                            } else if (hoxml_strcmp(encoding, context->encoding, "\"UTF-16\"", HOXML_ENC_UNKNOWN,
                                    HOXML_CASE_INSENSITIVE) != 0 || hoxml_strcmp(encoding, context->encoding,
                                    "'UTF-16'", HOXML_ENC_UNKNOWN, HOXML_CASE_INSENSITIVE) != 0) {
                                /* UTF-16 encoded documents require one of the UTF-16 BOMs so this is an error */
                                context->state = HOXML_STATE_ERROR_ENCODING;
                                return HOXML_ERROR_ENCODING;
                            } break;
                        case HOXML_ENC_UTF_8: /* The UTF-8 BOM was found at the beginning of the document */
                            if (hoxml_strcmp(encoding, context->encoding, "\"UTF-8\"", HOXML_ENC_UNKNOWN,
                                    HOXML_CASE_INSENSITIVE) == 0 && hoxml_strcmp(encoding, context->encoding,
                                    "'UTF-8'", HOXML_ENC_UNKNOWN, HOXML_CASE_INSENSITIVE) == 0) {
                                /* If the UTF-8 BOM was found but the encoding declaration was not "UTF-8" then we */
                                /* have a contradiction and, therefore, an error */
                                context->state = HOXML_STATE_ERROR_ENCODING;
                                return HOXML_ERROR_ENCODING;
                            } break;
                        case HOXML_ENC_UTF_16_LE: /* The UTF-16LE BOM was found at the beginning of the document */
                        case HOXML_ENC_UTF_16_BE: /* The UTF-16BE BOM was found at the beginning of the document */
                            if (hoxml_strcmp(encoding, context->encoding, "\"UTF-16\"", HOXML_ENC_UNKNOWN,
                                    HOXML_CASE_INSENSITIVE) == 0 && hoxml_strcmp(encoding, context->encoding,
                                    "'UTF-16'", HOXML_ENC_UNKNOWN, HOXML_CASE_INSENSITIVE) == 0)
                                return HOXML_ERROR_ENCODING;
                            break;
                        }
                    }
                }
                hoxml_append_terminator(context);
                if (context->state >= HOXML_STATE_NONE) /* If appending the terminator was successful */
                    context->state = HOXML_STATE_PROCESSING_INSTRUCTION_END;
            } else {
                if (context->content == NULL) /* If this is the first character of the PI's content */
                    context->content = HOXML_STACK->end + 1; /* The PI's content string will begin here */
                hoxml_append_character(context, c);
            } break;
        case HOXML_STATE_DTD_BEGIN1: /* Found a 'D' after "<!", looking for 'O' */
            HOXML_LOG_STATE("HOXML_STATE_DTD_BEGIN1")
            hoxml_pop_stack(context); /* The preceeding '<' triggered a new node. Undo it. */
            if (c.codepoint == 'O')
                context->state = HOXML_STATE_DTD_BEGIN2;
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_DTD_BEGIN2: /* Found an 'O' after "<!D", looking for 'C' */
            HOXML_LOG_STATE("HOXML_STATE_DTD_BEGIN2")
            if (c.codepoint == 'C')
                context->state = HOXML_STATE_DTD_BEGIN3;
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_DTD_BEGIN3: /* Found a 'C' after "<!DO", looking for 'T' */
            HOXML_LOG_STATE("HOXML_STATE_DTD_BEGIN3")
            if (c.codepoint == 'T')
                context->state = HOXML_STATE_DTD_BEGIN4;
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_DTD_BEGIN4: /* Found a 'T' after "<!DOC", looking for 'Y' */
            HOXML_LOG_STATE("HOXML_STATE_DTD_BEGIN4")
            if (c.codepoint == 'Y')
                context->state = HOXML_STATE_DTD_BEGIN5;
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_DTD_BEGIN5: /* Found a 'Y' after "<!DOCT", looking for 'P' */
            HOXML_LOG_STATE("HOXML_STATE_DTD_BEGIN5")
            if (c.codepoint == 'P')
                context->state = HOXML_STATE_DTD_BEGIN6;
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_DTD_BEGIN6: /* Found a 'P' after "<!DOCTY", looking for 'E' */
            HOXML_LOG_STATE("HOXML_STATE_DTD_BEGIN6")
            if (c.codepoint == 'E')
                context->state = HOXML_STATE_DTD_BEGIN7;
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_DTD_BEGIN7: /* Found an 'E' after "<!DOCTYP", looking for whitespace */
            HOXML_LOG_STATE("HOXML_STATE_DTD_BEGIN7")
            if (HOXML_IS_WHITESPACE(c.codepoint))
                context->state = HOXML_STATE_DTD_BEGIN8;
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_DTD_BEGIN8: /* Found space after "<!DOCTYPE", looking for more or a name start character */
            HOXML_LOG_STATE("HOXML_STATE_DTD_BEGIN8")
            if (HOXML_IS_NAME_START_CHAR(c.codepoint))
                context->state = HOXML_STATE_DTD_NAME;
            else if (!HOXML_IS_WHITESPACE(c.codepoint))
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_DTD_NAME: /* Found a name start character, looking for whitespace or name characters */
            HOXML_LOG_STATE("HOXML_STATE_DTD_NAME")
            if (HOXML_IS_WHITESPACE(c.codepoint))
                context->state = HOXML_STATE_DTD_CONTENT;
            else if (!HOXML_IS_NAME_CHAR(c.codepoint))
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_DTD_CONTENT: /* Found a DTD name and now looking for mostly anything but mainly '[' or '>' */
            HOXML_LOG_STATE("HOXML_STATE_DTD_CONTENT")
            /* We support Document Type Declarations (DTDs) insofar as they do not cause problems and DTD names may */
            /* be recognized as invalid. Beyond that, the content is ignored just as comments are. That said, some */
            /* checks are done here and the "open bracket" state because they're easy. */
            if (c.codepoint == '[')
                context->state = HOXML_STATE_DTD_OPEN_BRACKET;
            else if (c.codepoint == '>')
                context->state = HOXML_STATE_NONE; /* Return to the "before root element" state, the only one allowed */
            else if (!HOXML_IS_CHAR_DATA(c.codepoint))
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        case HOXML_STATE_DTD_OPEN_BRACKET: /* Found a '[' within DTD content, looking for a closing ']' */
            HOXML_LOG_STATE("HOXML_STATE_DTD_OPEN_BRACKET")
            /* Some additional characters are allowed between '[' and ']' brackets, namely markup declaration */
            /* characters like '<' and '>'. We'll just allow anything to keep things simple. */
            if (c.codepoint == ']')
                context->state = HOXML_STATE_DTD_CONTENT;
            break;
        case HOXML_STATE_PROCESSING_INSTRUCTION_END: /* Found a '?' after PI content, looking for '>' */
            HOXML_LOG_STATE("HOXML_STATE_PROCESSING_INSTRUCTION_END")
            if (c.codepoint == '>')
                return hoxml_end_tag(context);
            else
                context->state = HOXML_STATE_ERROR_SYNTAX;
            break;
        }
    } /* while (context->state >= HOXML_STATE_NONE && context->state <= HOXML_STATE_DONE) */

    /* A dozen or so states may try to add data to the buffer, new nodes, characters, or terminators. It's possible */
    /* there was not enough space left in the buffer for this putting us into an error state where parsing cannot */
    /* continue. In these cases, the state will have been set appropriately. */
    if (context->state == HOXML_STATE_ERROR_INSUFFICIENT_MEMORY) {
        /* Because the character leading to this error state could not be used, we'll undo the iteration in the hopes */
        /* that we recover from this error (one of two errors that can be recovered, by hoxml_realloc() in this case) */
        /* and parsing can continue on the next call to hoxml_parse() */
        context->iterator = previous_iterator;
        context->stream_length = previous_stream_length;
        context->column--; /* If recovered, parsing will continue with the same character so don't count this one */
        return HOXML_ERROR_INSUFFICIENT_MEMORY;
    }

    /* Any other error case not yet covered by previous checks is due to incorrect syntax in the document */
    return HOXML_ERROR_SYNTAX;
}

/* Attempt to push a new node to the stack as a child of the current head node */
void hoxml_push_stack(hoxml_context_t* context) {
    /* If "allocating" a new node would overflow the buffer */
    if ((context->stack == NULL && sizeof(hoxml_node_t) >= context->buffer_length) || (context->stack != NULL &&
            HOXML_STACK->end + 1 + sizeof(hoxml_node_t) >= context->buffer + context->buffer_length)) {
        context->error_return_state = context->state;
        context->state = HOXML_STATE_ERROR_INSUFFICIENT_MEMORY;
        return;
    }

    hoxml_node_t* node;
    if (context->stack == NULL)/* If pushing the root node */
        node = (hoxml_node_t*) context->buffer; /* Place the new node at the beginning of the buffer */
    else
        node = (hoxml_node_t*)(HOXML_STACK->end + 1);
    if (node != NULL) {
        /* Assign initial values to the node */
        node->parent = HOXML_STACK; /* This new node's parent is the previous stack node */
        node->end = &(node->tag) - 1; /* Point to the last byte of the node, -1 because no tag has been copied yet */
    }
    context->stack = (char*)node;
}

/* Pop the head node from the stack */
void hoxml_pop_stack(hoxml_context_t* context) {
    if (context->stack == NULL)
        return;

    /* Reassign the stack (head) pointer so that it now points to the parent of the node about to be popped */
    hoxml_node_t* popped_node = HOXML_STACK;
    context->stack = (char*)popped_node->parent;

    /* Overwrite the memory used by this node with zeroes */
    context->tag = context->attribute = context->value = context->content = NULL; /* TODO: move somewhere else */
    memset(popped_node, 0, popped_node->end - (char*)popped_node + 1);
}

/* Attempt to add the given character to the end of the stack's current head node */
void hoxml_append_character(hoxml_context_t* context, hoxml_character_t c) {
    HOXML_STACK->flags &= ~HOXML_FLAG_TERMINATED;

    if (HOXML_STACK->end + c.bytes >= context->buffer + context->buffer_length) {
        context->error_return_state = context->state;
        context->state = HOXML_STATE_ERROR_INSUFFICIENT_MEMORY;
        return;
    }

    memcpy(HOXML_STACK->end + 1, &(c.encoded), c.bytes); /* Copy the character to the stack */
    HOXML_STACK->end += c.bytes; /* Redirect the end pointer to the new end just after the appended character */
}

/* Attempt to add a null terminator to the end of the stack's current head node */
void hoxml_append_terminator(hoxml_context_t* context) {
    if (HOXML_STACK->flags & HOXML_FLAG_TERMINATED) /* If the node's current string is already terminated */
        return; /* To avoid adding additional terminators and using more bytes than expected, do nothing */
    HOXML_STACK->flags |= HOXML_FLAG_TERMINATED;

    /* If the document is encoded with UTF-16, two bytes will be appended. One byte otherwise. */
    size_t bytes = context->encoding >= HOXML_ENC_UTF_16_BE ? 2 : 1;
    if (HOXML_STACK->end + bytes >= context->buffer + context->buffer_length) {
        context->error_return_state = context->state;
        context->state = HOXML_STATE_ERROR_INSUFFICIENT_MEMORY;
        return;
    }

    memset(HOXML_STACK->end + 1, '\0', bytes); /* Copy the terminator to the stack */
    HOXML_STACK->end += bytes; /* Redirect the end pointer to the new end just after the appended terminator */
}

/* Perform the steps needed to decode and clean up after a character or entity reference given the context obect and */
/* the type of reference. There are three types defined in an enumeration. */
void hoxml_end_reference(hoxml_context_t* context, int type) {
    hoxml_character_t c;
    c.codepoint = c.encoded = 0;
    c.bytes = 0;
    unsigned long value; /* Integer value of numeric or hexadecimal reference */

    switch (type) {
    case HOXML_REF_TYPE_ENTITY:
        if (hoxml_strcmp(context->reference_start, context->encoding, "lt",
                HOXML_ENC_UNKNOWN, HOXML_CASE_SENSITIVE) != 0) {
            c = hoxml_encode_character('<', context->encoding);
        } else if (hoxml_strcmp(context->reference_start, context->encoding, "gt",
                HOXML_ENC_UNKNOWN, HOXML_CASE_SENSITIVE) != 0) {
            c = hoxml_encode_character('>', context->encoding);
        } else if (hoxml_strcmp(context->reference_start, context->encoding, "amp", HOXML_ENC_UNKNOWN,
                HOXML_CASE_SENSITIVE) != 0) {
            c = hoxml_encode_character('&', context->encoding);
        } else if (hoxml_strcmp(context->reference_start, context->encoding, "apos", HOXML_ENC_UNKNOWN,
                HOXML_CASE_SENSITIVE) != 0) {
            c = hoxml_encode_character('\'', context->encoding);
        } else if (hoxml_strcmp(context->reference_start, context->encoding, "quot", HOXML_ENC_UNKNOWN,
                HOXML_CASE_SENSITIVE) != 0) {
            c = hoxml_encode_character('"', context->encoding);
        } else
            context->state = HOXML_STATE_ERROR_SYNTAX;
        break;
    case HOXML_REF_TYPE_NUMERIC:
        value = strtoul(hoxml_to_ascii(context->reference_start, context->encoding), NULL, 10);
        if (value != 0) /* If the reference string could be converted as a base-ten integer */
            c = hoxml_encode_character(value, context->encoding);
        else
            context->state = HOXML_STATE_ERROR_SYNTAX;
        break;
    case HOXML_REF_TYPE_HEX:
        value = strtoul(hoxml_to_ascii(context->reference_start, context->encoding), NULL, 16);
        if (value != 0) /* If the reference string could be converted as a base-16 integer */
            c = hoxml_encode_character(value, context->encoding);
        else
            context->state = HOXML_STATE_ERROR_SYNTAX;
        break;
    }

    /* If the reference could not be turned into a character */
    if (c.bytes == 0)
        return;

    /* Remove the reference's string from the buffer. For example, "&lt;" would result in "lt" being stored so it */
    /* could be parsed here. It should now be removed from the buffer. */
    memset(context->reference_start, 0, HOXML_STACK->end - context->reference_start + 1);
    HOXML_STACK->end = context->reference_start - 1;
    context->reference_start = NULL;
    hoxml_append_character(context, c); /* Append the character being referenced */
    /* No need for any checks against the buffer length. In all cases, more bytes were removed just now than added. */
    context->state = context->return_state; /* Either HOXML_STATE_OPEN_TAG or HOXML_STATE_ATTRIBUTE_VALUE */
    context->return_state = HOXML_STATE_NONE;
}

void hoxml_begin_tag(hoxml_context_t* context) {
    hoxml_push_stack(context);
    if (context->state >= HOXML_STATE_NONE) { /* If pushing a new node was successful */
        context->return_state = context->state; /* For comments and references, so we know which state to return to */
        context->state = HOXML_STATE_TAG_BEGIN;
    }
}

hoxml_code_t hoxml_end_tag(hoxml_context_t* context) {
    context->state = HOXML_STATE_OPEN_TAG;
    context->post_state = HOXML_POST_STATE_TAG_END; /* Common to three of the four possible cases */
    hoxml_node_t* node = HOXML_STACK;
    hoxml_node_t* parent = node->parent;
    if (node->flags & HOXML_FLAG_END_TAG) { /* True for e.g. </tag> but not <tag/> */
        if (parent == NULL || hoxml_strcmp(&(node->tag), context->encoding, &(parent->tag), context->encoding,
                HOXML_CASE_SENSITIVE) == 0) { /* If there was preceeding open tag or there is but it doesn't match */
            context->state = HOXML_STATE_ERROR_TAG_MISMATCH;
            return HOXML_ERROR_TAG_MISMATCH;
        } else { /* If an element successfully closed a matching open tag */
            hoxml_pop_stack(context); /* Pop the end tag (e.g. "</tag>") */
            context->tag = &(parent->tag);
            /* Element content is placed, in memory, after the tag and its terminator... */
            context->content = context->tag + hoxml_strlen(context->tag, context->encoding);
            /* ...which may be either one or two bytes, depending on encoding */
            context->content += (context->encoding >= HOXML_ENC_UTF_16_BE ? 2 : 1);
             /* Closing an element means one less level of nesting so decrement the depth after returning */
            HOXML_STACK->flags |= HOXML_FLAG_DECREMENT_DEPTH;
            return HOXML_ELEMENT_END;
        }
    } else if (node->flags & HOXML_FLAG_EMPTY_ELEMENT) /* Self-closing/empty element (e.g. "<tag/>") */
        return HOXML_ELEMENT_END;
    else if (node->flags & HOXML_FLAG_PROCESSING_INSTRUCTION) /* Processing instruction (e.g. "<?xml?>") */
        return HOXML_PROCESSING_INSTRUCTION_END;
    /* The only remaining case is an open tag (e.g. "<tag>") and we expect a matching close tag later */
    context->post_state = HOXML_STATE_NONE; /* For this fourth case, of four possible, there is no clean up */
    /* Opening an element means one more level of nesting so increment the depth after returning */
    HOXML_STACK->flags |= HOXML_FLAG_INCREMENT_DEPTH;
    return HOXML_ELEMENT_BEGIN;
}

int hoxml_post_state_cleanup(hoxml_context_t* context) {
    if (context->post_state != HOXML_STATE_NONE) {
        switch (context->post_state) {
        case HOXML_POST_STATE_TAG_END: { /* Clean up after a close tag, empty element, or processing instruction */
            int was_document_or_document_type_declaration = 0;
            /* If the processing instruction flag is applied (i.e. this is a PI) and the PI's target is the reserved */
            /* "xml" target, or some other case variant of it */
            if (HOXML_STACK->flags & HOXML_FLAG_PROCESSING_INSTRUCTION && hoxml_strcmp(&(HOXML_STACK->tag),
                    context->encoding, "xml", HOXML_ENC_UNKNOWN, HOXML_CASE_INSENSITIVE)) {
                context->state = HOXML_STATE_NONE; /* Return to the initial state as if nothing happened */
                was_document_or_document_type_declaration = 1;
            }
            hoxml_pop_stack(context); /* Pop a start or self-closed tag (<tag> or <tag/> or <?pi?>)*/
            if (context->stack == NULL && was_document_or_document_type_declaration == 0)
                return 1; /* hoxml_parse() should return HOXML_END_OF_DOCUMENT */
            break;
        } case HOXML_POST_STATE_ATTRIBUTE_END: /* Remove the most recent attribute and value strings from the buffer */
            /* Zero the memory from the end pointer to the byte at which the attribute's name begins */
            memset(context->attribute, 0, HOXML_STACK->end - (char*)context->attribute + 1);
            HOXML_STACK->end = context->attribute - 1;
            /* With these public properties now pointing to zeroes, nullify them so there's no confusion */
            context->attribute = context->value = NULL;
            break;
        }
        context->post_state = HOXML_STATE_NONE;
    }

    return 0; /* hoxml_parse() should not return */
}

/* Decode the given character with the given encoding to the its equivalent value */
hoxml_character_t hoxml_decode_character(const char* str, size_t str_length, int encoding) {
    hoxml_character_t c;
    c.encoded = c.codepoint = 0; /* These default values are not valid so pausing will cease if returned */
    c.bytes = 0;

    switch (encoding) {
    case HOXML_ENC_UNKNOWN:
        c.bytes = 1;
        break;
    case HOXML_ENC_UTF_8:
        /* The first byte of a UTF-8 character can can begin with one of four bit patterns, each indicating the */
        /* number of remaining bytes: 0XXXXXXX = 1 byte, 110XXXXX = 2 bytes, 1110XXXX = 3 bytes, 11110XXX = 4 bytes. */
        /* NOTE: UTF-8 is *big* endian. */
        if (((str[0] >> 7) & 0x01) == 0x00)
            c.bytes = 1;
        else if (((str[0] >> 5) & 0x07) == 0x06)
            c.bytes = 2;
        else if (((str[0] >> 4) & 0x0F) == 0x0E)
            c.bytes = 3;
        else if (((str[0] >> 3) & 0x1F) == 0x1E)
            c.bytes = 4;
        break;
    case HOXML_ENC_UTF_16_BE:
        /* UTF-16 characters are either two bytes or four bytes where the four-byte characters are encoded such that */
        /* the first two bytes begin with 110110XX and the second with 110111XX. The rest are two-byte characters. */
        if (((str[0] >> 2) & 0x3F) == 0x36 && ((str[2] >> 2) & 0x3F) == 0x37)
            c.bytes = 4;
        else
            c.bytes = 2;
        break;
    case HOXML_ENC_UTF_16_LE:
        /* UTF-16LE (Little Endian) is just like UTF-16BE (Big Endian) but the most and least significant bytes in */
        /* any 16-bit sequence are swapped. (Technically, a byte isn't defined as eight bits but it is in practice.) */
        if (((str[1] >> 2) & 0x3F) == 0x36 && ((str[3] >> 2) & 0x3F) == 0x37)
            c.bytes = 4;
        else
            c.bytes = 2;
        break;
    }

    /* If the string doesn't have enough bytes in it to decode this character */
    if (c.bytes > str_length) {
        /* Set the decoded value to the maximum possible to indicate a failure, zero the rest, and return early */
        c.codepoint = UINT32_MAX;
        c.encoded = 0;
        c.bytes = 0;
        return c;
    }

    switch (encoding) {
    case HOXML_ENC_UNKNOWN:
        c.codepoint = str[0];
        break;
    case HOXML_ENC_UTF_8:
        if (c.bytes == 1) {
            /* One-byte UTF-8 characters are encoded as 0XXXXXXX where the Xs represent the bits of the character's */
            /* value. For all decoding, we want to grab only those bits and transform them into an integer. */
            /* The method here takes one byte from the string, uses a mask to zero out any bit that is not part of */
            /* resulting value, casts the masked byte to an unsigned 32-bit integer, shifts those bits to the left to */
            /* place them at the indexes they're expected in the value, and then bitwise ORs these components into a */
            /* single unsigned 32-bit integer. This one-byte case does not need any shift but the remaining cases do. */
            c.codepoint = (unsigned)(str[0] & 0x7F);
        } else if (c.bytes == 2) {
            /* Two-byte UTF-8 characters are encoded as 110XXXXX 10XXXXXX */
            c.codepoint = ((unsigned)(str[0] & 0x1F) << 6) | (unsigned)(str[1] & 0x3F);
        } else if (c.bytes == 3) {
            /* Three-byte UTF-8 characters are encoded as 1110XXXX 10XXXXXX 10XXXXXX */
            c.codepoint = ((unsigned)(str[0] & 0x0F) << 12) | ((unsigned)(str[1] & 0x3F) << 6) |
                          ((unsigned)(str[2] & 0x3F) << 0);
        } else if (c.bytes == 4) {
            /* Four-byte UTF-8 characters are encoded as 11110XXX 10XXXXXX 10XXXXXX 10XXXXXX */
            c.codepoint = ((unsigned)(str[0] & 0x07) << 18) | ((unsigned)(str[1] & 0x3F) << 12) |
                          ((unsigned)(str[2] & 0x3F) << 6)  |  (unsigned)(str[3] & 0x3F);
        } break;
    case HOXML_ENC_UTF_16_BE:
        if (c.bytes == 2) {
            /* Concatenate the two bytes together to retrieve the original value */
            c.codepoint = ((unsigned)str[0] << 8) | ((unsigned)str[1] << 0);
        } else if (c.bytes == 4) {
            /* Four-byte UTF-16 characters are encoded as 110110XX XXXXXXXX 110111XX XXXXXXXX after first subtracting */
            /* 0x00010000 from the value. Here, that subtracted value is reconstructed and 0x00010000 is added back. */
            c.codepoint = (((unsigned)(str[0] & 0x03) << 18) | ((unsigned)str[1] << 16) |
                           ((unsigned)(str[2] & 0x03) << 8)  | ((unsigned)str[3] << 0)) + 0x00010000;
        }
        break;
    case HOXML_ENC_UTF_16_LE:
        if (c.bytes == 2)
            c.codepoint = ((unsigned)str[1] << 8) | ((unsigned)str[0] << 0);
        else if (c.bytes == 4) {
            c.codepoint = (((unsigned)(str[1] & 0x03) << 18) | ((unsigned)str[0] << 16) |
                           ((unsigned)(str[3] & 0x03) << 8)  | ((unsigned)str[2] << 0)) + 0x00010000;
        }
        break;
    }

    memcpy(&(c.encoded), str, c.bytes); /* Copy the bytes of the character from the pointed-to string into c.encoded */

    return c;
}

/* Encode the given character codepoint to the given character encoding */
hoxml_character_t hoxml_encode_character(unsigned codepoint, int encoding) {
    hoxml_character_t c;
    c.codepoint = codepoint;
    c.encoded = 0;
    c.bytes = 0;

    /* This variable will make it easier to assign values to 'c.endoded' without difficult-to-read casts */
    char* str = (char*)&(c.encoded);

    switch (encoding) {
    case HOXML_ENC_UNKNOWN: /* If the encoding is somehow not specified, assume UTF-8 */
    case HOXML_ENC_UTF_8:
        if (codepoint <= 0x0000007F) { /* If the codepoint will fit into one byte */
            c.encoded = codepoint;
            c.bytes = 1;
        } else if (codepoint >= 0x000080 && codepoint <= 0x000007FF) { /* If the codepoint will fit into two bytes */
            /* For codepoints with bits XXXXXAAA AABBBBBB, we want to transform them to the form 110AAAAA 10BBBBBB. */
            /* The method here treats c.encoded as an array of unsigned, eight-bit integers. This is done to assign */
            /* bytes individually for the sake of endianness where UTF-8 is big endian. The codepoint is masked in */
            /* order to zero any bits that are not used in the byte being assigned, then shifted all the way to the */
            /* right. The prefixed "0xC0" and "0x80" bitwise ORs prepend the UTF-8 markers 110 and 10, respectively. */
            str[0] = 0xC0 | (char)((codepoint & 0x0000007C0) >> 6); /* 110AAAAAA */
            str[1] = 0x80 | (char)((codepoint & 0x0000000FF) >> 0); /* 10BBBBBB */
            c.bytes = 2;
        } else if ((codepoint >= 0x00000800 && codepoint <= 0x0000D7FF) ||
                (codepoint >= 0x0000E000 && codepoint <= 0x0000FFFF)) {
            /* For a codepoint with bits AAAABBBB BBCCCCCC we want 1110AAAA 10BBBBBB 10CCCCCC */
            str[0] = 0xE0 | (char)((codepoint & 0x0000F000) >> 12); /* 1110AAAA */
            str[1] = 0x80 | (char)((codepoint & 0x00000FC0) >> 6);  /* 10BBBBBB */
            str[2] = 0x80 | (char)((codepoint & 0x0000003F) >> 0);  /* 10CCCCCC */
            c.bytes = 3;
        } else if (codepoint >= 0x00010000 && codepoint <= 0x0010FFFF) {
            /* For a codepoint with bits XXXAAABB BBBBCCCC CCDDDDDD we want 11110AAA 10BBBBBB 10CCCCCC 10DDDDDD */
            str[0] = 0xF0 | (char)((codepoint & 0x001C0000) >> 18); /* 11110AAA */
            str[1] = 0x80 | (char)((codepoint & 0x0003F000) >> 12); /* 10BBBBBB */
            str[2] = 0x80 | (char)((codepoint & 0x00000FC0) >> 6);  /* 10CCCCCC */
            str[3] = 0x80 | (char)((codepoint & 0x0000003F) >> 0);  /* 10DDDDDD */
            c.bytes = 4;
        } else /* If the codepoint is not valid */
            c.bytes = 0; /* Don't even try */
        break;
    case HOXML_ENC_UTF_16_BE:
        if (codepoint <= 0x0000D7FF || (codepoint >= 0x0000E000 && codepoint <= 0x0000FFFF)) { /* Fits in two bytes */
            str[0] = (char)((codepoint & 0x0000FF00) >> 8);
            str[1] = (char) (codepoint & 0x000000FF);
            c.bytes = 2;
        } else if (codepoint >= 0x00010000 && codepoint <= 0x0010FFFF) { /* If the codepoint fits in four bytes */
            /* For codepoint - 0x00010000 with bits XXXXXXXX XXXXAABB BBBBBBCC DDDDDDDD we want to transform the bits */
            /* to the form 110110AA BBBBBBBB 110111CC DDDDDDDD. When decoded, as per UTF-16, 0x00010000 is added. */
            /* The prefixed "0xD8" and "0xDC" bitwise ORs prepend the UTF-16 markers 110110 and 110111, respectively. */
            codepoint -= 0x00010000;
            str[0] = 0xD8 | (char)((codepoint & 0x000C0000) >> 20); /* 110110AA */
            str[1] =        (char)((codepoint & 0x0003FC00) >> 18); /* BBBBBBBB */
            str[2] = 0xDC | (char)((codepoint & 0x00000300) >> 8);  /* 110111CC */
            str[3] =        (char)((codepoint & 0x000000FF) >> 0);  /* DDDDDDDD */
            c.bytes = 4;
        } else /* If the codepoint is not valid */
            c.bytes = 0; /* Don't even try */
        break;
    case HOXML_ENC_UTF_16_LE:
        /* UTF-16LE (Little Endian) is just like UTF-16BE (Big Endian) with the reverse endianness meaning that the */
        /* operations here are identical to those above but the indexes have been changed to reflect endianness */
        if (codepoint <= 0x0000D7FF || (codepoint >= 0x0000E000 && codepoint <= 0x0000FFFF)) {
            str[1] = (char)((codepoint & 0x0000FF00) >> 8);
            str[0] = (char)((codepoint & 0x000000FF) >> 0);
            c.bytes = 2;
        } else if (codepoint >= 0x00010000 && codepoint <= 0x0010FFFF) {
            codepoint -= 0x00010000;
            str[3] = 0xD8 | (char)((codepoint & 0x000C0000) >> 20); /* 110110AA */
            str[2] =        (char)((codepoint & 0x0003FC00) >> 18); /* BBBBBBBB */
            str[1] = 0xDC | (char)((codepoint & 0x00000300) >> 8);  /* 110111CC */
            str[0] =        (char)((codepoint & 0x000000FF) >> 0);  /* DDDDDDDD */
            c.bytes = 4;
        } else
            c.bytes = 0;
        break;
    }

    return c;
}

/* Given a reference string with the given encoding, return an equivalent string encoding using ASCII */
char* hoxml_to_ascii(const char* str, int encoding) {
    /* This is only for references which have a maximum length so this static array will always be large enough */
    static char ascii[16];
    memset(ascii, 0, sizeof(ascii));

    size_t ascii_index = 0; /* Current index in the ASCII string */
    const char* it = str;
    hoxml_character_t c = hoxml_decode_character(it, 65535, encoding);

    /* While we haven't iterated up to a null terminator AND not beyond the size of the result array */
    while (c.codepoint != '\0' && ascii_index < sizeof(ascii)) {
        /* Assign the decoded value of the source to the equivalent index in the result array. In practice, this is */
        /* effectively only necessary for UTF-16 content where the encoded values differ from decoded ones for */
        /* characters also included in ASCII. For UTF-8, this whole function call could be skipped. */
        ascii[ascii_index++] = c.codepoint;

        /* Iterate to the next character in the string */
        it += c.bytes;
        c = hoxml_decode_character(it, 65535, encoding);
    }

    return ascii;
}

/* Get the length, in bytes not characters, of the given string with the given encoding */
size_t hoxml_strlen(const char* str, int encoding) {
    size_t length = 0;
    const char* it = str;
    hoxml_character_t c = hoxml_decode_character(it++, 65535, encoding);

    while (c.codepoint != '\0') { /* While we haven't iterated to a null terminator */
        length++;
        c = hoxml_decode_character(it++, 65535, encoding);
    }

    return length;
}

/* Compare the given strings with the given encodings for equality with an additional parameter for case sensitivity. */
/* The return value is 0 if the strings are not equal. All other return values mean the strings are equal. */
int hoxml_strcmp(const char* str1, int encoding1, const char* str2, const int encoding2, int sensitivity) {
    const char* it1 = str1;
    const char* it2 = str2;
    hoxml_character_t c1 = hoxml_decode_character(it1, 65535, encoding1);
    hoxml_character_t c2 = hoxml_decode_character(it2, 65535, encoding2);

    while (c1.codepoint != '\0' && c2.codepoint != '\0') { /* While neither iterator has reached a null terminator */
        /* If, accounting for sensitivity, the charcters are not equal */
        if ((sensitivity == HOXML_CASE_INSENSITIVE && HOXML_TO_LOWER(c1.codepoint) != HOXML_TO_LOWER(c2.codepoint)) ||
                (sensitivity == HOXML_CASE_SENSITIVE && c1.codepoint != c2.codepoint)) {
            return 0;
        }

        /* Continue iterating through both strings one character at a time */
        it1 += c1.bytes;
        c1 = hoxml_decode_character(it1, 65535, encoding1);
        it2 += c2.bytes;
        c2 = hoxml_decode_character(it2, 65535, encoding2);
    }

    return c2.codepoint == '\0';
}

/* Search for a given string, needle, within another string, haystack. The return value is a pointer to the byte at */
/* which the string, needle, first appears or NULL if it cannot be found. */
const char* hoxml_strstr(const char* haystack, int haystack_encoding, const char* needle, int needle_encoding,
        int sensitivity) {
    const char* it_haystack = haystack;
    const char* it_needle = needle;
    hoxml_character_t c_haystack = hoxml_decode_character(it_haystack, 65535, haystack_encoding);
    hoxml_character_t c_needle = hoxml_decode_character(it_needle, 65535, needle_encoding);

    while (c_haystack.codepoint != '\0') { /* While we haven't iterated to a null terminator */
        /* If the current character in the haystack equals the first character in the needle AND the whole needle */
        /* string follows this character */
        if (c_haystack.codepoint == c_needle.codepoint &&
                hoxml_strcmp(it_haystack, haystack_encoding, it_needle, needle_encoding, sensitivity) != 0) {
            /* Return a pointer to the location the needle (first) appeared at */
            return it_haystack;
        }

        /* Continue iterating through characters in the haystack string */
        it_haystack += c_haystack.bytes;
        c_haystack = hoxml_decode_character(it_haystack, 65535, haystack_encoding);
    }

    return NULL; /* The needle was not found in the haystack */
}

#endif /* HOXML_IMPLEMENTATION */

#endif /* HOXML_H */
