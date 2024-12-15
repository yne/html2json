#include "yxml.c"
#include <stdio.h>
#include <unistd.h>
#ifndef BUFSIZE
#define BUFSIZE 4096
#endif
#define INTFROM8CHARS(a, b, c, d, e, f, g, h) ((((uint64_t)(a)) << 56) | (((uint64_t)(b)) << 48) | (((uint64_t)(c)) << 40) | (((uint64_t)(d)) << 32) | (((uint64_t)(e)) << 24) | (((uint64_t)(f)) << 16) | (((uint64_t)(g)) << 8) | (uint64_t)(h))

int die(yxml_ret_t r, yxml_t* x)
{
    char* reason = "undefined";
    if (r == YXML_EEOF)
        reason = "Unexpected EOF";
    if (r == YXML_EREF)
        reason = "Invalid character or entity reference (&whatever;)";
    if (r == YXML_ECLOSE)
        reason = "Close tag does not match open tag (<Tag> .. </OtherTag>)";
    if (r == YXML_ESTACK)
        reason = "Stack overflow (too deeply nested tags or too long element/attribute name)";
    if (r == YXML_ESYN)
        reason = "Syntax error (unexpected byte)";
    fprintf(stderr, "error %i at line %u col %u %s while in state:%i\n", r, x->line, x->byte, reason, x->state);
    return -r;
}
void puts_escaped(char* c)
{
    if (*c == '\\')
        printf("\\\\");
    else if (*c == '\r')
        printf("\\r");
    else if (*c == '\n')
        printf("\\n");
    else if (*c == '\t')
        printf("\\t");
    else if (*c == '"')
        printf("\\\"");
    else
        printf("%s", c);
}
uint64_t stack = 0;
void getcharUntil(uint64_t limit)
{
    char str[2] = { 0, 0 };
    stack = 0;
    for (int c; (c = getchar()) != EOF;) {
        stack = (stack << 8) | (uint64_t)c;
        if (stack == limit)
            break;
        str[0] = c;
        puts_escaped(str);
    }
}
int unstackOrGetChar()
{
    if (!stack)
        return getchar();
    int c = stack >> 56;
    stack <<= 8;
    return c;
}
typedef struct {
    char *pre, *data, *post;
} Printer;

int main(int argc, char** argv)
{
    yxml_t x;
    yxml_ret_t r = YXML_OK;
    int i = 0, old_index = 1; // so we arrive in YXML_ELEMSTART with from a recognizable impossible state (YXML_ATTRSTART)
    char buf[BUFSIZE];
    setbuf(stdout, NULL);
    yxml_init(&x, buf, BUFSIZE);
    size_t depth = 0, last_elem_depth = -1, element_has_text = 0, isRawTag = 0;
    for (int c; (c = unstackOrGetChar()) != EOF;) {
        yxml_ret_t ret_field = yxml_parse(&x, c);
        //        printf("(%i:%c)",ret_field, c);
        if (ret_field < 0)
            return die(ret_field, &x);
        if (ret_field == YXML_OK)
            continue;
        if (ret_field & YXML_ELEMSTART)
            isRawTag = !strcmp(x.elem, "script") ? 1 : !strcmp(x.elem, "style") ? 2
                                                                                : 0;
        // skip whitespace between tags
        //element_has_text = ret_field & YXML_CONTENT && ((c != ' ' && c != '\n' && c != '\r' && c != '\t') || element_has_text);
        //if (ret_field & YXML_CONTENT && !element_has_text) {
        //    continue;
        //}
        // get from stack when element is open+closed (<br>)
        char* el = ((ret_field & YXML_ELEMSTART) && (ret_field & YXML_ELEMEND)) ? (char*)x.stack + x.stacklen + 1 : x.elem;
        for (int ret_index = 0; ret_index < 6; ret_index++) {
            if (!((1 << ret_index) & ret_field))
                continue;
            Printer p = ((Printer[6][6]) {
                /* YXML_ELEMSTART */ { { "},[\n[\"", el, "\",{" }, { "[\"", el, "\",{" }, {}, { "},[[\"", el, "\",{" }, { "\",[\"", el, "\",{" }, { ",[\"", el, "\",{" } },
                /* YXML_ATTRSTART */ { { "\"", x.attr, "\":\"" }, {}, {}, { ",\"", x.attr, "\":\"" }, {}, {} },
                /* YXML_ATTRVAL   */ { {}, { "", x.data }, { "", x.data }, {}, {}, {} },
                /* YXML_ATTREND   */ { {}, {}, { "\"" }, {}, {}, {} },
                /* YXML_CONTENT   */ { { "},[\"", x.data }, { "\"", x.data }, {}, { "},[\"", x.data }, { "", x.data }, { ",\"", x.data } },
                /* YXML_ELEMEND   */ { { "},[]]" }, {}, {}, { "},[]]" }, { "\"]]" }, { "]]" } },
            })[ret_index][old_index];
            if (p.pre)
                fputs(p.pre, stdout);
            else
                printf("Missing Transition[%i][%i]", ret_index, old_index);
            if (p.data)
                puts_escaped(p.data);
            if (p.post)
                fputs(p.post, stdout);
            old_index = ret_index;
        }
        //if (ret_field & YXML_CONTENT) {
        //    if (isRawTag == 1) {
        //        getcharUntil(INTFROM8CHARS('<', '/', 's', 'c', 'r', 'i', 'p', 't'));
        //        unstackOrGetChar();
        //    }
        //    if (isRawTag == 2) {
        //        getcharUntil(INTFROM8CHARS('<', '/', 's', 't', 'y', 'l', 'e', '>'));
        //    }
        //}
    }
    r = yxml_eof(&x);
    if (r < 0)
        return die(r, &x);
}