
#include <unabto/unabto_env_base.h>

#include <unabto/unabto_external_environment.h>
#include <unabto_aes_cbc_test.h>
#include <unabto/unabto_aes_cbc.h>

/**
Case #4: Encrypting 64 bytes (4 blocks) using AES-CBC with 128-bit key
Key       : 0x56e47a38c5598974bc46903dba290349
IV        : 0x8ce82eefbea0da3c44699ed7db51b7d9
Plaintext : 0xa0a1a2a3a4a5a6a7a8a9aaabacadaeaf
              b0b1b2b3b4b5b6b7b8b9babbbcbdbebf
              c0c1c2c3c4c5c6c7c8c9cacbcccdcecf
              d0d1d2d3d4d5d6d7d8d9dadbdcdddedf
Ciphertext: 0xc30e32ffedc0774e6aff6af0869f71aa
              0f3af07a9a31a9c684db207eb0ef8e4e
              35907aa632c3ffdf868bb7b29d3d46ad
              83ce9f9a102ee99d49a53e87f4c3da55
 */


static uint8_t plaintext[80];


static const uint8_t key[16] = {0x56, 0xe4, 0x7a, 0x38, 0xc5, 0x59, 0x89, 0x74,
                                0xbc, 0x46, 0x90, 0x3d, 0xba, 0x29, 0x03, 0x49};

static const uint8_t iv[16] = {0x8c, 0xe8, 0x2e, 0xef, 0xbe, 0xa0, 0xda, 0x3c, 0x44, 0x69, 0x9e, 0xd7, 0xdb, 0x51, 0xb7, 0xd9};

// with iv
static uint8_t ciphertext[80] = {0x8c, 0xe8, 0x2e, 0xef, 0xbe, 0xa0, 0xda, 0x3c, 0x44, 0x69, 0x9e, 0xd7, 0xdb, 0x51, 0xb7, 0xd9,
                                 0xc3, 0x0e, 0x32, 0xff, 0xed, 0xc0, 0x77, 0x4e, 0x6a, 0xff, 0x6a, 0xf0, 0x86, 0x9f, 0x71, 0xaa,
                                 0x0f, 0x3a, 0xf0, 0x7a, 0x9a, 0x31, 0xa9, 0xc6, 0x84, 0xdb, 0x20, 0x7e, 0xb0, 0xef, 0x8e, 0x4e,
                                 0x35, 0x90, 0x7a, 0xa6, 0x32, 0xc3, 0xff, 0xdf, 0x86, 0x8b, 0xb7, 0xb2, 0x9d, 0x3d, 0x46, 0xad,
                                 0x83, 0xce, 0x9f, 0x9a, 0x10, 0x2e, 0xe9, 0x9d, 0x49, 0xa5, 0x3e, 0x87, 0xf4, 0xc3, 0xda, 0x55};

bool aes_cbc_test(void)
{
  bool ret = true;
  bool r;
  int i;

  memcpy(plaintext, iv, 16);
  for(i = 0; i < 64; i++)
  {
    plaintext[16 + i] = 160 + i;
  }

  if(!unabto_aes128_cbc_encrypt(key, plaintext, sizeof (plaintext)))
  {
    NABTO_LOG_TRACE(("aes_cbc_encrypt failed"));
  }
  r = (memcmp((void*) plaintext, (void*) ciphertext, sizeof (ciphertext)) == 0);
  ret &= r;
  if(!r)
  {
    NABTO_LOG_INFO(("Aes cbc encryption failed"));
  }

  if(!unabto_aes128_cbc_decrypt(key, ciphertext, sizeof (ciphertext)))
  {
    NABTO_LOG_TRACE(("aes_cbc decrypt failed"));
  }
  r = true;
  for(i = 0; i < 64; i++)
  {
    if(ciphertext[16 + i] != 160 + i)
    {
      r = false;
    }
  }

  ret &= r;
  if(!r)
  {
    NABTO_LOG_INFO(("Aes_cbc decryption failed"));
  }

  NABTO_LOG_INFO(("Aes_cbc decryption succeeded"));

  return ret;
}

int aes_cbc_timing_test(void)
{
  nabto_stamp_t future;
  int i = 0;

  nabtoSetFutureStamp(&future, 1000);

  while(!nabtoIsStampPassed(&future))
  {
    unabto_aes128_cbc_encrypt(key, plaintext, sizeof (plaintext));
    unabto_aes128_cbc_decrypt(key, plaintext, sizeof (plaintext));
    i++;
  }

  return i;
}

