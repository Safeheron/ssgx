/**
 * Copyright (c) 2018 rxi
 * Copyright (c) 2022 hejianhong@safeheron.com
 *
 * This file is free software; you can redistribute and/or modify
 * under the terms of the MIT license. See LICENSE for details.
 */

/* Modifications by hejianhong@safeheron.com on 2022-01-08:
 * - Utilized secure random number generation from SGX-enabled CPUs as the seed.
 */

#include "uuid4.h"

#include <stdint.h>
#include <stdio.h>

#include "sgx_trts.h"

static uint64_t seed[2];

static uint64_t xorshift128plus(uint64_t* s) {
    /* http://xorshift.di.unimi.it/xorshift128plus.c */
    uint64_t s1 = s[0];
    const uint64_t s0 = s[1];
    s[0] = s0;
    s1 ^= s1 << 23;
    s[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);
    return s[1] + s0;
}

int uuid4_init(void) {
    sgx_status_t ret;
    ret = sgx_read_rand((uint8_t*)seed, sizeof(seed));
    if (ret != SGX_SUCCESS) {
        return UUID4_EFAILURE;
    }
    return UUID4_ESUCCESS;
}

void uuid4_generate(char* dst) {
    static const char* template = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
    static const char* chars = "0123456789abcdef";
    union {
        unsigned char b[16];
        uint64_t word[2];
    } s;
    const char* p;
    int i, n;
    /* get random */
    s.word[0] = xorshift128plus(seed);
    s.word[1] = xorshift128plus(seed);
    /* build string */
    p = template;
    i = 0;
    while (*p) {
        n = s.b[i >> 1];
        n = (i & 1) ? (n >> 4) : (n & 0xf);
        switch (*p) {
        case 'x':
            *dst = chars[n];
            i++;
            break;
        case 'y':
            *dst = chars[(n & 0x3) + 8];
            i++;
            break;
        default:
            *dst = *p;
        }
        dst++, p++;
    }
    *dst = '\0';
}
