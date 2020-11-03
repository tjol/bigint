/* bigint library - unit tests
   Copyright 2020 Thomas Jollans - see COPYING */

#include <criterion/criterion.h>
#include "bigint.h"

Test(bigint_test, test_add32) {
    // basic 32-bit add
    char *s;
    bigint_tp i = bigint_from_int(1000);
    i = bigint_add32_inplace(i, 100);
    cr_assert(bigint_cmp32(i, 1100) == 0, "Basic addition");
    i = bigint_add32_inplace(i, -100);
    cr_assert(bigint_cmp32(i, 1000) == 0, "Basic subtraction");
    i = bigint_add32_inplace(i, -2000);
    cr_assert(bigint_cmp32(i, -1000) == 0, "Subtraction across zero");
    i = bigint_add32_inplace(i, 3000);
    cr_assert(bigint_cmp32(i, 2000) == 0, "Addition across zero");
    i = bigint_add32_inplace(i, 0x7fffffff);
    cr_assert_eq(i->digits, 2, "Addition with overflow must add a digit");
    s = bigint_to_string(i);
    cr_assert_str_eq(s, "2147485647", "Addition with overflow");
    free(s);
    bigint_free(i);
    i = bigint_from_int(-2147483648);
    i = bigint_add32_inplace(i, -10);
    cr_assert_eq(i->digits, 2, "Addition with underflow must add a digit");
    s = bigint_to_string(i);
    cr_assert_str_eq(s, "-2147483658", "Addition with underflow");
    free(s);
    bigint_tp r = bigint_add32(i, 2000000000);
    cr_assert_neq(r, i, "bigint_add32 creates a new number");
    s = bigint_to_string(i);
    cr_assert_str_eq(s, "-2147483658", "bigint_add32 does not change the argument");
    free(s);
    cr_assert_eq(r->num[0], (uint32_t)-147483658, "bigint_add32 must be correct");
    bigint_free(i);
    bigint_free(r);
}

Test(bigint_test, test_add) {
    // bigint add
    char *s;
    bigint_tp i = bigint_from_int(1000);
    bigint_tp j = bigint_from_int(100);
    i = bigint_add_inplace(i, j);
    cr_assert(bigint_cmp32(i, 1100) == 0, "Single-digit addition");

    bigint_free(j);
    j = bigint_from_string("23749238409823046709104012831203709123");
    i = bigint_add_inplace(i, j);
    s = bigint_to_string(i);
    cr_assert_str_eq(s, "23749238409823046709104012831203710223", "Big addition, right is bigger");
    free(s);

    bigint_free(j);
    j = bigint_from_int(-2379084);
    bigint_tp k = bigint_add(i, j);
    cr_assert_neq(k, i, "bigint_add creates a new number");
    s = bigint_to_string(k);
    cr_assert_str_eq(s, "23749238409823046709104012831201331139", "Big addition, left is bigger, right is negative");
    free(s);

    bigint_free(j);
    k = bigint_add32(k, 7137252);
    k = bigint_flipsign(k);
    i = bigint_add_inplace(i, k);
    cr_assert(bigint_cmp32(i, -4758168) == 0, "Subtract big number across zero");
    bigint_free(i);
    bigint_free(k);

    i = bigint_from_string("-20384017402174323840237402384");
    j = bigint_from_string("-1000000000000000000000000000000000000000000000000000000000000000000");
    i = bigint_add_inplace(i, j);
    s = bigint_to_string(i);
    cr_assert_str_eq(s, "-1000000000000000000000000000000000000020384017402174323840237402384",
        "Add big negative numbers");
    free(s);
    bigint_free(i);
    bigint_free(j);
}

Test(bigint_test, test_mul32) {
    char *s;
    bigint_tp i, j;

    i = bigint_from_int(10000000);
    i = bigint_mul32_inplace(i, 20000000);
    s = bigint_to_string(i);
    cr_assert_str_eq(s, "200000000000000", "Basic addition with overflow");
    free(s);
    i = bigint_mul32_inplace(i, -999);
    s = bigint_to_string(i);
    cr_assert_str_eq(s, "-199800000000000000", "Positive times negative is negative");
    free(s);
    i = bigint_mul32_inplace(i, 10);
    s = bigint_to_string(i);
    cr_assert_str_eq(s, "-1998000000000000000", "Negative times positive is negative");
    free(s);

    j = bigint_mul32(i, -100);

    s = bigint_to_string(i);
    cr_assert_str_eq(s, "-1998000000000000000", "bigint_mul32 leaves argument invariant");
    free(s);
    s = bigint_to_string(j);
    cr_assert_str_eq(s, "199800000000000000000", "Negative times negative is positive");
    free(s);
    bigint_free(j);
    bigint_free(i);

    i = bigint_from_int(100);
    i = bigint_mul32u_inplace(i, 0x80000000);
    s = bigint_to_string(i);
    cr_assert_str_eq(s, "214748364800", "bigint_mul32u_inplace does The Right Thing with overflow");
    free(s);
    bigint_free(i);
    i = bigint_from_int(1);
    i = bigint_mul32u_inplace(i, 0x80000000);
    s = bigint_to_string(i);
    cr_assert_str_eq(s, "2147483648", "bigint_mul32u_inplace does The Right Thing without overflow");
    cr_assert_eq(i->digits, 2, "bigint_mul32u_inplace does The Right Thing without overflow");
    cr_assert_eq(i->num[1], 0, "bigint_mul32u_inplace does The Right Thing without overflow");
    free(s);
    bigint_free(i);
}

Test(bigint_test, test_mul) {
    char *s;

    bigint_tp i, j, r;

    i = bigint_from_string("23497230472037409182301740983214");
    j = bigint_from_string("948593479263471208");
    r = bigint_mul(i, j);
    s = bigint_to_string(r);
    cr_assert_str_eq(s, "22289319606525621891508260125475180778856500302512", "bigint_mul with large positives");
    free(s);
    bigint_free(i);
    bigint_free(j);
    bigint_free(r);

    i = bigint_from_string("-10");
    j = bigint_from_string("1000000000000000000000000000000000000000000000000000000");
    r = bigint_mul(i, j);
    s = bigint_to_string(r);
    cr_assert_str_eq(s, "-10000000000000000000000000000000000000000000000000000000", "bigint_mul with one small negative and one large positive");
    free(s);
    bigint_free(i);
    bigint_free(j);
    bigint_free(r);


    i = bigint_from_string("-10");
    j = bigint_from_string("-1000000000000000000000000000000000000000000000000000000");
    r = bigint_mul(i, j);
    s = bigint_to_string(r);
    cr_assert_str_eq(s, "10000000000000000000000000000000000000000000000000000000", "bigint_mul with two negatives");
    free(s);
    bigint_free(i);
    bigint_free(j);
    bigint_free(r);
}

Test(bigint_test, test_div32) {
    char *s;
    bigint_tp n, r;
    int32_t rem = -1;

    n = bigint_from_string("1000000000000000000000000000000000000000000000000000000");

    n = bigint_div32_inplace(n, -10000, &rem);
    cr_assert_eq(rem, 0, "bigint_div32_inplace zero remainder");
    s = bigint_to_string(n);
    cr_assert_str_eq(s, "-100000000000000000000000000000000000000000000000000", "bigint_div32_inplace positive over negative is negative");
    free(s);

    n = bigint_div32_inplace(n, 23, &rem);
    s = bigint_to_string(n);
    cr_assert_str_eq(s, "-4347826086956521739130434782608695652173913043478", "bigint_div32_inplace rounds to zero");
    free(s);
    cr_assert_eq(rem, -6, "bigint_div32_inplace nonzero remainder");

    r = bigint_div32(n, -2, NULL);
    cr_assert_neq(n, r, "bigint_div32 returns a new number");
    s = bigint_to_string(n);
    cr_assert_str_eq(s, "-4347826086956521739130434782608695652173913043478", "bigint_div32 leaves argument intact");
    free(s);
    s = bigint_to_string(r);
    cr_assert_str_eq(s, "2173913043478260869565217391304347826086956521739", "bigint_div32 negative over negative is positive");
    free(s);

    bigint_free(r);

    r = bigint_div32_inplace(n, 0, NULL);
    cr_assert_eq(r, NULL, "bigint_div32_inplace refuses to divide by zero");

    bigint_free(n);
}


Test(bigint_test, test_div) {
    char *s;
    bigint_tp n, d, r;

    n = bigint_from_string("1000000000000000000000000000000000000000000000000000000");
    d = bigint_from_string("1000000000000000000000000000000000000000000000000000001");
    r = bigint_div(n, d);
    cr_assert(bigint_cmp32(r, 0) == 0, "bigint_div rounds to zero from positive");
    bigint_free(r);

    n = bigint_flipsign(n);
    d = bigint_flipsign(d);
    r = bigint_div(n, d);
    cr_assert(bigint_cmp32(r, 0) == 0, "bigint_div rounds to zero from negative");
    bigint_free(r);

    r = bigint_div(d, n);
    cr_assert(bigint_cmp32(r, 1) == 0, "bigint_div negative over negative is positive");
    bigint_free(r);

    bigint_free(n);
    bigint_free(d);

    n = bigint_from_string("74927340823023480293740928340923740234890");
    d = bigint_from_string("-9237492374060912834");
    r = bigint_div(n, d);
    s = bigint_to_string(r);
    cr_assert_str_eq(s, "-8111220858316608212883", "bigint_div rounds towards zero for negatives");
    free(s);
    bigint_free(r);

    bigint_free(n);
    bigint_free(d);

    n = bigint_from_string("564328754028304621037509913742034237047602730478071");
    d = bigint_from_string("2734067104670821367409127844802374");
    r = bigint_div(n, d);
    s = bigint_to_string(r);
    cr_assert_str_eq(s, "206406328895226283", "bigint_div rounds towards zero for positive");
    free(s);
    bigint_free(r);
    bigint_free(d);

    d = bigint_from_int(0);

    r = bigint_div(n, d);
    cr_assert_eq(r, NULL, "bigint_div refuses to divide by zero");

    bigint_free(n);
    bigint_free(d);
}

Test(bigint_test, test_sqrt) {
    char *s;
    bigint_tp n = bigint_from_string("23232328323215435345345345343458098856756556809400840980980980980809092343243243243243098799634");

    bigint_tp r = bigint_sqrt(n);
    s = bigint_to_string(r);
    cr_assert_str_eq(s, "152421548093487868711992623730429930751178496967", "bigint_sqrt");
    free(s);

    bigint_free(r);
    bigint_free(n);

    n = bigint_from_int(-100);
    r = bigint_sqrt(n);
    cr_assert_eq(r, NULL, "bigint_sqrt cannot take the square root of negative numbers");

    bigint_free(n);
}
