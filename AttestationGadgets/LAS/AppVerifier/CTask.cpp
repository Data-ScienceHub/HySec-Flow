// #include "Thread.h"
#include "CTask.h"
#include <iostream>
// #include <stdio.h>
// #include <deque>

void CTask::SetConnFd(int data)
{
    connfd = data;
}

int CTask::GetConnFd()
{
    return connfd;
}