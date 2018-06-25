#include "des.h"

void DesEncrypt8(const unsigned char *key, const unsigned char *plaintext, unsigned char *ciphertext)
{
    DES_key_schedule schedule;

    DES_set_key_unchecked((DES_cblock*)key, &schedule);
    DES_ecb_encrypt((DES_cblock*)plaintext, (DES_cblock*)ciphertext, &schedule, DES_ENCRYPT);
}

void DesDecrypt8(const unsigned char *key, const unsigned char *ciphertext, unsigned char *plaintext)
{
    DES_key_schedule schedule;

    DES_set_key_unchecked((DES_cblock*)key, &schedule);
    DES_ecb_encrypt((DES_cblock*)ciphertext, (DES_cblock*)plaintext, &schedule, DES_DECRYPT);
}

void DesEncrypt16(const unsigned char *key, const unsigned char *plaintext, unsigned char *ciphertext)
{
    DES_key_schedule schedule;

    DES_set_key_unchecked((DES_cblock*)key, &schedule);
    DES_ecb_encrypt((DES_cblock*)plaintext, (DES_cblock*)ciphertext, &schedule, DES_ENCRYPT);
    DES_set_key_unchecked((DES_cblock*)key, &schedule);
    DES_ecb_encrypt((DES_cblock*)(plaintext+8), (DES_cblock*)(ciphertext+8), &schedule, DES_ENCRYPT);
}

void DesDecrypt16(const unsigned char *key, const unsigned char *ciphertext, unsigned char *plaintext)
{
    DES_key_schedule schedule;

    DES_set_key_unchecked((DES_cblock*)key, &schedule);
    DES_ecb_encrypt((DES_cblock*)ciphertext, (DES_cblock*)plaintext, &schedule, DES_DECRYPT);
    DES_set_key_unchecked((DES_cblock*)key, &schedule);
    DES_ecb_encrypt((DES_cblock*)(ciphertext+8), (DES_cblock*)(plaintext+8), &schedule, DES_DECRYPT);
}


