/*
 * @file: CThreadFactory.cpp
 * @brief: 线程工厂实现
 * @author: nuo
 * @date: 2026/6/8
 * @Detail: 负责创建和管理线程对象
 */

#include "CThreadFactory.h"
#include "CLogThreadHandler.h"
#include "CSystemThreadHandler.h"
#include "CDynsDataSaveThreadHandler.h"

using namespace std;
CThreadFactory::CThreadFactory()
{
    InitInstances();
}

CThreadFactory::~CThreadFactory()
{
}

unique_ptr<CThread> CThreadFactory::Create(const QString& strName, std::shared_ptr<CThreadHandler> pHandle, INT32 s32Interval)
{
    if (pHandle == nullptr)
    {
        return nullptr;
    }
    
    return std::make_unique<CThread>(strName, pHandle, s32Interval);
}

void CThreadFactory::InitInstances()
{
    InitThreadHanders();
    InitThread();
}

void CThreadFactory::InitThread()
{
    struct
    {
        QString                             m_strThreadName;
        std::shared_ptr<CThreadHandler>     m_pThreadHandler;
        INT32                               m_s32Interval;
        QThread::Priority                   m_emPriority;

    } st_TaskArr[] =
    {
        {"AppLog Thread",       m_vecpThreadHanders[E_THREAD_LOG],          1000,  QThread::NormalPriority },
        {"AppSystem Thread",    m_vecpThreadHanders[E_THREAD_SYSTEM],       1000,  QThread::NormalPriority },
        {"DataSave Thread",     m_vecpThreadHanders[E_THREAD_DYNC_DATA],       1000,  QThread::NormalPriority },
    };

    for(INT32U i = 0; i < ARRAYSIZE(st_TaskArr); ++i)
    {
        auto pThread = std::make_shared<CThread>(st_TaskArr[i].m_strThreadName,
                                                 st_TaskArr[i].m_pThreadHandler,
                                                 st_TaskArr[i].m_s32Interval);

        if (pThread != nullptr)
        {
            pThread->start();
            pThread->setPriority(st_TaskArr[i].m_emPriority);
            m_vecpThread.push_back(std::move(pThread));
        }
    }

}

void CThreadFactory::InitThreadHanders()
{
    m_vecpThreadHanders.resize(E_THREAD_MAX);
    m_vecpThreadHanders[E_THREAD_LOG] = std::make_shared<CLogThreadHandler>();
    m_vecpThreadHanders[E_THREAD_SYSTEM] = std::make_shared<CSystemThreadHandler>();
    m_vecpThreadHanders[E_THREAD_DYNC_DATA] = std::make_shared<CDynsDataSaveThreadHandler>();
}

void CThreadFactory::ThreadModule()
{

}