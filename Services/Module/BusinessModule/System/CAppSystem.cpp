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
    std::vector<unique_ptr<CThread *>>                 m_vecpThread;
    std::vector<shared_ptr<CThreadHandler>>            m_vecpThreadHanders;
    unique_ptr<CAppSystem>                             q_ptr;
};

CAppSystem::CAppSystem(QObject *parent)
    : QObject(parent)
    , d_ptr(std::make_unique<CAppSystemPrivate>())
{
    Q_D(CAppSystem);
    IniAppFrame();
}

CAppSystem::~CAppSystem()
{
}

void CAppSystem::SetThreadHandler(E_THREAD_ID eThreadId, CThreadHandler *pThreadHandler)
{
    Q_D(CAppSystem);
    if (NULL == pThreadHandler)
    {
        return;
    }
    if (NULL != d->m_vecpThreadHanders[eThreadId])
    {
        d->m_vecpThreadHanders[eThreadId].reset(pThreadHandler);
    }
    else
    {
        d->m_vecpThreadHanders[eThreadId] = std::unique_ptr<CThreadHandler>(pThreadHandler);
    }
}

void CAppSystem::IniAppFrame()
{
    CreateThread();
}

INT32 CAppSystem::DoSecEvent()
{
    return true;
}


void CAppSystem::CreateThread()
{
    Q_D(CAppSystem);

}

shared_ptr<CThreadHandler> CAppSystem::GetThreadHandler(E_THREAD_ID eThreadId)
{
    Q_D(CAppSystem);

    if (0 <= eThreadId && eThreadId < sizeof(d->m_vecpThreadHanders) / sizeof(d->m_vecpThreadHanders[0]))
    {
        return d->m_vecpThreadHanders[eThreadId];
    }
    return nullptr;
}
