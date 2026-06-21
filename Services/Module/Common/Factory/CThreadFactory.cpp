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

CThreadFactory::CThreadFactory()
{
}

CThreadFactory::~CThreadFactory()
{
}

unique_ptr<CThread> CThreadFactory::Create(const QString& strName, std::shared_ptr<CThreadHandler> pHandle, INT32 s32Interval)
{
    if (pHandle == nullptr) {
        return nullptr;
    }
    
    return std::make_unique<CThread>(strName, pHandle, s32Interval);
}

CThread* CThreadFactory::Create()
{
    // 无参版本返回 nullptr，实际应使用带参数的 Create 方法
    return nullptr;
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
        {"AppLog Thread",       m_vecpThreadHanders[E_THREAD_LOG],          0,  QThread::NormalPriority },
        {"AppSystem Thread",    m_vecpThreadHanders[E_THREAD_SYSTEM],       0,  QThread::NormalPriority },
        {"DataSave Thread",     m_vecpThreadHanders[E_THREAD_SAVE_DATA],       0,  QThread::NormalPriority },

    };

    //    CThread *pDataSaveThread = NULL;

    for(INT32U i = 0; i < ARRAYSIZE(st_TaskArr); ++i)
    {
        CThread *pThread =  new CThread(st_TaskArr[i].m_strThreadName, \
                            st_TaskArr[i].m_pThreadHandler, \
                            st_TaskArr[i].m_s32Interval);

        if (nullptr != pThread)
        {
            m_vecpThread.push_back(unique_ptr<CThread>(pThread));
            pThread->start();
            pThread->setPriority(st_TaskArr[i].m_emPriority);
        }
    }
}

void CThreadFactory::InitThreadHanders()
{

}

void CThreadFactory::ThreadModule()
{
}
