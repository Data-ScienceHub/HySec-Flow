
#ifndef adapter_H
#define adapter_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#define SHA256_DIGEST_LEN 32

#include "mbedtls/cmac.h"
typedef struct AES LIB_AES_CONTEXT;

#include "mbedtls/dhm.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"

typedef mbedtls_sha256_context LIB_SHA256_CONTEXT;


/* DH_SIZE is tied to the choice of parameters in mbedtls_dh.c. */
#define DH_SIZE 256
typedef mbedtls_dhm_context LIB_DH_CONTEXT;
typedef mbedtls_rsa_context LIB_RSA_KEY;

/* SHA256 */
int lib_SHA256Init(LIB_SHA256_CONTEXT* context);
int lib_SHA256Update(LIB_SHA256_CONTEXT* context, const uint8_t* data, size_t data_size);
int lib_SHA256Final(LIB_SHA256_CONTEXT* context, uint8_t* output);


/* Diffie-Hellman Key Exchange */
int lib_DhInit(LIB_DH_CONTEXT* context);
int lib_DhCreatePublic(LIB_DH_CONTEXT* context, uint8_t* public_param, size_t* public_size);
int lib_DhCalcSecret(LIB_DH_CONTEXT* context, uint8_t* peer, size_t peer_size, uint8_t* secret,
                     size_t* secret_size);
void lib_DhFinal(LIB_DH_CONTEXT* context);

#endif
