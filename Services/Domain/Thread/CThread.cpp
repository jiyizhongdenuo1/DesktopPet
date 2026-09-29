/*
* @file: CThread.cpp
 * @brief: 线程类实现
 * @author: nuo
 * @date: 2026/6/1
 * @Detail: 封装 QThread，提供线程调度和任务处理功能
 */

#include <semaphore>
#include <chrono>
#include "CThread.h"
#include "CThreadHandler.h"
using namespace std;

class CThreadPrivate
{
public:
    friend class CThread;
    explicit CThreadPrivate(CThread *pThread, const QString &strName, std::shared_ptr<CThreadHandler> pHandle, INT32 s32Interval)
        : q_ptr(pThread)
        , m_strName(strName)
        , m_pTaskHandler(pHandle)
        , m_s32Interval(s32Interval)
        , m_bIsExit(false)
    {
    }
    ~CThreadPrivate()
    {
    }
public:
    CThread                         *q_ptr;
    QString                         m_strName;
    std::counting_semaphore<>         m_Semaphore{0};
    INT32                           m_s32Interval;
    shared_ptr<CThreadHandler>      m_pTaskHandler;
    BOOL                            m_bIsExit;
};

CThread::CThread(const QString &strName, std::shared_ptr<CThreadHandler> pHandle, INT32 s32Interval)
    : QThread()
    , d_ptr(new CThreadPrivate(this, strName, pHandle, s32Interval))
{
    setObjectName(strName);
}

CThread::~CThread()
{
    ExitThread();
}

void CThread::WakeUp(INT32 n)
{
    d_ptr->m_Semaphore.release(n);
}

void CThread::SetInterval(INT32 s32Interval)
{
    d_ptr->m_s32Interval = s32Interval;
}

void CThread::run ()
{
    if (d_ptr->m_pTaskHandler)
    {
        d_ptr->m_pTaskHandler->AttachThread(shared_from_this());
    }

    while (!d_ptr->m_bIsExit)
    {
        // 等待唤醒：定时模式用超时等待，事件驱动模式用永久阻塞
        if (d_ptr->m_s32Interval > 0)
        {
            // C++20 counting_semaphore::try_acquire_for 要求带类型的时间单位，不同于 Qt 的 QSemaphore::tryAcquire(1, int_ms)
            d_ptr->m_Semaphore.try_acquire_for(chrono::milliseconds(d_ptr->m_s32Interval));
        }
        else
        {
            d_ptr->m_Semaphore.acquire();
        }

        // 退出标志可能在等待期间被设置，需再次检查
        if (d_ptr->m_bIsExit)
        {
            break;
        }

        // 执行实际任务处理
        if (d_ptr->m_pTaskHandler)
        {
            d_ptr->m_pTaskHandler->HandleTask();
        }

        // 定时模式下任务执行完后 sleep 一个周期，避免忙轮询
        if (!d_ptr->m_bIsExit && d_ptr->m_s32Interval > 0)
        {
            msleep(d_ptr->m_s32Interval);
        }
    }
}

void CThread::ExitThread()
{
    d_ptr->m_bIsExit = true;
    WakeUp(1);
    wait();
}