/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_AES_H_
#define _UNABTO_AES_H_

#include "unabto/unabto_env_base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AES_MAXROUNDS 14
#define AES_IV_SIZE 16
#define AES_BLOCKSIZE 16

typedef struct aes_key_st
{
    uint16_t rounds;
    uint16_t key_size;
    uint32_t ks[(AES_MAXROUNDS+1)*8];
    uint8_t iv[AES_IV_SIZE];
} AES_CTX;

typedef enum
{
    AES_MODE_128,
    AES_MODE_256
} AES_MODE;

void AES_encrypt(const AES_CTX *ctx, uint32_t *data);
void AES_decrypt(const AES_CTX *ctx, uint32_t *data);

void AES_set_key(AES_CTX *ctx, const uint8_t* key,
                 const uint8_t *iv, AES_MODE mode);

void AES_convert_key(AES_CTX *ctx);

void AES_cbc_encrypt(AES_CTX *ctx, const uint8_t *msg, uint8_t *out, int length);

void AES_cbc_decrypt(AES_CTX *ctx, const uint8_t *msg, uint8_t *out, int length);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
