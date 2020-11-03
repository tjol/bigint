/* bigint library - bigint_impl.h
   Function definitions (all inline).
   Copyright 2020 Thomas Jollans - see COPYING */

#ifndef _BIGINT_IMPL_H_
#define _BIGINT_IMPL_H_

#include "bigint.h"

#include <stdlib.h>
#include <string.h>

#define BIGINT_HIGH_MASK 0xFFFFFFFF00000000
#define BIGINT_LOW_MASK  0x00000000FFFFFFFF
#define BIGINT_WIDTH_BITS 32
#define BIGINT_SIGN_BIT 0x80000000

#ifndef _BIGINT_INLINE
# define _BIGINT_INLINE inline
#endif

#ifdef __cplusplus
extern "C"
{
#endif

_BIGINT_INLINE bigint_tp _bigint_new(uint32_t digits)
{
    bigint_tp res = malloc(sizeof(struct _bigint) + digits * sizeof(uint32_t));
    res->digits = digits;
    return res;
}

_BIGINT_INLINE bigint_tp _bigint_realloc(bigint_tp n, uint32_t digits)
{
    n = realloc(n, sizeof(struct _bigint) + digits * sizeof(uint32_t));
    n->digits = digits;
    return n;
}

_BIGINT_INLINE void bigint_free(bigint_tp n)
{
    free(n);
}

_BIGINT_INLINE bigint_tp bigint_dup(bigint_tp n)
{
    bigint_tp res = _bigint_new(n->digits);
    memcpy(res->num, n->num, n->digits * sizeof(uint32_t));
    return res;
}

_BIGINT_INLINE void _bigint_crop(bigint_tp n)
{
    while (n->digits > 1
        && ((n->num[n->digits-1] == 0 && !(n->num[n->digits-2] & BIGINT_SIGN_BIT))
            || (n->num[n->digits-1] == (uint32_t)-1l && (n->num[n->digits-2] & BIGINT_SIGN_BIT))))
                n->digits--;
}

_BIGINT_INLINE bigint_tp bigint_from_int(int64_t i)
{
    if ((i > 0 && (i & (BIGINT_HIGH_MASK | BIGINT_SIGN_BIT))) || (i < 0 && (~i & BIGINT_HIGH_MASK))) {
        // two digits
        bigint_tp res = _bigint_new(2);
        res->num[0] = i & BIGINT_LOW_MASK;
        res->num[1] = (i & BIGINT_HIGH_MASK) >> BIGINT_WIDTH_BITS;
        return res;
    } else {
        // one digit
        bigint_tp res = _bigint_new(1);
        res->num[0] = i;
        return res;
    }
}

_BIGINT_INLINE int bigint_sgn(bigint_tp n)
{
    return n->num[n->digits-1] & BIGINT_SIGN_BIT ? -1 : +1;
}

_BIGINT_INLINE int bigint_cmp32(bigint_tp n, int32_t m)
{
    int n_sign = bigint_sgn(n);
    int m_sign = m < 0 ? -1 : +1;
    if (n_sign > m_sign) return 1;
    else if (n_sign < m_sign) return -1;
    else if (n->digits > 1) return n_sign;
    else if ((int32_t)n->num[0] > m) return 1;
    else if ((int32_t)n->num[0] < m) return -1;
    else return 0;
}

_BIGINT_INLINE int bigint_cmp(bigint_tp n, bigint_tp m)
{
    int n_sign = bigint_sgn(n);
    int m_sign = bigint_sgn(m);
    if (n_sign > m_sign) return 1;
    else if (n_sign < m_sign) return -1;
    else if (n->digits > m->digits) return n_sign;
    else if (n->digits < m->digits) return n_sign;
    else {
        // same number of digits, same sign.
        for (int i = n->digits-1; i >= 0; --i) {
            if (n->num[i] > m->num[i]) return 1;
            else if (n->num[i] < m->num[i]) return -1;
        }
        return 0;
    }
}

_BIGINT_INLINE bigint_tp bigint_shift(bigint_tp n, int32_t shift)
{
    if (shift == 0) return n;
    else if (shift < 0) {
        // right shift
        shift = -shift;
        uint32_t BIGINT_sign_bit = n->num[n->digits-1] & BIGINT_SIGN_BIT;
        while (shift >= BIGINT_WIDTH_BITS) {
            shift -= BIGINT_WIDTH_BITS;
            memcpy(&n->num[0], &n->num[1], sizeof(uint32_t) * (--n->digits));
        }
        for (unsigned int i = 0; i < n->digits; ++i) {
            uint64_t val = n->num[i];
            if (i < n->digits - 1)
                val |= ((uint64_t)n->num[i+1]) << BIGINT_WIDTH_BITS;
            else
                // last digit
                if (BIGINT_sign_bit) val |= -1ll & BIGINT_HIGH_MASK;

            n->num[i] = val >> shift;
        }
        _bigint_crop(n);
        return n;
    } else {
        // left shift
        int shift_within_digit = shift % 32;
        uint32_t BIGINT_sign_bit = n->num[n->digits-1] & BIGINT_SIGN_BIT;
        uint32_t overflow = 0;
        for (unsigned int i = 0; i < n->digits; ++i) {
            uint64_t val = ((uint64_t)n->num[i] << shift_within_digit) | overflow;
            overflow = val >> BIGINT_WIDTH_BITS;
            n->num[i] = val;
        }
        // expand the number if needed
        int extra_digits = shift / 32;
        uint32_t next_val = ((BIGINT_sign_bit ? -1l : 0) << shift_within_digit) | overflow;
        uint32_t current_sign_bit = n->num[n->digits-1] & BIGINT_SIGN_BIT;
        int have_overflow_digit = current_sign_bit ? (next_val != (uint32_t)-1l)
                                                   : (next_val != 0);
        int old_len = n->digits;
        n = _bigint_realloc(n, old_len + extra_digits + have_overflow_digit);
        for (int i = old_len - 1; i >= 0; --i)
            n->num[i+extra_digits] = n->num[i];
        for (int i = 0; i < extra_digits; ++i)
            n->num[i] = 0;
        if (have_overflow_digit)
            n->num[n->digits-1] = next_val;
        return n;
    }
}

_BIGINT_INLINE bigint_tp bigint_add32_inplace(bigint_tp n, int32_t m)
{
    int n_sign = bigint_sgn(n);
    
    int m_sign = m < 0 ? -1 : +1;

    uint64_t carry = (uint32_t) m;
    uint64_t val;

    for (unsigned int i = 0; i < n->digits; ++i) {
        val = n->num[i] + carry;
        n->num[i] = val;
        carry = ((val & BIGINT_HIGH_MASK) >> BIGINT_WIDTH_BITS);
        if (carry != 0 && m_sign == -1) carry += (uint32_t) -1;
        if (carry == 0) {
            break;
        }
    }

    if ((n_sign & BIGINT_SIGN_BIT) != (val & BIGINT_SIGN_BIT) && (m_sign & BIGINT_SIGN_BIT) != (val & BIGINT_SIGN_BIT)) {
        n = _bigint_realloc(n, n->digits + 1);
        if (n_sign == -1) n->num[n->digits-1] = -1; // underflow
        else n->num[n->digits-1] = carry; // overflow
    }

    _bigint_crop(n);
    
    return n;
}

_BIGINT_INLINE bigint_tp bigint_add32(bigint_tp n, int32_t m)
{
    bigint_tp res = bigint_dup(n);
    return bigint_add32_inplace(res, m);
}

_BIGINT_INLINE bigint_tp bigint_add_inplace(bigint_tp n, bigint_tp m)
{
    int sgn_n = bigint_sgn(n);
    int sgn_m = bigint_sgn(m);
    int digits_n = n->digits;
    int digits_m = m->digits;
    int digits = digits_n >= digits_m ? digits_n : digits_m;

    uint64_t carry = 0;
    uint64_t val_n, val_m, sum;
    for (int i = 0; i < digits; ++i) {
        if (i < digits_n) val_n = n->num[i];
        else {
            n = _bigint_realloc(n, ++digits_n);
            val_n = sgn_n >= 0 ? 0 : (uint32_t)-1;
        }
        if (i < digits_m) val_m = m->num[i];
        else val_m = sgn_m >= 0 ? 0 : (uint32_t)-1;

        sum = val_n + val_m + carry;
        n->num[i] = sum;
        carry = ((sum & BIGINT_HIGH_MASK) >> BIGINT_WIDTH_BITS);
    }

    if ((sgn_n & BIGINT_SIGN_BIT) != (sum & BIGINT_SIGN_BIT) && (sgn_m & BIGINT_SIGN_BIT) != (sum & BIGINT_SIGN_BIT)) {
        n = _bigint_realloc(n, ++digits_n);
        if (sgn_n == -1) n->num[n->digits-1] = -1; // underflow
        else n->num[n->digits-1] = 0; // overflow
    }

    _bigint_crop(n);

    return n;
}

_BIGINT_INLINE bigint_tp bigint_add(bigint_tp n, bigint_tp m)
{
    bigint_tp res = bigint_dup(n);
    return bigint_add_inplace(res, m);
}

_BIGINT_INLINE bigint_tp bigint_flipsign(bigint_tp n)
{
    for (unsigned int i = 0; i < n->digits; ++i) {
        n->num[i] = ~n->num[i];
    }
    n = bigint_add32_inplace(n, 1);
    return n;
}

_BIGINT_INLINE bigint_tp bigint_div32_inplace(bigint_tp numerator, int32_t denominator, int32_t *remainder)
{
    if (denominator == 0) return NULL;

    int d_sign = denominator < 0 ? -1 : +1;
    denominator *= d_sign;
    int n_sign = bigint_sgn(numerator);
    int r_sign = d_sign * n_sign;

    if (n_sign < 0) {
        // flip the sign before dividing
        numerator = bigint_flipsign(numerator);
    }

    uint64_t rem = 0;
    for (int i = numerator->digits - 1; i >= 0; --i) {
        uint64_t val = numerator->num[i] + (rem << BIGINT_WIDTH_BITS);
        numerator->num[i] = val / (uint64_t)denominator;
        rem = val % denominator;
    }

    if (r_sign < 0) {
        // flip the sign
        numerator = bigint_flipsign(numerator);
    }

    // remove extraneous digits if possible
    _bigint_crop(numerator);

    if (remainder != NULL) *remainder = rem * r_sign;
    return numerator;
}

_BIGINT_INLINE bigint_tp bigint_div32(bigint_tp numerator, int32_t denominator, int32_t *remainder)
{
    if (denominator == 0) return NULL;

    bigint_tp res = bigint_dup(numerator);
    res = bigint_div32_inplace(res, denominator, remainder);
    return res;
}

_BIGINT_INLINE bigint_tp bigint_mul32u_inplace(bigint_tp n, uint32_t m)
{
    int n_sign = bigint_sgn(n);
    uint32_t carry = 0;

    if (n_sign < 0) {
        // flip the sign before multiplying
        n = bigint_flipsign(n);
    }

    for (unsigned int i = 0; i < n->digits; ++i) {
        uint64_t val = n->num[i];
        val *= m;
        val += carry;
        carry = (val & BIGINT_HIGH_MASK) >> BIGINT_WIDTH_BITS;
        n->num[i] = val & BIGINT_LOW_MASK;
    }

    if (carry != 0 || (n->num[n->digits-1] & BIGINT_SIGN_BIT)) { // result must be positive at this point
        n = _bigint_realloc(n, n->digits + 1);
        n->num[n->digits-1] = carry;
    }

    if (n_sign < 0) {
        // flip the sign
        n = bigint_flipsign(n);
    }

    return n;
}

_BIGINT_INLINE bigint_tp bigint_mul32_inplace(bigint_tp n, int32_t m)
{
    uint32_t m_u = m >= 0 ? m : -m;
    n = bigint_mul32u_inplace(n, m_u);
    if (m < 0) n = bigint_flipsign(n);
    return n;
}

_BIGINT_INLINE bigint_tp bigint_mul32(bigint_tp n, int32_t m)
{
    bigint_tp res = bigint_dup(n);
    return bigint_mul32_inplace(res, m);
}

_BIGINT_INLINE bigint_tp bigint_mul32u(bigint_tp n, uint32_t m)
{
    bigint_tp res = bigint_dup(n);
    return bigint_mul32u_inplace(res, m);
}


_BIGINT_INLINE bigint_tp bigint_mul(bigint_tp n, bigint_tp m)
{
    bigint_tp res = bigint_from_int(0);
    int m_sign = bigint_sgn(m);
    if (m_sign < 0) {
        m = bigint_dup(m);
        m = bigint_flipsign(m);
    }

    for (int i = m->digits - 1; i >= 0; --i) {
        bigint_tp tmp = bigint_mul32u(n, m->num[i]);
        tmp = bigint_shift(tmp, i * BIGINT_WIDTH_BITS);
        res = bigint_add_inplace(res, tmp);
        bigint_free(tmp);
    }

    if (m_sign < 0) {
        bigint_free(m);
        res = bigint_flipsign(res);
    }
    return res;
}

_BIGINT_INLINE bigint_tp _bigint_find_quotient(bigint_tp n, bigint_tp d,
                                               bigint_tp overestimate,
                                               bigint_tp underestimate)
{
    bigint_tp trial = bigint_add(overestimate, underestimate);
    trial = bigint_shift(trial, -1);
    if (bigint_cmp(trial, underestimate) == 0)
        return trial;
    bigint_tp res = bigint_mul(trial, d);
    bigint_tp rec_res;
    switch (bigint_cmp(res, n)) {
    case 0: // correct
        bigint_free(res);
        return trial;
    case 1: // to big
        bigint_free(res);
        rec_res = _bigint_find_quotient(n, d, trial, underestimate);
        bigint_free(trial);
        return rec_res;
    default:
    case -1:
        bigint_free(res);
        rec_res = _bigint_find_quotient(n, d, overestimate, trial);
        bigint_free(trial);
        return rec_res;
    }
}

_BIGINT_INLINE bigint_tp bigint_div(bigint_tp n, bigint_tp d)
{
    int n_sgn = bigint_sgn(n);
    int d_sgn = bigint_sgn(d);
    int q_sgn = n_sgn * d_sgn;

    // simple cases
    if (bigint_cmp32(d, 0) == 0) return NULL;
    else if (bigint_cmp32(d, d_sgn) == 0) {
        if (d_sgn == 1) return bigint_dup(n);
        else return bigint_flipsign(bigint_dup(n));
    } else if (d->digits == 1) {
        return bigint_div32(n, d->num[0], NULL);
    } else if (q_sgn == 1) {
        int cmp_result = bigint_cmp(n, d);
        if (cmp_result == 0) return bigint_from_int(1);
        else if (cmp_result == -n_sgn) return bigint_from_int(0);
    } else if (q_sgn == -1) {
        bigint_tp d2 = bigint_flipsign(bigint_dup(d));
        int cmp_result = bigint_cmp(n, d2);
        bigint_free(d2);
        if (cmp_result == 0) return bigint_from_int(-1);
        else if (cmp_result == -n_sgn) return bigint_from_int(0);
    }

    // sad times, we must search!
    bigint_tp overestimate, underestimate;
    // make everything positive, this makes the logic simpler
    if (n_sgn < 0) n = bigint_flipsign(bigint_dup(n));
    if (d_sgn < 0) d = bigint_flipsign(bigint_dup(d));

    overestimate = n;
    underestimate = bigint_from_int(-1);

    bigint_tp q = _bigint_find_quotient(n, d, overestimate, underestimate);
    if (q_sgn < 0) q = bigint_flipsign(q);
    bigint_free(underestimate);
    if (n_sgn < 0) bigint_free(n);
    if (d_sgn < 0) bigint_free(d);
    return q;
}

_BIGINT_INLINE bigint_tp _bigint_find_sqrt(bigint_tp n,
                                          bigint_tp overestimate,
                                          bigint_tp underestimate)
{
    bigint_tp trial = bigint_add(overestimate, underestimate);
    trial = bigint_shift(trial, -1);
    if (bigint_cmp(trial, underestimate) == 0)
        return trial;
    bigint_tp res = bigint_mul(trial, trial);
    bigint_tp rec_res;
    switch (bigint_cmp(res, n)) {
    case 0: // correct
        bigint_free(res);
        return trial;
    case 1: // to big
        bigint_free(res);
        rec_res = _bigint_find_sqrt(n, trial, underestimate);
        bigint_free(trial);
        return rec_res;
    default:
    case -1:
        bigint_free(res);
        rec_res = _bigint_find_sqrt(n, overestimate, trial);
        bigint_free(trial);
        return rec_res;
    }
}

_BIGINT_INLINE bigint_tp bigint_sqrt(bigint_tp n)
{
    int n_sgn = bigint_sgn(n);

    // simple cases
    if (n_sgn < 0) return NULL;
    else if (bigint_cmp32(n, 0) == 0) return bigint_from_int(0);
    else if (bigint_cmp32(n, 1) == 0) return bigint_from_int(1);

    // sad times, we must search!
    bigint_tp overestimate = n;
    bigint_tp underestimate = bigint_from_int(1);

    bigint_tp sqrt = _bigint_find_sqrt(n, overestimate, underestimate);
    bigint_free(underestimate);
    return sqrt;
}

_BIGINT_INLINE char *bigint_to_string(bigint_tp n)
{
    int sign = bigint_sgn(n);
    bigint_tp n2 = bigint_mul32(n, sign);

    char *s = malloc(2 + 10 * n->digits);
    // write the number backwards
    char *p = s;
    do {
        int32_t remainder;
        n2 = bigint_div32_inplace(n2, 10, &remainder);
        *(p++) = remainder + '0';
    } while (n2->digits > 1 || n2->num[0] != 0);

    if (sign == -1) *(p++) = '-';
    *p = '\0';

    // reverse
    char *b = s, *e = (p-1);
    while (b < e) {
        char tmp = *b;
        *(b++) = *e;
        *(e--) = tmp;
    }

    bigint_free(n2);

    return realloc(s, p - s + 1);
}

_BIGINT_INLINE bigint_tp bigint_from_string(const char *c)
{
    bigint_tp res = bigint_from_int(0);
    int is_negative = 0;
    if (*c == '-') {
        is_negative = 1;
        c++;
    }
    for (; *c; ++c) {
        res = bigint_mul32_inplace(res, 10);
        int i = *c - '0';
        if (i < 0 || i > 9) {
            // error!
            bigint_free(res);
            return NULL;
        }
        res = bigint_add32_inplace(res, i);
    }
    if (is_negative) res = bigint_flipsign(res);
    return res;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* _BIGINT_IMPL_H */

