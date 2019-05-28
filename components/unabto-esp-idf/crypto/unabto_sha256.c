/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/*
 * FILE:    sha2.c
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
 */

#include <string.h>   /* memcpy()/memset() or bcopy()/bzero() */
//#include <assert.h>   /* assert() */

#include "unabto/unabto_env_base.h"
#include "unabto/unabto_util.h"

#if NABTO_ENABLE_UCRYPTO

#include "unabto_sha256.h"

/*
 * ASSERT NOTE:
 * Some sanity checking code is included using assert().  On my FreeBSD
 * system, this additional code can be removed by compiling with NDEBUG
 * defined.  Check your own systems manpage on assert() to see how to
 * compile WITHOUT the sanity checking code on your system.
 *
 * UNROLLED TRANSFORM LOOP NOTE:
 * You can define SHA2_UNROLL_TRANSFORM to use the unrolled transform
 * loop version for the hash transform rounds (defined using macros
 * later in this file).  Either define on the command line, for example:
 *
 *   cc -DSHA2_UNROLL_TRANSFORM -o sha2 sha2.c sha2prog.c
 *
 * or define below:
 *
 *   #define SHA2_UNROLL_TRANSFORM
 *
 */


/*** SHA-256 Machine Architecture Definitions *****************/

/*
 * Define the followingsha2_* types to types of the correct length on
 * the native archtecture.   Most BSD systems and Linux define u_intXX_t
 * types.  Machines with very recent ANSI C headers, can use the
 * uintXX_t definintions from inttypes.h by defining SHA2_USE_INTTYPES_H
 * during compile or in the sha.h header file.
 *
 * Machines that support neither u_intXX_t nor inttypes.h's uintXX_t
 * will need to define these three typedefs below (and the appropriate
 * ones in sha.h too) by hand according to their system architecture.
 *
 * Thank you, Jun-ichiro itojun Hagino, for suggesting using u_intXX_t
 * types and pointing out recent ANSI C support for uintXX_t in inttypes.h.
 */
typedef uint8_t  sha2_byte;      /* Exactly 1 byte */
typedef uint32_t sha2_word32;    /* Exactly 4 bytes */
// typedef uint64_t sha2_word64;    /* Exactly 8 bytes */ 


/*** SHA-256/384/512 Various Length Definitions ***********************/
/* NOTE: Most of these are in sha2.h */
#define SHA256_SHORT_BLOCK_LENGTH   (SHA256_BLOCK_LENGTH - 8)


#define MEMSET_BZERO(p,l)   memset((p), 0, (l))
#define MEMCPY_BCOPY(d,s,l) memcpy((d), (s), (l))


/*** THE SIX LOGICAL FUNCTIONS ****************************************/
/*
 * Bit shifting and rotation (used by the six SHA-XYZ logical functions:
 *
 *   NOTE:  The naming of R and S appears backwards here (R is a SHIFT and
 *   S is a ROTATION) because the SHA-256/384/512 description document
 *   (see http://csrc.nist.gov/cryptval/shs/sha256-384-512.pdf) uses this
 *   same "backwards" definition.
 */
/* Shift-right (used in SHA-256, SHA-384, and SHA-512): */
#define R(b,x)      ((x) >> (b))
/* 32-bit Rotate-right (used in SHA-256): */
#define S32(b,x)    (((x) >> (b)) | ((x) << (32 - (b))))

/* Two of six logical functions used in SHA-256, SHA-384, and SHA-512: */
#define Ch(x,y,z)   (((x) & (y)) ^ ((~(x)) & (z)))
#define Maj(x,y,z)  (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

/* Four of six logical functions used in SHA-256: */
#define Sigma0_256(x)   (S32(2,  (x)) ^ S32(13, (x)) ^ S32(22, (x)))
#define Sigma1_256(x)   (S32(6,  (x)) ^ S32(11, (x)) ^ S32(25, (x)))
#define sigma0_256(x)   (S32(7,  (x)) ^ S32(18, (x)) ^ R(3 ,   (x)))
#define sigma1_256(x)   (S32(17, (x)) ^ S32(19, (x)) ^ R(10,   (x)))

/*** INTERNAL FUNCTION PROTOTYPES *************************************/
/* NOTE: These should not be accessed directly from outside this
 * library -- they are intended for private internal visibility/use
 * only.
 */
static void SHA256_Transform(sha256_ctx*, const sha2_word32*);


/*** SHA-XYZ INITIAL HASH VALUES AND CONSTANTS ************************/
/* Hash constant words K for SHA-256: */
static __ROM sha2_word32 K256[64] = {
    0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
    0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
    0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
    0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
    0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
    0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
    0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
    0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
    0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
    0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
    0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
    0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
    0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
    0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
    0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
    0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

/* Initial hash value H for SHA-256: */
 static __ROM sha2_word32 sha256_initial_hash_value[8] = {
    0x6a09e667UL,
    0xbb67ae85UL,
    0x3c6ef372UL,
    0xa54ff53aUL,
    0x510e527fUL,
    0x9b05688cUL,
    0x1f83d9abUL,
    0x5be0cd19UL
};

/*
 * Constant used by SHA256/384/512_End() functions for converting the
 * digest to a readable hexadecimal character string:
 */
//static const char *sha2_hex_digits = "0123456789abcdef";

/*** SHA-256: *********************************************************/

void unabto_sha256_init(sha256_ctx* context) {
#if UNABTO_PLATFORM_PIC18
  memcpypgm2ram(context->state, sha256_initial_hash_value, sizeof(sha256_initial_hash_value));
#else
    memcpy(context->state, sha256_initial_hash_value, sizeof(sha256_initial_hash_value));
#endif
    MEMSET_BZERO(context->buffer, SHA256_BLOCK_LENGTH);

    context->byteCount = 0;
}

static void SHA256_Transform(sha256_ctx* context, const sha2_word32* data) {
    sha2_word32 a, b, c, d, e, f, g, h, s0, s1;
    sha2_word32 T1, T2, *W256;
    uint8_t     j;

    W256 = (sha2_word32*)context->buffer;

    /* Initialize registers with the prev. intermediate value */
    a = context->state[0];
    b = context->state[1];
    c = context->state[2];
    d = context->state[3];
    e = context->state[4];
    f = context->state[5];
    g = context->state[6];
    h = context->state[7];
  

    j = 0;
    do {
        /* Copy data while converting to host byte order */
        READ_U32(W256[j],data++);

        /* Apply the SHA-256 compression function to update a..h */
        T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + W256[j];
        T2 = Sigma0_256(a) + Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;

        j++;
    } while (j < 16);

    do {
        /* Part of the message block expansion: */
        s0 = W256[(j+1)&0x0f];
        s0 = sigma0_256(s0);
        s1 = W256[(j+14)&0x0f];
        s1 = sigma1_256(s1);

        /* Apply the SHA-256 compression function to update a..h */
        T1 = h + Sigma1_256(e) + Ch(e, f, g) + K256[j] + (W256[j&0x0f] += s1 + W256[(j+9)&0x0f] + s0);
        T2 = Sigma0_256(a) + Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;

        j++;
    } while (j < 64);

    /* Compute the current intermediate hash value */
    context->state[0] += a;
    context->state[1] += b;
    context->state[2] += c;
    context->state[3] += d;
    context->state[4] += e;
    context->state[5] += f;
    context->state[6] += g;
    context->state[7] += h;

    /* Clean up */
    a = b = c = d = e = f = g = h = T1 = T2 = 0;
}

void unabto_sha256_update(sha256_ctx* context, const sha2_byte *data, uint16_t len) {
    uint16_t freespace, usedspace;

    if (len == 0) {
        /* Calling with no data is valid - we do nothing */
        return;
    }

    /* Sanity check: */
//    assert(context != (sha256_ctx*)0 && data != (sha2_byte*)0);

    usedspace = context->byteCount % SHA256_BLOCK_LENGTH;
    if (usedspace > 0) {
        /* Calculate how much free space is available in the buffer */
        freespace = SHA256_BLOCK_LENGTH - usedspace;

        if (len >= freespace) {
            /* Fill the buffer completely and process it */
            MEMCPY_BCOPY(&context->buffer[usedspace], data, freespace);
            context->byteCount += freespace;
            len -= freespace;
            data += freespace;
            SHA256_Transform(context, (sha2_word32*)context->buffer);
        } else {
            /* The buffer is not yet full */
            MEMCPY_BCOPY(&context->buffer[usedspace], data, len);
            context->byteCount += len;
            /* Clean up: */
            usedspace = freespace = 0;
            return;
        }
    }
//    NABTO_LOG_TRACE(("len %i", len));
    while (len >= SHA256_BLOCK_LENGTH) {
        /* Process as many complete blocks as we can */
        SHA256_Transform(context, (sha2_word32*)data);
        context->byteCount += SHA256_BLOCK_LENGTH;
        //    print_sha256_ctx(context);

        len -= SHA256_BLOCK_LENGTH;
        data += SHA256_BLOCK_LENGTH;
    }
    //  NABTO_LOG_TRACE((" i: %i",i));
    if (len > 0) {
        /* There's left-overs, so save 'em */
        MEMCPY_BCOPY(context->buffer, data, len);
        context->byteCount += len;
        //    print_sha256_ctx(context);
    }
    /* Clean up: */
    usedspace = freespace = 0;
}

void unabto_sha256_final(sha256_ctx* context,sha2_byte digest[]) {
    sha2_word32 *d = (sha2_word32*)digest;
    unsigned int usedspace;

    /* Sanity check: */
//    assert(context != (sha256_ctx*)0);

    /* If no digest buffer is passed, we don't bother doing this: */
    if (digest != (sha2_byte*)0) {
        usedspace = context->byteCount % SHA256_BLOCK_LENGTH;

        if (usedspace > 0) {
            /* Begin padding with a 1 bit: */
            context->buffer[usedspace++] = 0x80;

            if (usedspace <= SHA256_SHORT_BLOCK_LENGTH) {
                /* Set-up for the last transform: */
                MEMSET_BZERO(&context->buffer[usedspace], SHA256_BLOCK_LENGTH - usedspace);
            } else {
                if (usedspace < SHA256_BLOCK_LENGTH) {
                    MEMSET_BZERO(&context->buffer[usedspace], SHA256_BLOCK_LENGTH - usedspace);
                }
                /* Do second-to-last transform: */
                SHA256_Transform(context, (sha2_word32*)context->buffer);

                /* And set-up for the last transform: */
                MEMSET_BZERO(context->buffer, SHA256_BLOCK_LENGTH);
            }
        } else {
            /* Set-up for the last transform: */
            MEMSET_BZERO(context->buffer, SHA256_BLOCK_LENGTH);

            /* Begin padding with a 1 bit: */
            *context->buffer = 0x80;
        }

        /* Set the bit count: */
        /* Convert FROM host byte order */
    {
        // write length in bits in network byte order.
        uint16_t tmp16 = context->byteCount;
        tmp16 <<= 3;
        memset(&context->buffer[SHA256_BLOCK_LENGTH - 8], 0, 6);
        WRITE_U16(&context->buffer[SHA256_BLOCK_LENGTH - 2], tmp16);
    }
        //print_sha256_ctx(context);
        /* Final transform: */
        SHA256_Transform(context, (sha2_word32*)context->buffer);
        //print_sha256_ctx(context);

        {
            /* Convert TO host byte order */
            uint8_t j;
            for (j = 0; j < 8; j++) {
        WRITE_U32(d, context->state[j]);
        d++;
            }
        }
    }

    /* Clean up state data: */
    MEMSET_BZERO(context, sizeof(sha256_ctx));
    usedspace = 0;
}

#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_BUFFERS | NABTO_LOG_SEVERITY_TRACE)

//#define DEBUG_HMAC_SHA256 0

void print_sha256_ctx(sha256_ctx* ctx) {
#if DEBUG_HMAC_SHA256
    NABTO_LOG_TRACE(("byteCount %lu",ctx->byteCount));
//    NABTO_LOG_BUFFER(ctx->buffer, SHA256_BLOCK_LENGTH);
    /* NABTO_LOG_TRACE(("%02x %02x %02x %02x %02x %02x %02x %02x", */
    /*               ctx->buffer[0],ctx->buffer[1],ctx->buffer[2],ctx->buffer[3], */
    /*               ctx->buffer[4],ctx->buffer[5],ctx->buffer[6],ctx->buffer[7])); */
    //NABTO_LOG_BUFFER(ctx->block, 2*SHA256_BLOCK_SIZE);
    NABTO_LOG_TRACE(("h[0-3] %lu %lu %lu %lu",ctx->state[0],ctx->state[1],ctx->state[2],ctx->state[3]));
    NABTO_LOG_TRACE(("h[4-7] %lu %lu %lu %lu",ctx->state[4],ctx->state[5],ctx->state[6],ctx->state[7]));
#else
    NABTO_NOT_USED(ctx);
#endif
}

#endif

#endif // NABTO_ENABLE_UCRYPTO
