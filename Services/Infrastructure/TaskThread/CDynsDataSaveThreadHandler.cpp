/*
* @file: CDynsDataSaveThreadHandler.cpp
 * @brief: 动态数据保存线程处理器实现
 * @author: nuo
 * @date: 2026/6/10
 * @Detail: 负责数据保存任务的处理和调度
 */
#include "CDynsDataSaveThreadHandler.h"
#include <memory>
#include <queue>
#include <map>
#include <functional>
#include <iostream>
#include <QMutex>
#include "CDataRWMgr.h"
#include "CThread.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "INoteDataBuffer.h"
#include "DSaveDefine.h"
#include "CNoteDataCache.h"
#include "CNoteDataService.h"
 #include "DDataCache.h"
#include "DThread.h"
#include "datatype.h"

using namespace std;

void CDynsDataSaveThreadHandler::FuncSaveNoteData(ST_DATA_SAVE_EVENT &event)
{
    (void)event;

    if (!d_ptr->m_pDataSaveRWMgr)
    {
        return ;
    }
    if (!d_ptr->m_pBuffer)
    {
        return;
    }

    // ★ nextId 先更新到内存 header（后面写文件时一并落盘）
    d_ptr->m_pDataSaveRWMgr->UpdateNextId(d_ptr->m_pBuffer->GetNextID());

    if (!d_ptr->m_pBuffer->HasData())
    {
        return;
    }

    INT32 s32_SaveNoteDataLimit = DSaveDefine::SINGLE_SAVE_NOTE_DATA_COUNT * sizeof(st_NoteData);
    char *pNoteData = new char[s32_SaveNoteDataLimit];
    INT32 s32_ReadSize = d_ptr->m_pBuffer->ReadBuffer(s32_SaveNoteDataLimit, pNoteData);

    if (s32_ReadSize > 0)
    {
        auto p_Cache = d_ptr->m_pCache;
        if (p_Cache)
        {
            const INT32 s32Count = s32_ReadSize / static_cast<INT32>(sizeof(ST_NOTE_DATA));
            const auto *pArrNote = reinterpret_cast<const ST_NOTE_DATA *>(pNoteData);
            p_Cache->AppendNoteData(pArrNote, s32Count);      // 整批只发布一次快照
        }

        d_ptr->m_pDataSaveRWMgr->AddOneNoteData(pNoteData, s32_ReadSize);
    }

    if (!d_ptr->m_pRecycleBuffer)
    {
        RELEASEIF(pNoteData);
        return;
    }
    s32_ReadSize = d_ptr->m_pRecycleBuffer->ReadBuffer(s32_SaveNoteDataLimit, pNoteData);
    if (s32_ReadSize > 0)
    {
        auto p_Cache = d_ptr->m_pCache;
        if (p_Cache)
        {
            const INT32 s32Count = s32_ReadSize / static_cast<INT32>(sizeof(ST_NOTE_DATA));
            const auto *pArrNote = reinterpret_cast<const ST_NOTE_DATA *>(pNoteData);
            p_Cache->AppendRecycleNoteData(pArrNote, s32Count);
        }

        d_ptr->m_pDataSaveRWMgr->AddOneNoteData(pNoteData, s32_ReadSize, TRUE);
    }

    RELEASEIF(pNoteData);
}

void CDynsDataSaveThreadHandler::FuncCompactData(ST_DATA_SAVE_EVENT &event)
{
    (void)event;

    if (!d_ptr->m_pDataSaveRWMgr)
    {
        return ;
    }
    d_ptr->m_pDataSaveRWMgr->CompactNoteFile();
}

void CDynsDataSaveThreadHandler::FuncReadNoteData(ST_DATA_SAVE_EVENT &event)
{
    if (!d_ptr->m_pDataSaveRWMgr)
    {
        return ;
    }
    auto p_DataMgr = d_ptr->m_pDataSaveRWMgr;
    auto p_Cache = d_ptr->m_pCache;
    auto p_NoteDataService = d_ptr->m_pNoteDataService;
    if (!p_DataMgr || !p_Cache || !p_NoteDataService)
    {
        return;
    }
    INT32 s32_CacheSize = DDataCache::MAX_CACHE_SIZE * static_cast<INT32>(sizeof(ST_NOTE_DATA));
    auto it_Param = event.mapParams.find(DataSaveFucName::READ_NOTE_DATA_SIZE);
    if (it_Param != event.mapParams.end())
    {
        INT32 s32_ReadLimit = stoi(it_Param->second) * static_cast<INT32>(sizeof(ST_NOTE_DATA));
        s32_CacheSize = min(s32_CacheSize, s32_ReadLimit);
    }
    char *pNoteData = new char[s32_CacheSize];
    INT32 s32_ReadSize = 0;
    p_DataMgr->ReadFromFile(pNoteData, s32_CacheSize, s32_ReadSize);
    if (s32_ReadSize > 0)
    {
        p_NoteDataService->LoadFromBuffer(pNoteData, s32_ReadSize);
    }

    memset(pNoteData, 0, s32_ReadSize);
    s32_ReadSize  = 0;
    p_DataMgr->ReadFromFile(pNoteData, s32_CacheSize, s32_ReadSize, TRUE);
    if (s32_ReadSize > 0)
    {
        p_NoteDataService->LoadRecycleFromBuffer(pNoteData, s32_ReadSize);
    }

    delete[] pNoteData;
}

class CDynsDataSaveThreadHandlerPrivate
{
    friend class CDynsDataSaveThreadHandler;

public:
    explicit CDynsDataSaveThreadHandlerPrivate(CDynsDataSaveThreadHandler *q,
                                               shared_ptr<CDataRWMgr> pDataRWMgr,
                                               shared_ptr<INoteDataBuffer> pBuffer,
                                               shared_ptr<INoteDataBuffer> pRecycleBuffer,
                                               shared_ptr<CNoteDataCache> pCache,
                                               shared_ptr<CNoteDataService> pService)
        : q_ptr(q)
        , m_pDataSaveRWMgr(std::move(pDataRWMgr))
        , m_pBuffer(std::move(pBuffer))
        , m_pRecycleBuffer(std::move(pRecycleBuffer))
        , m_pCache(std::move(pCache))
        , m_pNoteDataService(std::move(pService))
        , m_bIsExit(false)
    {

    }

private:
    CDynsDataSaveThreadHandler              *q_ptr;
    shared_ptr<CDataRWMgr>                  m_pDataSaveRWMgr;
    shared_ptr<INoteDataBuffer>             m_pBuffer;
    shared_ptr<INoteDataBuffer>             m_pRecycleBuffer;
    shared_ptr<CNoteDataCache>              m_pCache;
    shared_ptr<CNoteDataService>            m_pNoteDataService;
    queue<ST_DATA_SAVE_EVENT>               m_queSaveEvent;
    QMutex                                  m_mutex;
    map<string, std::function<void(ST_DATA_SAVE_EVENT &)>> m_mapFunc;
    BOOL                                    m_bIsExit;
};

CDynsDataSaveThreadHandler::CDynsDataSaveThreadHandler(shared_ptr<CDataRWMgr> pDataRWMgr,
                                                       shared_ptr<INoteDataBuffer> pBuffer,
                                                       shared_ptr<INoteDataBuffer> pRecycleBuffer,
                                                       shared_ptr<CNoteDataCache> pCache,
                                                       shared_ptr<CNoteDataService> pService)
    : d_ptr(new CDynsDataSaveThreadHandlerPrivate(this, std::move(pDataRWMgr),
                                                  std::move(pBuffer), std::move(pRecycleBuffer),
                                                  std::move(pCache), std::move(pService)))
{
    d_ptr->m_mapFunc[DataSaveFucName::MSG_DATASAVE_NOTE]    = [this](ST_DATA_SAVE_EVENT &e) { FuncSaveNoteData(e); };
    d_ptr->m_mapFunc[DataSaveFucName::MSG_DATAREAD_NOTE]    = [this](ST_DATA_SAVE_EVENT &e) { FuncReadNoteData(e); };
    d_ptr->m_mapFunc[DataSaveFucName::MSG_COMPACT_FILE]     = [this](ST_DATA_SAVE_EVENT &e) { FuncCompactData(e); };
}

CDynsDataSaveThreadHandler::~CDynsDataSaveThreadHandler()
{
    // 停止线程并等待结束
    d_ptr->m_bIsExit = true;

    // 清空任务队列
    d_ptr->m_mutex.lock();
    while (!d_ptr->m_queSaveEvent.empty())
    {
        d_ptr->m_queSaveEvent.pop();
    }
    d_ptr->m_mutex.unlock();

    // 如果有正在运行的线程，唤醒它以退出
    auto pThread = m_pThread.lock();
    if (pThread)
    {
        pThread->WakeUp(1);
        pThread->wait();
    }

    // 释放数据保存对象
    d_ptr->m_pDataSaveRWMgr.reset();
}

void CDynsDataSaveThreadHandler::SaveAllData()
{
    if (!d_ptr->m_pDataSaveRWMgr)
    {
        return;
    }

    // 处理所有待保存的数据
    d_ptr->m_mutex.lock();
    while (!d_ptr->m_queSaveEvent.empty())
    {
        ST_DATA_SAVE_EVENT event = d_ptr->m_queSaveEvent.front();
        d_ptr->m_queSaveEvent.pop();

        auto it = d_ptr->m_mapFunc.find(event.strMsgKey);
        if (it != d_ptr->m_mapFunc.end())
        {
            it->second(event);
        }
    }
    d_ptr->m_mutex.unlock();
}

void CDynsDataSaveThreadHandler::HandleTask()
{
    if (!d_ptr->m_pDataSaveRWMgr)
    {
        return;
    }

    while (true)
    {
        d_ptr->m_mutex.lock();
        if (d_ptr->m_queSaveEvent.empty())
        {
            d_ptr->m_mutex.unlock();
            return;
        }
        ST_DATA_SAVE_EVENT event = d_ptr->m_queSaveEvent.front();
        d_ptr->m_queSaveEvent.pop();
        d_ptr->m_mutex.unlock();

        auto it = d_ptr->m_mapFunc.find(event.strMsgKey);
        if (it != d_ptr->m_mapFunc.end())
        {
            it->second(event);
            #ifdef _WIN32
            Sleep(1000);
#else
            sleep(1);
#endif
        }
    }
}

void CDynsDataSaveThreadHandler::AddTask(const string &strKey, const unordered_map<string, string> &mapParams)
{
    static constexpr INT32 s32_MaxLimit = 1000;
    ST_DATA_SAVE_EVENT tmpEvent;
    if (!strKey.empty())
    {
        tmpEvent.strMsgKey = strKey;
        if (!mapParams.empty())
        {
            tmpEvent.mapParams = mapParams;
        }
        d_ptr->m_mutex.lock();
        if (d_ptr->m_queSaveEvent.size() < s32_MaxLimit)
        {
            d_ptr->m_queSaveEvent.push(tmpEvent);
        }
        d_ptr->m_mutex.unlock();
    }

    auto pThread = m_pThread.lock();
    if (pThread)
    {
        pThread->WakeUp(1);
    }
}
