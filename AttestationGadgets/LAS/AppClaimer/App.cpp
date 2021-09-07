#include "sgx_eid.h"
#include "sgx_urts.h"
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include "datatypes.h"
#include "App.h"
#include "EnclaveClaimer_u.h"

sgx_enclave_id_t global_eid = 0;

void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    printf("%s", str);
}

int comm()
{
    int fd_socket;
    struct sockaddr_un un_socket;
    char buff[BUFFSIZE];

    memset(&un_socket, 0, sizeof(un_socket));

    un_socket.sun_family = AF_UNIX;
    strcpy(un_socket.sun_path, SOCKETPATH);

    // create a fd for the socket (UNIX domain socket)
    if ((fd_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        exit(1);
    }

    int conn_result = connect(fd_socket, (struct sockaddr *)&un_socket, sizeof(un_socket));
    if (conn_result == -1)
    {
        perror("connect error");
        close(fd_socket);
        return -1;
    }

    socklen_t socket_len = sizeof(un_socket);
    int recvret;

    while (1)
    {
        // int client_fd = accept(fd_socket, (struct sockaddr *)&un_socket, &socket_len);
        printf("CLIENT: Please input: ");
        // scanf("%s", &buff);
        fgets(buff, 1024, stdin);
        send(fd_socket, buff, strlen(buff), 0);
        DEBUG("sent: %s \n", buff);
        bzero(buff, sizeof(buff));
        recvret = recv(fd_socket, buff, BUFFSIZE, 0);
        DEBUG("RECVRET: %d \n", recvret);
        printf("CLIENT: recved: %s \n", buff);
        bzero(buff, sizeof(buff));
    }
}
int main(int argc, char const *argv[])
{

    // enclave creation
    sgx_status_t status;
    sgx_launch_token_t token = {0};
    int updated = 0;

    status = sgx_create_enclave(CLAIMER_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &global_eid, NULL);
    if (status != SGX_SUCCESS)
    {
        printf("failed to load enclave %s, error code is 0x%x.\n", CLAIMER_FILENAME, status);
        return -1;
    }

    ecall_printf_string(global_eid, "hello\n");

    sgx_destroy_enclave(global_eid);

    //WL: comment for now
    // comm();

    return 0;
}
