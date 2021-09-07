#define VERIFIER_FILENAME "EnclaveVerifier.signed.so"

#define DEBUG(fmt...)   \
    do {                         \
        printf("VERIFIER: "); \
        printf(fmt);     \
    } while (0)
