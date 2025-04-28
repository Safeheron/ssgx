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

#ifndef UUID4_H
#define UUID4_H

#define UUID4_VERSION "1.0.0"
#define UUID4_LEN 37

#ifdef __cplusplus
extern "C" {
#endif

enum { UUID4_ESUCCESS = 0, UUID4_EFAILURE = -1 };

int uuid4_init(void);
void uuid4_generate(char* dst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
