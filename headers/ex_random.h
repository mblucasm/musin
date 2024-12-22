/* Codigo extraido de https://github.com/EBI-Metagenomics/xrandom/blob/main/xrandom.h
 * Usage example
 * -------------
 *
 * struct xrandom rnd = xrandom(1);
 * double dbl_value = xrandom_dbl(&rnd);
 * uint64_t u64_value = xrandom_u64(&rnd);
 *
 * Acknowledgement
 * ---------------
 *
 * I'm using xoshiro256+ 1.0 (David Blackman and Sebastiano Vigna) and
 * splitmix64 (Sebastiano Vigna) methods for random number generation.
 *
 * - https://prng.di.unimi.it
 * - https://github.com/svaarala/duktape/blob/master/misc/splitmix64.c
 */

#ifndef XRANDOM_H
#define XRANDOM_H

#include <stdint.h>

struct xrandom
{
    uint64_t data[4];
};

/* A standard double (64-bit) floating-point number in IEEE floating point
 * format has 52 bits of significand, plus an implicit bit at the left of the
 * significand. Thus, the representation can actually store numbers with 53
 * significant binary digits. */
static inline double __xrandom_u64_to_dbl(uint64_t x)
{
    return (double)(x >> 11) * 0x1.0p-53;
}

static inline uint64_t __xrandom_rotl(const uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

static uint64_t xrandom_u64(struct xrandom *rnd)
{
    const uint64_t result = rnd->data[0] + rnd->data[3];

    const uint64_t t = rnd->data[1] << 17;

    rnd->data[2] ^= rnd->data[0];
    rnd->data[3] ^= rnd->data[1];
    rnd->data[1] ^= rnd->data[2];
    rnd->data[0] ^= rnd->data[3];

    rnd->data[2] ^= t;

    rnd->data[3] = __xrandom_rotl(rnd->data[3], 45);

    return result;
}

static uint64_t __xrandom_splitmix64_next(uint64_t *x)
{
    uint64_t z = (*x += UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}

struct xrandom xrandom(uint64_t seed)
{
    struct xrandom r;
    r.data[0] = __xrandom_splitmix64_next(&seed);
    r.data[1] = __xrandom_splitmix64_next(&seed);
    r.data[2] = __xrandom_splitmix64_next(&seed);
    r.data[3] = __xrandom_splitmix64_next(&seed);
    return r;
}

static inline double xrandom_dbl(struct xrandom *rnd)
{
    return __xrandom_u64_to_dbl(xrandom_u64(rnd));
}

#endif