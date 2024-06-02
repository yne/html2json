#include "yxml.c"
#include <stdio.h>
#include <unistd.h>
#ifndef BUFSIZE
#define BUFSIZE 4096
#endif
#define INTFROM8CHARS(a, b, c, d, e, f, g, h) (    (((uint64_t)(a))<<56) |    (((uint64_t)(b))<<48) |    (((uint64_t)(c))<<40) |    (((uint64_t)(d))<<32) |    (((uint64_t)(e))<<24) |   (((uint64_t)(f))<<16) |    (((uint64_t)(g))<<8) |    (uint64_t)(h))

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
    else if (*c == '"')
        printf("\\\"");
    else
        printf("%s", c);
}
uint64_t stack=0;
void getcharUntil(uint64_t limit) {
    char str[2]={0,0};
    stack = 0;
    for (int c; (c = getchar()) != EOF;) {
        stack = (stack << 8) | (uint64_t)c;
        if(stack == limit)break;
        str[0]=c;
        putc_escape(str);
    }
}
int unstackOrGetChar() {
    if(!stack)return getchar();
    int c = stack>>56;
    stack <<= 8;
    return c;
}
int main(int argc, char** argv)
{
    yxml_t x;
    yxml_ret_t old_r, r = YXML_OK;
    char buf[BUFSIZE];
    setbuf(stdout, NULL);
    yxml_init(&x, buf, BUFSIZE);
    size_t depth = 0, last_elem_depth = -1, element_has_text = 0, isRawTag = 0;
    for (int c; (c = unstackOrGetChar()) != EOF;) {
        yxml_ret_t tmp = yxml_parse(&x, c);
        if (tmp < 0)
            return die(tmp, &x);
        if (tmp == YXML_OK)
            continue;
        element_has_text = tmp == YXML_CONTENT && ((c != ' ' && c != '\n' && c != '\r' && c != '\t') || element_has_text);
        if (tmp == YXML_CONTENT && !element_has_text){
            continue;
        }
        #define is_self_close(ctx) ((ctx & YXML_ELEMSTART) && (ctx & YXML_ELEMEND))
        old_r = is_self_close(r) ? r&~YXML_ELEMSTART : r;
        r = tmp;

        // global test, put it first
        if (old_r & YXML_CONTENT && !(r & YXML_CONTENT)) printf("\"");
        // TODO: smarter way of doing that:
        // use a list of {[state]:str,...} so we can do multiple printf according to the [old_r...r] range
        if (r & YXML_ELEMSTART && old_r & YXML_ATTREND) printf("}, [");
        if (r & YXML_ELEMSTART && old_r & YXML_ELEMSTART)printf(", {}, [");
        if (r & YXML_ELEMSTART) printf("%s", (last_elem_depth == depth || old_r & YXML_CONTENT)?",":"");
        if (r & YXML_ELEMSTART) printf("\n%*.s[\"%s\"", 2*(depth++),"",is_self_close(r)?(char*)x.stack+x.stacklen+1:x.elem, isRawTag=!strcmp(x.elem, "script")?1:!strcmp(x.elem, "style")?2:0);
        if (r & YXML_ATTRSTART && old_r & YXML_ELEMSTART)printf(", {");
        if (r & YXML_ATTRSTART) printf("%s\"%s\":",(old_r & YXML_ATTREND?",":""),x.attr);
        if (r & YXML_ATTRVAL && old_r & YXML_ATTRSTART)printf("\"");
        if (r & YXML_ATTRVAL) putc_escape(x.data);
        if (r & YXML_ATTREND && old_r & YXML_ATTRSTART)printf("\"");
        if (r & YXML_ATTREND) printf("\"");
        if (r & YXML_CONTENT && old_r & YXML_ATTREND) printf("}, [");
        if (r & YXML_CONTENT && old_r & YXML_ELEMSTART) printf(", {}, [");
        if (r & YXML_CONTENT && !(old_r & YXML_CONTENT)) printf("%s\"",last_elem_depth == depth ? ",":"");
        if (r & YXML_CONTENT) putc_escape(x.data);
        if (r & YXML_ELEMEND && (old_r & YXML_ELEMSTART)) printf(", {}, []");
        if (r & YXML_ELEMEND && (old_r & YXML_ATTREND)) printf("}, []");
        if (r & YXML_ELEMEND && (old_r & YXML_CONTENT)) printf("]");
        if (r & YXML_ELEMEND && (old_r & YXML_ELEMEND)) printf("]");
        if (r & YXML_ELEMEND) printf("]",2*(last_elem_depth = --depth),"");
        if (r & YXML_CONTENT && isRawTag) { // TODO: find a better trigger
            if(isRawTag==1)getcharUntil(INTFROM8CHARS('<','/','s','c','r','i','p','t'));
            if(isRawTag==2)getcharUntil(INTFROM8CHARS('<','/','s','t','y','l','e','>'));
        }
    }
    r = yxml_eof(&x);
    if (r < 0)
        return die(r, &x);
}