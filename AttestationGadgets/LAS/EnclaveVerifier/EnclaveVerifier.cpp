#include "sgx_eid.h"

#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */
#include <string.h>

#include "EnclaveVerifier_t.h"  /* print_string */
#include "sgx_trts.h"

#include "sgx_utils.h"
#include "sgx_report.h"
#include "sgx_tcrypto.h"

#include "log_utils.h"
#include "g_pal.h"

#include "assert.h"

#include "datatypes.h"

//WL: we don't use SGXSDK's crypto lib. we use mbedtls instead.
// #include "sgx_dh.h"
#include "mbedtls_dh.h"

int streamread(int connfd, uint64_t offset, uint64_t len, void* buf,
                        char* conn_addr, size_t conn_addrlen) {
    //WL: offset, conn_addr, conn_addrlen are unused currently
    //WL: int flag = 0
    //WL: no need to use ms
    // ms_ocall_send_t* ms;

    int bytes;
    ocall_recv(&bytes, connfd, buf, len, 0);
    return bytes;
}

int streamwrite(int connfd, uint64_t offset, uint64_t len, const void* buf,
                       const char* conn_addr, int conn_addrlen){
    int bytes;
    ocall_send(&bytes, connfd, buf, len, 0);
    return bytes;
}


int las_dh_exchange(int stream, g_PAL_SESSION_KEY* key) {
    uint8_t pub[DH_SIZE]   __attribute__((aligned(DH_SIZE)));
    uint8_t agree[DH_SIZE] __attribute__((aligned(DH_SIZE)));
    g_PAL_NUM pubsz, agreesz;
    LIB_DH_CONTEXT context;
    int64_t bytes;
    int64_t ret;

    ret = lib_DhInit(&context);
    if (ret < 0) {
        printf("Key Exchange: DH Init failed: %ld\n", ret);
        goto out_no_final;
    }

    pubsz = sizeof pub;
    ret = lib_DhCreatePublic(&context, pub, &pubsz);
    if (ret < 0) {
        printf("Key Exchange: DH CreatePublic failed: %ld\n", ret);
        goto out;
    }

    assert(pubsz > 0 && pubsz <= DH_SIZE);
    if (pubsz < DH_SIZE) {
        /* Insert leading zero bytes if necessary. These values are big-
         * endian, so we either need to know the length of the bignum or
         * zero-pad at the beginning instead of the end. This code chooses
         * to do the latter. */
        memmove(pub + (DH_SIZE - pubsz), pub, pubsz);
        memset(pub, 0, DH_SIZE - pubsz);
    }

    printf("DH: wait for reading stream...\n");
    for (bytes = 0, ret = 0; bytes < DH_SIZE; bytes += ret) {
        //WL: read from socket
        //WL: need a Ocall: streamread()
        ret = streamread(stream, 0, DH_SIZE - bytes, pub + bytes, NULL, 0);
        if (ret < 0) {
            if (ret != 0) {
                ret = 0;
                continue;
            }
            printf("Failed to exchange the secret key via RPC: %ld\n", ret);
            goto out;
        }
    }

    printf("DH: start to write...\n");
    for (bytes = 0, ret = 0; bytes < DH_SIZE; bytes += ret) {
        //WL: write the socket
        //WL: need a Ocall: streamwrite()
        ret = streamwrite(stream, 0, DH_SIZE - bytes, pub + bytes, NULL, 0);
        if (ret < 0) {
            if (ret != 0) {
                ret = 0;
                continue;
            }
            printf("Failed to exchange the secret key via RPC: %ld\n", ret);
            goto out;
        }
    }

    agreesz = sizeof agree;
    ret = lib_DhCalcSecret(&context, pub, DH_SIZE, agree, &agreesz);
    if (ret < 0) {
        printf("Key Exchange: DH CalcSecret failed: %ld\n", ret);
        goto out;
    }

    assert(agreesz > 0 && agreesz <= sizeof agree);

    /*
     * Using SHA256 as a KDF to convert the 128-byte DH secret to a 256-bit AES key.
     * According to the NIST recommendation:
     * https://nvlpubs.nist.gov/nistpubs/SpecialPublications/NIST.SP.800-56Cr1.pdf,
     * a key derivation function (KDF) can be a secure hash function (e.g., SHA-256),
     * HMAC, or KMAC.
     */
    LIB_SHA256_CONTEXT sha;
    if ((ret = lib_SHA256Init(&sha)) < 0 ||
        (ret = lib_SHA256Update(&sha, agree, agreesz)) < 0 ||
        (ret = lib_SHA256Final(&sha, (uint8_t*)key)) < 0) {
        printf("Failed to derive the session key: %ld\n", ret);
        goto out;
    }

    printf("Key exchange succeeded\nkey 15th: %d\n", (*key)[15]);
    ret = 0;
out:
    lib_DhFinal(&context);
out_no_final:
    return ret;
}


//temp key
static sgx_aes_gcm_128bit_key_t key = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};

void ecall_process_dh(int connfd){
    g_PAL_SESSION_KEY key_store;
    int rv = las_dh_exchange(connfd, &key_store);
    printf("rv from dh: %d\n");
}


/* 
 * msg0 here is the target info, currently 
 * packet length (msg0_len, msg1_len) is 1024 for now
 */
void ecall_process_msg0(char *msg0_buf, size_t msg0_len, char *msg1_buf, size_t msg1_len) {

    sgx_target_info_t target_info;
    sgx_report_t temp_report;
    sgx_status_t se_ret;

    memset(&target_info, 0, sizeof(target_info));
    //Here msg0_buf is a single target info
    memcpy(&target_info, msg0_buf, sizeof(sgx_target_info_t));
    //Create Report to get target info which targeted towards the initiator of the session
    se_ret = sgx_create_report(&target_info, nullptr, &temp_report);
    if(se_ret != SGX_SUCCESS)
    {
        printf("create report failed\n");
    }
    else {
        printf("Create report successfully\n");
        memcpy(msg1_buf, &temp_report, sizeof(sgx_report_t));
    }
    return;

}

int report_match(sgx_measurement_t m1, sgx_measurement_t m2) {
    for (int i = 0; i < SGX_HASH_SIZE; i++) {
        if (m1.m[i] != m2.m[i]) {
            return -1;
        }
    }
    return 0;
}

/* 
 * msg2 here is the report from Graphene instance, currently 
 * packet length is 1024 for now
 */
void ecall_verify_msg2(char *msg2_buf, size_t msg2_len, char *msg3_buf, size_t msg3_len) {

    sgx_report_t temp_report, report2b_verified;
    sgx_measurement_t temp_mr_signer, mr_signer2b_verified;
    sgx_status_t se_ret;

    //Create Report
    //WL: not sure if we need create report every single time
    se_ret = sgx_create_report(nullptr, nullptr, &temp_report);
    if(se_ret != SGX_SUCCESS)
    {
        printf("Create report failed\n");
        return;
    }
    else {
        temp_mr_signer = temp_report.body.mr_signer;
        //TODO: check if temp_report's mr_signer is consistent with msg2's mr_signer
        //first we should copy the private key (pem) into Graphene's signer tool folder
        memcpy(&report2b_verified, msg2_buf, sizeof(sgx_report_t));
        mr_signer2b_verified = report2b_verified.body.mr_signer;
        if (report_match(mr_signer2b_verified, temp_mr_signer) == 0) {
            printf("Verify report successfully\n");
        }
        else {
            printf("Verify report failed\n");
            return;
            //return if failed, donot send msg3
        }
        //Prepare msg3
        //Msg3 here is a key
        memcpy(msg3_buf, &key, sizeof(sgx_aes_gcm_128bit_key_t));
    }
    return;

}