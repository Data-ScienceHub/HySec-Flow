#ifndef _DATATYPES_H
#define _DATATYPES_H

#define SOCKETPATH "/home/box/LA.socket"
#define BUFFSIZE 1024
#define SERVER_MAX_THREAD 10

typedef enum{
	FIFO_DH_REQ_MSG1,
	FIFO_DH_RESP_MSG1,
	FIFO_DH_MSG2,
	FIFO_DH_MSG3,
	FIFO_DH_MSG_REQ,
	FIFO_DH_MSG_RESP,
	FIFO_DH_CLOSE_REQ,
	FIFO_DH_CLOSE_RESP
}FIFO_MSG_TYPE;

//WL: for ocall_send
// typedef unsigned short int sa_family_t;

// struct sockaddr {
//     sa_family_t sa_family;
//     char sa_data[128 - sizeof(unsigned short)];
// };

// typedef struct {
//     int ms_sockfd;
//     const void* ms_buf;
//     size_t ms_count;
//     const struct sockaddr* ms_addr;
//     size_t ms_addrlen;
//     void* ms_control;
//     size_t ms_controllen;
// } ms_ocall_send_t;

#endif