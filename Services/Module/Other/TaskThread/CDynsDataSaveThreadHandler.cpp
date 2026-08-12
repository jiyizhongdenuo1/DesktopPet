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
#include <iostream>
#include <QMutex>
#include "CDataRWMgr.h"
#include "CThread.h"
#include "CServiceLocator.h"
#include "INoteDataBuffer.h"
#include "DSaveDefine.h"
#include "CNoteDataCache.h"
#include "CNoteDataService.h"
 #include "DDataCache.h"
#include "DThread.h"

using namespace std;

static void func_SaveNoteData(shared_ptr<CDataRWMgr> pDataSaveRWMgr, ST_DATA_SAVE_EVENT &event)
{
    auto p_Buffer = g_ServiceLocator.GetNoteCollect();
    if (!p_Buffer || !p_Buffer->HasData())
    {
        return;
    }

    INT32 s32_SaveNoteDataLimit = DSaveDefine::SINGLE_SAVE_NOTE_DATA_COUNT * sizeof(st_NoteData);
    char *pNoteData = new char[s32_SaveNoteDataLimit];
    INT32 s32_ReadSize = p_Buffer->ReadBuffer(s32_SaveNoteDataLimit, pNoteData);

    if (s32_ReadSize > 0)
    {
        pDataSaveRWMgr->WriteToFile(pNoteData, s32_ReadSize);
    }

    RELEASEIF(pNoteData);
}

static void func_ReadNoteData(shared_ptr<CDataRWMgr> pDataSaveRWMgr, ST_DATA_SAVE_EVENT &event)
{
    auto p_DataMgr = g_ServiceLocator.GetDataRWMgr();
    auto p_Cache = g_ServiceLocator.GetNoteCache();
    auto p_NoteDataService = g_ServiceLocator.GetNoteService();
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
    delete[] pNoteData;
}

class CDynsDataSaveThreadHandlerPrivate
{
    friend class CDynsDataSaveThreadHandler;

public:
    explicit CDynsDataSaveThreadHandlerPrivate(CDynsDataSaveThreadHandler *q)
        : q_ptr(q)
        , m_pDataSaveRWMgr(g_ServiceLocator.GetDataRWMgr())
        , m_bIsExit(false)
    {

    }

private:
    typedef void (*DATASAVE_FUNC)(shared_ptr<CDataRWMgr> pDataSaveRWMgr, ST_DATA_SAVE_EVENT &event);

    CDynsDataSaveThreadHandler              *q_ptr;
    shared_ptr<CDataRWMgr>                  m_pDataSaveRWMgr;
    queue<ST_DATA_SAVE_EVENT>               m_queSaveEvent;
    QMutex                                  m_mutex;
    map<string, DATASAVE_FUNC>              m_mapFunc;
    BOOL                                    m_bIsExit;
};

CDynsDataSaveThreadHandler::CDynsDataSaveThreadHandler()
    : d_ptr(new CDynsDataSaveThreadHandlerPrivate(this))
{
    d_ptr->m_mapFunc[DataSaveFucName::MSG_DATASAVE_NOTE] = func_SaveNoteData;
    d_ptr->m_mapFunc[DataSaveFucName::MSG_DATAREAD_NOTE] = func_ReadNoteData;

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
            it->second(d_ptr->m_pDataSaveRWMgr, event);
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
            it->second(d_ptr->m_pDataSaveRWMgr, event);
            sleep(1);
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
        pThread->WakeUp(s32_MaxLimit);
    }
}