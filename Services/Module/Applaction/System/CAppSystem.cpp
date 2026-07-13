/*
 * @file: CAppSystem.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/4
 * @Detail:
 */

#include "CAppSystem.h"
#include "GlobalEnums.h"
#include "CThread.h"
#include "CThreadHandler.h"
#include "CDynsDataSaveThreadHandler.h"
#include "CSystemThreadHandler.h"
#include "CThreadFactory.h"

using namespace std;
CAppSystem *CAppSystem::m_pInstance = nullptr;
CAppSystem *CAppSystem::GetInstance()
{
    if (!m_pInstance)
    {
        CAppSystem::m_pInstance  = new CAppSystem();
    }
    return m_pInstance;
}

class CAppSystemPrivate
{
    friend class CAppSystem;
public:
    explicit CAppSystemPrivate()
    {
        m_vecpThread.reserve(E_THREAD_MAX);
        m_vecpThreadHanders.reserve(E_THREAD_MAX);
    }

private:
    vector<unique_ptr<CThread>>                 m_vecpThread;
    vector<shared_ptr<CThreadHandler>>          m_vecpThreadHanders;
    unique_ptr<CAppSystem>                      q_ptr;
};

CAppSystem::CAppSystem(QObject *parent)
    : QObject(parent)
    , d_ptr(make_unique<CAppSystemPrivate>())
{
    IniAppFrame();
}

CAppSystem::~CAppSystem()
{
}

void CAppSystem::SetThreadHandler(E_THREAD_ID eThreadId, CThreadHandler *pThreadHandler)
{
    if (NULL == pThreadHandler)
    {
        return;
    }
    if (NULL != d_ptr->m_vecpThreadHanders[eThreadId])
    {
        d_ptr->m_vecpThreadHanders[eThreadId].reset(pThreadHandler);
    }
    else
    {
        d_ptr->m_vecpThreadHanders[eThreadId] = unique_ptr<CThreadHandler>(pThreadHandler);
    }
}

void CAppSystem::AddSaveDataTask()
{
    shared_ptr<CDynsDataSaveThreadHandler> pThreadHandler = dynamic_pointer_cast<CDynsDataSaveThreadHandler>(d_ptr->m_vecpThreadHanders[E_THREAD_SAVE_DATA]);
    if (pThreadHandler)
    {
        pThreadHandler->AddTask(DataSaveFucName::MSG_DATASAVE_NOTE, QVariantHash());
    }

}

void CAppSystem::SaveDataSeconed()
{
}

void CAppSystem::IniAppFrame()
{
    CreateThread();
    auto p_SystemThreadHandler = dynamic_pointer_cast<CSystemThreadHandler>(d_ptr->m_vecpThreadHanders[E_THREAD_SYSTEM]);
    if (p_SystemThreadHandler)
    {
        p_SystemThreadHandler->SetSystemThreadFunc(bind(&CAppSystem::DoSecEvent, this) );
    }
    StartThread();
}

INT32 CAppSystem::DoSecEvent()
{
    AddSaveDataTask();
    return true;
}


void CAppSystem::CreateThread()
{
    CThreadFactory factory;
    
    d_ptr->m_vecpThread = factory.ReleaseThreads();
    d_ptr->m_vecpThreadHanders = factory.ReleaseHandlers();
}

shared_ptr<CThreadHandler> CAppSystem::GetThreadHandler(E_THREAD_ID eThreadId)
{
    if (0 <= eThreadId && eThreadId < sizeof(d_ptr->m_vecpThreadHanders) / sizeof(d_ptr->m_vecpThreadHanders[0]))
    {
        return d_ptr->m_vecpThreadHanders[eThreadId];
    }
    return nullptr;
}

void CAppSystem::StartThread()
{
    for (auto &pThread : d_ptr->m_vecpThread)
    {
        pThread->start();
    }
}
