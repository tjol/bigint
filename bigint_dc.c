/* bigint_dc - simple big integer calculator program
   Copyright 2020 Thomas Jollans - see COPYING */

#include "bigint.h"

#include <stdio.h>

struct stack {
    bigint_tp n;
    struct stack *next;
};

static inline bigint_tp pop(struct stack **stack_ptr)
{
    struct stack *s = *stack_ptr;
    if (s == NULL) return NULL;
    bigint_tp n = s->n;
    *stack_ptr = s->next;
    free(s);
    return n;
}

static inline void push(struct stack **stack_ptr, bigint_tp n)
{
    struct stack *s = malloc(sizeof(struct stack));
    s->next = *stack_ptr;
    s->n = n;
    *stack_ptr = s;
}

int main()
{
    const char *const HELP = "This is a simple reverse-polish big integer\n"
                             "calculator in the style of dc.\n\n"
                             "Arithmetic commands:\n"
                             "  + - * / v         (v is integer square root)\n"
                             "Stack manipulation:\n"
                             "  p       print the last item\n"
                             "  n       pop and print\n"
                             "  l       print the entire stack\n"
                             "  d       duplicate\n"
                             "  r       swap (reverse) the top two items\n"
                             "Other:\n"
                             "  h       help\n"
                             "  q       quit\n\n";

    const char *const delim = " \t\n";
    struct stack *st = NULL;

    char line[1024];
    while (fgets(line, 1024, stdin)) {
        for (char *tok = strtok(line, delim); tok != NULL; tok = strtok(NULL, delim)) {
            if (strcmp(tok, "+") == 0) {
                bigint_tp b = pop(&st);
                bigint_tp a = pop(&st);
                if (a == NULL || b == NULL)
                    fputs("ERROR: too few items in the stack\n", stderr);
                else {
                    a = bigint_add_inplace(a, b);
                    bigint_free(b);
                    push(&st, a);
                }
            } else if (strcmp(tok, "-") == 0) {
                bigint_tp b = pop(&st);
                bigint_tp a = pop(&st);
                if (a == NULL || b == NULL)
                    fputs("ERROR: too few items in the stack\n", stderr);
                else {
                    b = bigint_flipsign(b);
                    a = bigint_add_inplace(a, b);
                    bigint_free(b);
                    push(&st, a);
                }
            } else if (strcmp(tok, "*") == 0) {
                bigint_tp b = pop(&st);
                bigint_tp a = pop(&st);
                if (a == NULL || b == NULL)
                    fputs("ERROR: too few items in the stack\n", stderr);
                else {
                    bigint_tp r = bigint_mul(a, b);
                    bigint_free(a);
                    bigint_free(b);
                    push(&st, r);
                }
            } else if (strcmp(tok, "/") == 0) {
                bigint_tp b = pop(&st);
                bigint_tp a = pop(&st);
                if (a == NULL || b == NULL)
                    fputs("ERROR: too few items in the stack\n", stderr);
                else {
                    bigint_tp r = bigint_div(a, b);
                    bigint_free(a);
                    bigint_free(b);
                    if (r == NULL) fputs("MATH ERROR\n", stderr);
                    else push(&st, r);
                }
            } else if (strcmp(tok, "v") == 0) {
                bigint_tp n = pop(&st);
                if (n == NULL)
                    fputs("ERROR: too few items in the stack\n", stderr);
                else {
                    bigint_tp r = bigint_sqrt(n);
                    bigint_free(n);
                    if (r == NULL) fputs("MATH ERROR\n", stderr);
                    else push(&st, r);
                }
            } else if (strcmp(tok, "p") == 0) {
                if (st == NULL)
                    fputs("ERROR: too few items in the stack\n", stderr);
                else {
                    bigint_tp n = st->n;
                    char *s = bigint_to_string(n);
                    puts(s);
                    free(s);
                }
            } else if (strcmp(tok, "n") == 0) {
                bigint_tp n = pop(&st);
                if (n == NULL)
                    fputs("ERROR: too few items in the stack\n", stderr);
                else {
                    char *s = bigint_to_string(n);
                    puts(s);
                    free(s);
                    bigint_free(n);
                }
            } else if (strcmp(tok, "d") == 0) {
                bigint_tp n = pop(&st);
                if (n == NULL)
                    fputs("ERROR: too few items in the stack\n", stderr);
                else {
                    bigint_tp n2 = bigint_dup(n);
                    push(&st, n);
                    push(&st, n2);
                }
            } else if (strcmp(tok, "r") == 0) {
                bigint_tp b = pop(&st);
                bigint_tp a = pop(&st);
                if (a == NULL || b == NULL)
                    fputs("ERROR: too few items in the stack\n", stderr);
                else {
                    push(&st, b);
                    push(&st, a);
                }
            } else if (strcmp(tok, "f") == 0) {
                for (struct stack *iter = st; iter; iter = iter->next) {
                    bigint_tp n = iter->n;
                    char *s = bigint_to_string(n);
                    puts(s);
                    free(s);
                }
            } else if (strcmp(tok, "q") == 0) {
                goto exit;
            } else if (strcmp(tok, "h") == 0 || strcmp(tok, "?") == 0) {
                puts(HELP);
            } else {
                // Is this a number?
                bigint_tp n = bigint_from_string(tok);
                if (n != NULL)
                    push(&st, n);
                else
                    fputs("ERROR: invalid number or command\n", stderr);
            }
        }
    }
exit:
    return 0;
}

