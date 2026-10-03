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
#include "DThread.h"
#include "DDataCache.h"
#include "CNoteDataCache.h"
#include "CDataRWMgr.h"
#include "INoteDataBuffer.h"
#include "CNoteDataService.h"
#include "CNoteApp.h"
#include "DNoteCommon.h"

using namespace std;

class CAppSystemPrivate
{
    friend class CAppSystem;
public:
    explicit CAppSystemPrivate(shared_ptr<CDataRWMgr> pDataRWMgr,
                               shared_ptr<INoteDataBuffer> pBuffer,
                               shared_ptr<INoteDataBuffer> pRecycleBuffer,
                               shared_ptr<CNoteDataCache> pCache,
                               shared_ptr<CNoteDataService> pService)
            : m_timeCompactStore(0)
            , m_pDataRWMgr(std::move(pDataRWMgr))
            , m_pBuffer(std::move(pBuffer))
            , m_pRecycleBuffer(std::move(pRecycleBuffer))
            , m_pCache(std::move(pCache))
            , m_pService(std::move(pService))
    {
        m_vecpThread.reserve(E_THREAD_MAX);
        m_vecpThreadHanders.reserve(E_THREAD_MAX);
    }

private:
    vector<shared_ptr<CThread>>                 m_vecpThread;
    vector<shared_ptr<CThreadHandler>>          m_vecpThreadHanders;
    unique_ptr<CNoteApp>                        m_pNoteApp;
    time_t                                      m_timeCompactStore;
    shared_ptr<CDataRWMgr>                      m_pDataRWMgr;
    shared_ptr<INoteDataBuffer>                 m_pBuffer;
    shared_ptr<INoteDataBuffer>                 m_pRecycleBuffer;
    shared_ptr<CNoteDataCache>                  m_pCache;
    shared_ptr<CNoteDataService>                m_pService;
};

CAppSystem::CAppSystem(shared_ptr<CDataRWMgr> pDataRWMgr,
                       shared_ptr<INoteDataBuffer> pBuffer,
                       shared_ptr<INoteDataBuffer> pRecycleBuffer,
                       shared_ptr<CNoteDataCache> pCache,
                       shared_ptr<CNoteDataService> pService)
    : d_ptr(make_unique<CAppSystemPrivate>(std::move(pDataRWMgr), std::move(pBuffer),
                                           std::move(pRecycleBuffer), std::move(pCache),
                                           std::move(pService)))
{
    IniAppFrame();
}

CAppSystem::~CAppSystem()
{
    StopThread();
}

void CAppSystem::SetThreadHandler(E_THREAD_ID eThreadId, CThreadHandler *pThreadHandler)
{
    if (NULL == pThreadHandler || eThreadId < 0 || eThreadId >= E_THREAD_MAX)
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
    shared_ptr<CDynsDataSaveThreadHandler> pThreadHandler = dynamic_pointer_cast<CDynsDataSaveThreadHandler>(d_ptr->m_vecpThreadHanders[E_THREAD_DYNC_DATA]);
    if (pThreadHandler)
    {
        pThreadHandler->AddTask(DataSaveFucName::MSG_DATASAVE_NOTE, {});

        const time_t time_Now= time(nullptr);
        if (time_Now - d_ptr->m_timeCompactStore >= CSpaceTime::COMPACT_TIME_SPACE)
        {
            pThreadHandler->AddTask(DataSaveFucName::MSG_COMPACT_FILE, {});
        }
    }

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
    CreateModule();
    InitSystem();
}

INT32 CAppSystem::DoSecEvent()
{
    AddSaveDataTask();
    return true;
}


void CAppSystem::CreateThread()
{
    CThreadFactory factory(d_ptr->m_pDataRWMgr, d_ptr->m_pBuffer,
                           d_ptr->m_pRecycleBuffer, d_ptr->m_pCache, d_ptr->m_pService);

    d_ptr->m_vecpThreadHanders = factory.ReleaseHandlers();
    d_ptr->m_vecpThread = factory.ReleaseThreads();
}

shared_ptr<CThreadHandler> CAppSystem::GetThreadHandler(E_THREAD_ID eThreadId)
{
    if (0 <= eThreadId && eThreadId < d_ptr->m_vecpThreadHanders.size())
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

void CAppSystem::StopThread()
{
    d_ptr->m_vecpThread.clear();
    d_ptr->m_vecpThreadHanders.clear();
}

void CAppSystem::CreateModule()
{
    if (!d_ptr->m_pNoteApp)
    {
        d_ptr->m_pNoteApp = make_unique<CNoteApp>();
    }
}

void CAppSystem::InitSystem()
{
    auto p_ThreadHander = dynamic_pointer_cast<CDynsDataSaveThreadHandler>(d_ptr->m_vecpThreadHanders[E_THREAD_DYNC_DATA]);
    if (p_ThreadHander)
    {
        p_ThreadHander->AddTask(DataSaveFucName::MSG_DATAREAD_NOTE, {{DataSaveFucName::READ_NOTE_DATA_SIZE, to_string(DDataCache::MAX_CACHE_SIZE)}});
    }
}

void CAppSystem::SetCache(std::shared_ptr<IDataCache> pCache)
{
    if (d_ptr->m_pNoteApp)
    {
        d_ptr->m_pNoteApp->SetCache(pCache);
    }
}
