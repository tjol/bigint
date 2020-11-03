/* bigint library - bigint.h
   Function declarations / public interface.
   Copyright 2020 Thomas Jollans - see COPYING */

#ifndef _BIGINT_H_
#define _BIGINT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct _bigint {
    uint32_t digits;
    uint32_t num[];
};
typedef struct _bigint * bigint_tp;

inline bigint_tp bigint_dup(bigint_tp n);
inline void bigint_free(bigint_tp n);

inline bigint_tp bigint_from_int(int64_t i);
inline char *bigint_to_string(bigint_tp n);
inline bigint_tp bigint_from_string(const char *c);

inline int bigint_sgn(bigint_tp n);
inline int bigint_cmp32(bigint_tp n, int32_t m);
inline int bigint_cmp(bigint_tp n, bigint_tp m);

inline bigint_tp bigint_flipsign(bigint_tp n);
inline bigint_tp bigint_shift(bigint_tp n, int32_t shift);

inline bigint_tp bigint_add(bigint_tp n, bigint_tp m);
inline bigint_tp bigint_add_inplace(bigint_tp n, bigint_tp m);
inline bigint_tp bigint_add32(bigint_tp n, int32_t m);
inline bigint_tp bigint_add32_inplace(bigint_tp n, int32_t m);

inline bigint_tp bigint_mul(bigint_tp n, bigint_tp m);
inline bigint_tp bigint_mul32(bigint_tp n, int32_t m);
inline bigint_tp bigint_mul32_inplace(bigint_tp n, int32_t m);
inline bigint_tp bigint_mul32u(bigint_tp n, uint32_t m);
inline bigint_tp bigint_mul32u_inplace(bigint_tp n, uint32_t m);

inline bigint_tp bigint_div(bigint_tp n, bigint_tp d);
inline bigint_tp bigint_div32(bigint_tp numerator, int32_t denominator, int32_t *remainder);
inline bigint_tp bigint_div32_inplace(bigint_tp numerator, int32_t denominator, int32_t *remainder);

inline bigint_tp bigint_sqrt(bigint_tp n);

inline bigint_tp _bigint_new(uint32_t digits);
inline bigint_tp _bigint_realloc(bigint_tp n, uint32_t digits);
inline void _bigint_crop(bigint_tp n);
inline bigint_tp _bigint_find_quotient(bigint_tp n, bigint_tp d, bigint_tp overestimate, bigint_tp underestimate);
inline bigint_tp _bigint_find_sqrt(bigint_tp n, bigint_tp overestimate, bigint_tp underestimate);

#ifdef __cplusplus
} // extern "C"
#endif

#include "bigint_impl.h"

#endif /* _BIGINT_H_ */
