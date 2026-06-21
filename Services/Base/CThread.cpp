/*
* @file: CThread.cpp
 * @brief: 线程类实现
 * @author: nuo
 * @date: 2026/6/1
 * @Detail: 封装 QThread，提供线程调度和任务处理功能
 */

#include "CThread.h"
#include <QSemaphore>
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
    QSemaphore                      m_Semaphore;
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
    Q_D(CThread);
    d->m_Semaphore.release(n);
}

void CThread::SetInterval(INT32 s32Interval)
{
    Q_D(CThread);
    d->m_s32Interval = s32Interval;
}

void CThread::run()
{
    Q_D(CThread);

    while (!d->m_bIsExit)
    {
        d->m_Semaphore.acquire();

        if (d->m_bIsExit)
        {
            break;
        }

        if (d->m_pTaskHandler)
        {
            d->m_pTaskHandler->HandleTask();
        }

        if (!d->m_bIsExit && d->m_s32Interval > 0)
        {
            msleep(d->m_s32Interval);
        }
    }
}

void CThread::ExitThread()
{
    Q_D(CThread);
    d->m_bIsExit = true;
    WakeUp(1);
    wait();
}