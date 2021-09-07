# Overview

There are four folders containing two local attestation demos and two remote attestation demos, developed by ourselves and intel respectively.

- LA: local attestation implemented by us
- LocalAttestation: Intel sample code
- RA: remote attestation implemented by us
- RemoteAttestation: Intel sample code

# LA

There are two parts here: a LA server (responser/verifier) and a LA client (initiator/claimer). The server will be completely implemented but the client may only be implemented partially for debug use. The **REAL** client will be integrated into graphene-sgx [here](https://github.com/StanPlatinum/graphene-atstub).

We by design use C++ multithreading server and unix socket to do data exchange.

Both the Claimer's and Verifier's code are divided into Application parts and Enclave parts. run `make` at `LA/` will iteratively go into each sub dir and execute make in the dirs, and the binaries will be generated at `LA/bin`. Run the verifier first to start a server.

# References

- [SGX developer Guide](https://download.01.org/intel-sgx/sgx-linux/2.12/docs/Intel_SGX_Developer_Guide.pdf), p19-21
- [SGX 101, Local attestation chapter](https://sgx101.gitbook.io/sgx101/sgx-bootstrap/attestation/inter-process-local-attestation)
- [C++ socket multithreading programming](https://blog.csdn.net/qq_38506897/article/details/81135648)