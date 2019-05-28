/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include "unabto/unabto_env_base.h"

#if NABTO_ENABLE_UCRYPTO

#include "unabto/unabto_aes_cbc.h"
#include "modules/crypto/generic/unabto_aes.h"
#include "mbedtls/aes.h"



bool unabto_aes128_cbc_encrypt(const uint8_t* key, uint8_t* input, uint16_t input_len) {

    int ret=0;

    uint8_t cbc[16];

    
    mbedtls_aes_context aes;
    if ((input_len < 16) || (input_len % 16 != 0)) {
        return false;
    }

    //mbedtls_aes_init( &aes );

    ret = mbedtls_aes_setkey_enc( &aes, key, 128 );
    if(ret < 0) {
        mbedtls_aes_free(&aes );
        NABTO_LOG_ERROR(("AES error in encrypt - could not setkey_dec ret:%d", ret));
        return false;
    }

    // Crypto alters the IV.. so we need to copy it first
    memcpy(cbc, input, 16);
    ret = mbedtls_aes_crypt_cbc( &aes, MBEDTLS_AES_ENCRYPT, input_len-16, cbc, input+16, input+16);

    if(ret != 0) {
        NABTO_LOG_ERROR(("AES error in encrypt %d", ret));
    }
    //mbedtls_aes_free(&aes);
    return ret==0;
}

/**
 * we are running the algoritm backwards to eliminate the need to remember too much state.
 */
bool unabto_aes128_cbc_decrypt(const uint8_t* key, uint8_t* input, uint16_t input_len) {

    int ret = 0;
    
    mbedtls_aes_context aes;
    if ((input_len < 16) || (input_len % 16 != 0)) {
        return false;
    }

    mbedtls_aes_init( &aes );

    ret = mbedtls_aes_setkey_dec( &aes, key, 128 );

    if(ret < 0) {
        mbedtls_aes_free(&aes );
        NABTO_LOG_ERROR(("AES error in decrypt - could not setkey_dec ret:%d", ret));
        return false;
    }
    
    ret = mbedtls_aes_crypt_cbc( &aes, MBEDTLS_AES_DECRYPT, input_len-16, input, input+16, input+16);
    
    if(ret != 0) {
        NABTO_LOG_ERROR(("AES error in encrypt ret:%d", ret));
    }
    mbedtls_aes_free(&aes);
    return ret==0;
}

#endif /*NABTO_ENABLE_UCRYPTO*/
