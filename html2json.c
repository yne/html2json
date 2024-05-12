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
void putc_escape(char* c)
{
    if (*c == '\\')
        printf("\\\\");
    else if (*c == '\r')
        printf("\\r");
    else if (*c == '\n')
        printf("\\n");
    else if (*c == '\t')
        printf("\\t");
    else
        printf("%s", c);
}
int main(int argc, char** argv)
{
    yxml_t x;
    yxml_ret_t old_r, r = YXML_OK;
    char buf[BUFSIZE];
    setbuf(stdout, NULL);
    yxml_init(&x, buf, BUFSIZE);
    size_t depth = 0, last_elem_depth = -1, element_has_text = 0;
    for (int c; (c = getchar()) != EOF;) {
        yxml_ret_t tmp = yxml_parse(&x, c);
        if (tmp < 0)
            return die(tmp, &x);
        if (tmp == YXML_OK)
            continue;
        element_has_text = tmp == YXML_CONTENT && ((c != ' ' && c != '\n' && c != '\r' && c != '\t') || element_has_text);
        if (tmp == YXML_CONTENT && !element_has_text){
            continue;
        }
        old_r = r;
        r = tmp;

        // global test, put it first
        if (old_r & YXML_CONTENT && !(r & YXML_CONTENT)) printf("\"");

        if (r & YXML_ELEMSTART && old_r & YXML_ELEMSTART)printf(", {}, [");
        if (r & YXML_ELEMSTART) printf("%s", (last_elem_depth == depth || old_r & YXML_CONTENT)?",":"");//0,1,2,4
        if (r & YXML_ELEMSTART) printf("\n%*.s[\"%s\"", 2*(depth++),"",x.elem);
        if (r & YXML_ATTRSTART && old_r & YXML_ELEMSTART)printf(", {");
        if (r & YXML_ATTRSTART) printf("%s\"%s\":",(old_r & YXML_ATTREND?",":""),x.attr);
        if (r & YXML_ATTRVAL && old_r & YXML_ATTRSTART)printf("\"");
        if (r & YXML_ATTRVAL) putc_escape(x.data);
        if (r & YXML_ATTREND) printf("\"");
        if (r & YXML_CONTENT && old_r & YXML_ATTREND) printf("}, [");
        if (r & YXML_CONTENT && old_r & YXML_ELEMSTART) printf(", {}, [");// insert empty object for easier JQ
        if (r & YXML_CONTENT && !(old_r & YXML_CONTENT)) printf("%s\"",last_elem_depth == depth ? ",":"");//1,2,4,32
        if (r & YXML_CONTENT) putc_escape(x.data);
        if (r & YXML_ELEMEND && (old_r & YXML_ELEMSTART)) printf(", {}, []");// insert empty child for easeir jq
        if (r & YXML_ELEMEND && (old_r & YXML_ATTREND)) printf("}, []");// insert empty child for easeir jq
        if (r & YXML_ELEMEND && (old_r & YXML_CONTENT)) printf("]");
        if (r & YXML_ELEMEND && (old_r & YXML_ELEMEND)) printf("]");
        if (r & YXML_ELEMEND) printf("]",2*(last_elem_depth = --depth),"");
    }
    r = yxml_eof(&x);
    if (r < 0)
        return die(r, &x);
}