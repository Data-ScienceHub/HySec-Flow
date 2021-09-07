#ifndef __CTASK_H
#define __CTASK_H

#include <string>
#include <pthread.h>

using namespace std;

class CTask
{
protected:
    string m_strTaskName; //任务的名称
    int connfd;           //接收的地址

public:
    CTask() = default;
    CTask(string &taskName) : m_strTaskName(taskName), connfd(NULL) {}
    virtual int Run() = 0;
    void SetConnFd(int data); //设置接收的套接字连接号。
    int GetConnFd();
    virtual ~CTask() {}
};


#endif