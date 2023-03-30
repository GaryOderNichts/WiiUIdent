#include "Utils.hpp"
#include <cstring>

#include <mbedtls/aes.h>
#include <mbedtls/sha256.h>

namespace Utils
{

std::string ToHexString(const void* data, size_t size)
{
    std::string str;
    for (size_t i = 0; i < size; ++i)
        str += Utils::sprintf("%02x", ((const uint8_t*) data)[i]);
    
    return str;
}

int AESDecrypt(const void* key, unsigned keybits, const void* iv, const void* in, void* out, size_t dataSize)
{
    int res;
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);

    if ((res = mbedtls_aes_setkey_dec(&ctx, (const unsigned char*)key, keybits)) != 0) {
        mbedtls_aes_free(&ctx);
        return res;
    }

    unsigned char tmpiv[16];
    if (iv) {
        memcpy(tmpiv, iv, sizeof(tmpiv));
    } else {
        memset(tmpiv, 0, sizeof(tmpiv));
    }

    res = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, dataSize, tmpiv, (const unsigned char*)in, (unsigned char*)out);
    mbedtls_aes_free(&ctx);
    return res;
}

int SHA256(const void* in, size_t inSize, void* out, size_t outSize)
{
    if (outSize != 32) {
        return -1;
    }

    int res;
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);

    if ((res = mbedtls_sha256_starts_ret(&ctx, 0)) != 0) {
        mbedtls_sha256_free(&ctx);
        return res;
    }

    if ((res = mbedtls_sha256_update_ret(&ctx, (const unsigned char*)in, inSize)) != 0) {
        mbedtls_sha256_free(&ctx);
        return res; 
    }

    res = mbedtls_sha256_finish_ret(&ctx, (unsigned char*)out);
    mbedtls_sha256_free(&ctx);
    return res; 
}

}
