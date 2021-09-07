#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <errno.h>
#include <limits.h>

#include "assert.h"

#include "log_utils.h"
#include "mbedtls_dh.h"

size_t RandomBitsRead(void* buffer, size_t size);

#define BITS_PER_BYTE 8

/* Macros */

#ifndef MIN
#define MIN(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })
#endif
#ifndef MAX
#define MAX(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b;      \
    })
#endif

/*!
 * \brief Low-level wrapper around RDRAND instruction (get hardware-generated random value).
 */
static inline uint32_t rdrand(void) {
    uint32_t ret;
    __asm__ volatile(
        "1: .byte 0x0f, 0xc7, 0xf0\n" /* RDRAND %EAX */
        "jnc 1b\n"
        :"=a"(ret)
        :: "cc");
    return ret;
}

size_t RandomBitsRead(void* buffer, size_t size) {
    uint32_t rand;
    for (size_t i = 0; i < size; i += sizeof(rand)) {
        rand = rdrand();
        memcpy(buffer + i, &rand, MIN(sizeof(rand), size - i));
    }
    return 0;
}

int RandomWrapper(void* private_param, unsigned char* data, size_t size) {
    // __UNUSED(private_param);
    return RandomBitsRead(data, size);
}

//WL:
int lib_SHA256Init(LIB_SHA256_CONTEXT* context) {
    mbedtls_sha256_init(context);
    mbedtls_sha256_starts(context, 0 /* 0 = use SSH256 */);
    return 0;
}

int lib_SHA256Update(LIB_SHA256_CONTEXT* context, const uint8_t* data, size_t data_size) {
    /* For compatibility with other SHA256 providers, don't support
     * large lengths. */
    if (data_size > UINT32_MAX) {
        return -EINVAL;
    }
    mbedtls_sha256_update(context, data, data_size);
    return 0;
}

int lib_SHA256Final(LIB_SHA256_CONTEXT* context, uint8_t* output) {
    mbedtls_sha256_finish(context, output);
    /* This function is called free, but it doesn't actually free the memory.
     * It zeroes out the context to avoid potentially leaking information
     * about the hash that was just performed. */
    mbedtls_sha256_free(context);
    return 0;
}


int lib_DhInit(LIB_DH_CONTEXT* context) {
    int ret;
    mbedtls_dhm_init(context);

    /* Configure parameters. Note that custom Diffie-Hellman parameters
     * are considered more secure, but require more data be exchanged
     * between the two parties to establish the parameters, so we haven't
     * implemented that yet. */
    ret = mbedtls_mpi_read_string(&context->P, 16 /* radix */, MBEDTLS_DHM_RFC3526_MODP_2048_P);
    if (ret != 0) {
        printf("D-H initialization failed: %d\n", ret);
        return ret;
    }

    ret = mbedtls_mpi_read_string(&context->G, 16 /* radix */, MBEDTLS_DHM_RFC3526_MODP_2048_G);
    if (ret != 0) {
        printf("D-H initialization failed: %d\n", ret);
        return ret;
    }

    context->len = mbedtls_mpi_size(&context->P);

    return 0;
}

int lib_DhCreatePublic(LIB_DH_CONTEXT* context, uint8_t* public_param, size_t* public_size) {
    int ret;

    if (*public_size != DH_SIZE)
        return -EINVAL;

    /* The RNG here is used to generate secret exponent X. */
    ret = mbedtls_dhm_make_public(context, context->len, public_param, *public_size, RandomWrapper, NULL);
    if (ret != 0)
        return ret;

    /* mbedtls writes leading zeros in the big-endian output to pad to
     * public_size, so leave caller's public_size unchanged */
    return 0;
}

int lib_DhCalcSecret(LIB_DH_CONTEXT* context, uint8_t* peer, size_t peer_size, uint8_t* secret,
                     size_t* secret_size) {
    int ret;

    if (*secret_size != DH_SIZE)
        return -EINVAL;

    ret = mbedtls_dhm_read_public(context, peer, peer_size);
    if (ret != 0)
        return ret;

    /* The RNG here is used for blinding against timing attacks if X is
     * reused and not used otherwise. mbedtls recommends always passing
     * in an RNG. */
    ret = mbedtls_dhm_calc_secret(context, secret, *secret_size, secret_size, RandomWrapper, NULL);
    return ret;
}

void lib_DhFinal(LIB_DH_CONTEXT* context) {
    /* This call zeros out context for us. */
    mbedtls_dhm_free(context);
}
