/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_ENCRYPTION

#include <unabto/unabto_env_base.h>

#if NABTO_ENABLE_UCRYPTO


#include "mbedtls/md.h"
#include <unabto/unabto_hmac_sha256.h>
#include "unabto_sha256.h"

#include <string.h>

/* HMAC-SHA-256 functions */

//static sha256_ctx sha_ctx;
//static uint8_t block_pad[SHA256_BLOCK_LENGTH];

//void print_sha256_ctx(sha256_ctx* ctx);

void unabto_hmac_sha256_buffers(const unabto_buffer keys[], uint8_t keys_size,
                                const unabto_buffer messages[], uint8_t messages_size,
                                uint8_t *mac, uint16_t mac_size)
{

    uint16_t keySize = 0;
    uint16_t i;
    
    uint8_t key[UNABTO_SHA256_BLOCK_LENGTH];
    uint8_t* ptr;
    int res;

    uint8_t hmacResult[32];

    
    for (i = 0; i < keys_size; i++) {
        keySize += keys[i].size;
    }
    
    UNABTO_ASSERT(keySize <= UNABTO_SHA256_BLOCK_LENGTH);
    
    ptr = key;
    for(i = 0; i < keys_size; i++) {
        memcpy(ptr, (const void*) keys[i].data, keys[i].size);
        ptr += keys[i].size;
    }


    
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    
    
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char *) key, keySize);

    for (i = 0; i < messages_size; i++) {
        if (messages[i].size > 0) {
            res = mbedtls_md_hmac_update(&ctx, (const unsigned char *) messages[i].data, messages[i].size);
            if (res != 0) {
                NABTO_LOG_ERROR(("mbedtls_md_hmac_update error"));
            }
        }
    }
    

    mbedtls_md_hmac_finish(&ctx, hmacResult);
    mbedtls_md_free(&ctx);

    memcpy(mac, hmacResult, mac_size);

  
}

#endif /* NABTO_ENABLE_UCRYPTO */
