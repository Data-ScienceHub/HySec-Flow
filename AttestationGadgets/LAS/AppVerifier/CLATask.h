#ifndef _CLATASK_H
#define _CLATASK_H

#include "Thread.h"
#include "CTask.h"

class CLATask : public CTask
{
protected:
    string m_strTaskName; //任务的名称
    int connfd;           //接收的地址

public:
    // CLATask() = default;
    CLATask() {}
    ~CLATask() {}
    CLATask(string &taskName) : m_strTaskName(taskName), connfd(NULL) {}
    
    // error: invalid new-expression of abstract class type ‘CLATask’
    // virtual int Run() = 0;

    virtual int Run();
    void SetConnFd(int data); //设置接收的套接字连接号。

    // error: could not find variable specification at offset
    // /home/ya0guang/Code/AttestationGadgets/LA/AppVerifier/CLATask.cpp:13: undefined reference to `CLATask::GetConnFd()'
    // int GetConnFd();
};


#endif