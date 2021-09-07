#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include "sgx_eid.h"
#include "sgx_urts.h"
#include "CLATask.h"
#include "Thread.h"
#include "datatypes.h"
#include "App.h"
#include "EnclaveVerifier_u.h"

// enclave id
sgx_enclave_id_t global_eid = 0;

//test ocall
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    printf("%s", str);
}

int main(int argc, char const *argv[])
{
    // enclave creation
    /* Function Description: load responder enclave */
    sgx_status_t status;
    sgx_launch_token_t token = {0};
    int updated = 0;

    status = sgx_create_enclave(VERIFIER_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &global_eid, NULL);
    if (status != SGX_SUCCESS)
    {
        printf("failed to load enclave %s, error code is 0x%x.\n", VERIFIER_FILENAME, status);
        return -1;
    }

    char buff[BUFFSIZE];
    // init LA unix domain socket
    int la_socket_fd;
    struct sockaddr_un la_server_addr, la_client_addr;
    memset(&la_server_addr, 0, sizeof(la_server_addr));

    la_server_addr.sun_family = AF_UNIX;
    strcpy(la_server_addr.sun_path, SOCKETPATH);

    // create a fd for the socket (UNIX domain socket)
    if ((la_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        exit(1);
    }

    if (bind(la_socket_fd, (struct sockaddr *)&la_server_addr, sizeof(la_server_addr)) == -1)
    {
        perror("bind error");
        close(la_socket_fd);
        return -1;
    }

    if (listen(la_socket_fd, SERVER_MAX_THREAD) == -1)
    {
        perror("listen error");
        close(la_socket_fd);
        return -1;
    }

    // create a thread pool 
    CThreadPool LAPool(10);

    while (1)
    {
        socklen_t socket_len = sizeof(la_client_addr);
        int la_client_fd = accept(la_socket_fd, (struct sockaddr *)&la_client_addr, &socket_len);
        if (la_client_fd < 0)
        {
            perror("accept failed");
        }
        else
        {   
            // add a task when accept an valid client
            CTask *la_task = new CLATask;
            la_task->SetConnFd(la_client_fd);
            printf("%d is set as a la_client_fd\n", la_client_fd);
            LAPool.AddTask(la_task);
        }
    }

    DEBUG("exit normally");
    close(la_socket_fd);
    unlink(SOCKETPATH);

    return 0;
}