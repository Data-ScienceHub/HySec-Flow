#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <linux/socket.h>
// #include <linux/uio.h>
#include <stdint.h>

#include "CLATask.h"
#include "Thread.h"
#include "CTask.h"
#include "datatypes.h"

#include "sgx_urts.h"
#include "sgx_report.h"
#include "sgx_tcrypto.h"

#include "EnclaveVerifier_u.h"

//using the global eid defined in App.cpp
extern sgx_enclave_id_t global_eid;

#define MSG0_SIZE sizeof(sgx_target_info_t)

//WL: here comes the sgx_ocall_send implementation from Graphene
// struct msghdr
// {
//     void *msg_name;
//     int msg_namelen;
//     struct iovec *msg_iov;
//     size_t msg_iovlen;
//     void *msg_control;
//     size_t msg_controllen;
//     int msg_flags;
// };

int ocall_send(int sockfd, const void *buf, size_t len, int flags)
{
    int rv;
    struct msghdr msg;
    struct iovec iov[1];
    struct sockaddr_storage conn_addr;
    size_t conn_addrlen = sizeof(conn_addr);

    iov[0].iov_base    = (void*)buf;
    iov[0].iov_len     = len;
    msg.msg_name = &conn_addr;
    msg.msg_namelen = sizeof(conn_addr);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;

    printf("Waiting on sendmsg\n");
    rv = sendmsg(sockfd, &msg, 0);
    if (rv < 0)
    {
        perror("sendmsg() failed");
        exit(-1);
    }
    //WL: simple recv does not work
    // int rv = send(sockfd, buf, len, flags);
    return rv;
}

int ocall_recv(int sockfd, void *buf, size_t len, int flags)
{
    int rv;
    struct iovec iov[1];
    struct msghdr msg;

    /*************************************************/
    /* Initialize message header structure           */
    /*************************************************/
    memset(&msg, 0, sizeof(msg));
    memset(iov, 0, sizeof(iov));
    struct sockaddr_storage conn_addr;
    /*************************************************/
    /* The recvmsg() call will NOT block unless a    */
    /* non-zero length data buffer is specified      */
    /*************************************************/
    iov[0].iov_base = buf;
    iov[0].iov_len = len;
    msg.msg_name = &conn_addr;
    msg.msg_namelen = sizeof(conn_addr);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;

    /*************************************************/
    /* Wait for the descriptor to arrive             */
    /*************************************************/
    printf("Waiting on recvmsg\n");
    rv = recvmsg(sockfd, &msg, 0);
    if (rv < 0)
    {
        perror("recvmsg() failed");
        exit(-1);
    }

    //WL: simple recv does not work
    // int rv = recv(sockfd, buf, len, flags);
    return rv;
}

int CLATask::Run()
{

    int connfd = GetConnFd();
    while (1)
    {
        // BUFFSIZE = 1024
        // char msg0_buf[MSG0_SIZE];
        char msg0_buf[BUFFSIZE];
        char msg1_buf[BUFFSIZE];
        char msg2_buf[BUFFSIZE];
        char msg3_buf[BUFFSIZE];

        //WL: do the DH
        // Step1:
        printf("begin with DH...\n");
        ecall_process_dh(global_eid, connfd);

        memset(msg0_buf, 0x00, sizeof(msg0_buf));
        int bytes = recv(connfd, msg0_buf, sizeof(msg0_buf), 0);
        if (bytes <= 0)
        {
            printf("No msg0 received. Wait for a while...\n");
            //TODO: time-out
            break;
        }

        //WL: do the LA
        // Step1: extract target info and use the recved target info to generate a report

        //init dbg variables
        sgx_target_info_t target_info_dbg;
        memset(&target_info_dbg, 0, sizeof(target_info_dbg));
        sgx_report_t report_dbg;
        memset(&report_dbg, 0, sizeof(report_dbg));
        sgx_aes_gcm_128bit_key_t key_dbg;
        memset(&key_dbg, 0, sizeof(key_dbg));

        memcpy(&target_info_dbg, &msg0_buf, sizeof(sgx_target_info_t));
        // printf("Receiving attr.flag: %016lx\n", target_info_dbg.attributes.flags);
        // printf("Receiving attr.xfrm: %016lx\n", target_info_dbg.attributes.xfrm);
        // fflush(stdout);

        // clean the msg0_buf for the first connection
        if (!target_info_dbg.attributes.xfrm)
        {
            // bzero(msg0_buf, sizeof(msg0_buf));
            printf("Not correct msg0...\n");
            printf("Next round (next 1024)...\n");
            // continue;
            //WL: donot use continue here!!!
        }
        printf("Starting a new LA session...\n");

        ecall_process_msg0(global_eid, msg0_buf, sizeof(msg0_buf), msg1_buf, sizeof(msg1_buf));
        bzero(msg0_buf, sizeof(msg0_buf));
        //send msg1
        memcpy(&report_dbg, &msg1_buf, sizeof(sgx_report_t));
        // printf("Preparing attr.flag: %016lx\n", report_dbg.body.attributes.flags);
        // printf("Preparing attr.xfrm: %016lx\n", report_dbg.body.attributes.xfrm);
        bytes = send(connfd, msg1_buf, sizeof(msg1_buf), 0);
        if (bytes <= 0)
        {
            printf("No msg1 sent! Abort!\n");
            break;
        }
        else
        {
            printf("Msg1 sent\n");
        }

        memset(msg2_buf, 0x00, sizeof(msg2_buf));
        bytes = recv(connfd, msg2_buf, sizeof(msg2_buf), 0);
        if (bytes <= 0)
        {
            printf("No msg2 received. Abort!\n");
            break;
        }

        memset(&report_dbg, 0, sizeof(report_dbg));
        memcpy(&report_dbg, &msg2_buf, sizeof(sgx_report_t));
        // printf("Receiving attr.flag: %016lx\n", report_dbg.body.attributes.flags);
        // printf("Receiving attr.xfrm: %016lx\n", report_dbg.body.attributes.xfrm);
        // fflush(stdout);

        printf("Verifying msg2...\n");
        ecall_verify_msg2(global_eid, msg2_buf, sizeof(msg2_buf), msg3_buf, sizeof(msg3_buf));
        bzero(msg2_buf, sizeof(msg2_buf));
        //send msg3
        memcpy(&key_dbg, &msg3_buf, sizeof(sgx_aes_gcm_128bit_key_t));
        bytes = send(connfd, msg3_buf, sizeof(msg3_buf), 0);
        if (bytes <= 0)
        {
            printf("No msg3 sent! Abort!\n");
            break;
        }
        else
        {
            printf("Msg3 sent\n");
        }
        printf("LA session finished.\n\n");
        // stop LA
        break;

    } //while ends

    close(connfd);
    return 0;
}
