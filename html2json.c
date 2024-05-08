#include "yxml.c"
#include <stdio.h>
#include <unistd.h>
#ifndef BUFSIZE
#define BUFSIZE 4096
#endif
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
    fprintf(stderr, "error %i at line %u col %u %s", r, x->line, x->byte, reason);
    return -r;
}
void putc_escape(int c)
{
    if (c == '\\')
        printf("\\\\");
    else if (c == '\r')
        printf("\\r");
    else if (c == '\n')
        printf("\\n");
    else if (c == '\t')
        printf("\\t");
    else
        printf("%c", c);
}
int main(int argc, char** argv)
{
    yxml_t x;
    yxml_ret_t r;
    char buf[BUFSIZE];
    yxml_init(&x, buf, BUFSIZE);
    size_t need_close_attr = 0, need_close_tag = 0, depth = 0, last_elem_depth = -1, need_sep_attr = 0, element_has_text = 0;
    for (int c; (c = getchar()) != EOF;) {
        r = yxml_parse(&x, c);
        if (0 && r)
            printf("(%i)", r);
        if (r < 0) {
            return die(r, &x);
        }
        // pre-processing (autoclose,...)
        if (need_close_attr && (r == YXML_ELEMSTART || r == YXML_CONTENT || r == YXML_ELEMEND)) {
            printf("},[");
            need_close_attr = 0;
        }

        if (r == YXML_ELEMSTART) {
            if (last_elem_depth == depth)
                printf(",");
            printf("[\"%s\",{", x.elem);
            depth++;
            need_close_attr = 1;
            need_sep_attr = 0;
			element_has_text = 0;
        } else if (r == YXML_ATTRSTART) {
            if (need_sep_attr)
                printf(",");
            printf("\"%s\":\"", x.attr);
            need_sep_attr = 1;
        } else if (r == YXML_ATTRVAL) {
            putc_escape(c);
        } else if (r == YXML_ATTREND) {
            printf("\"");
        } else if (r == YXML_CONTENT) {
            int is_space = (c == ' ' || c == '\n' || c == '\r' || c == '\t');
			if(!element_has_text && !is_space){printf("\"");}
            element_has_text |= !is_space;
			if(element_has_text)putc_escape(c);
        } else if (r == YXML_ELEMEND) {
			if(element_has_text){printf("\"");}
			element_has_text=0;
            printf("]]");
            depth--;
            last_elem_depth = depth;
        }
    }
    r = yxml_eof(&x);
    if (r < 0)
        return die(r, &x);
}