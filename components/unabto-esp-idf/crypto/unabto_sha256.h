/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/*
 * FILE:    sha2.h
 * AUTHOR:  Aaron D. Gifford - http://www.aarongifford.com/
 * 
 * Copyright (c) 2000-2001, Aaron D. Gifford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTOR(S) ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTOR(S) BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: sha2.h,v 1.1 2001/11/08 00:02:01 adg Exp adg $
 */

#ifndef _SHA256_H_
#define _SHA256_H_

#if NABTO_SLIM
#include <unabto_platform_types.h>
#else
#include <unabto/unabto_env_base.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum {
    SHA256_BLOCK_LENGTH = 64,
#ifndef SHA256_DIGEST_LENGTH // Defined in openssl hence we need to avoid collision.
    SHA256_DIGEST_LENGTH = 32,
#endif
    SHA256_BLOCK_SIZE = SHA256_BLOCK_LENGTH, // deprecated
    SHA256_DIGEST_SIZE = SHA256_DIGEST_LENGTH // deprecated
};


/*** SHA-256/384/512 Various Length Definitions ***********************/

typedef struct {
    uint32_t state[8];
    uint8_t buffer[SHA256_BLOCK_LENGTH];
    uint16_t byteCount;
} sha256_ctx;


/**
 * Initialize the sha256 context
 * @param sha256_ctx context to be initialized
 */
void unabto_sha256_init(sha256_ctx *context);

/**
 * Update the sha256 context with the given data
 */
void unabto_sha256_update(sha256_ctx* context, const uint8_t* data, uint16_t length);

void unabto_sha256_final(sha256_ctx* context, uint8_t* digest);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
