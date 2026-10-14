/*
 * @file: CNoteDataService.cpp
 * @brief: 笔记数据服务层实现
 * @author: nuo
 * @date: 2026/7/24
 * @Detail:
 */

#include "CNoteDataService.h"
#include "INoteCache.h"
#include "INoteDataBuffer.h"
#include "DDataMgrBase.h"
#include "DServiceBase.h"
#include "DNoteMapping.h"

using namespace std;

CNoteDataService::CNoteDataService(std::shared_ptr<INoteCache> pCache,
                                   std::shared_ptr<INoteDataBuffer> pBuffer,
                                   std::shared_ptr<INoteDataBuffer> pRecycleBuffer)
    : m_pCache(std::move(pCache))
    , m_pBuffer(std::move(pBuffer))
    , m_pRecycleBuffer(std::move(pRecycleBuffer))
{
}

CNoteDataService::~CNoteDataService() = default;

void CNoteDataService::AddNote(const NOTE_MODEL_ITEM &stModelItem)
{
    if (m_pBuffer)
    {
        m_pBuffer->AppendData(stModelItem);
    }
}

void CNoteDataService::InvalidateCache()
{
    if (m_pCache)
    {
        m_pCache->InvalidateCache();
    }
}

INT32 CNoteDataService::LoadFromBuffer(char *pBuffer, INT32 s32BufferSize)
{
    if (m_pCache)
    {
        INT32 s32_PutCount = m_pCache->PutBuffer2CacheData(pBuffer, s32BufferSize);
        if (s32_PutCount > 0)
        {
            NotifyDataLoaded();
        }
        return s32_PutCount;
    }
    return 0;
}

INT32 CNoteDataService::LoadRecycleFromBuffer(char *pBuffer, INT32 s32BufferSize)
{
    if (m_pCache)
    {
        return m_pCache->PutBuffer2RecycleData(pBuffer, s32BufferSize);
    }
    return 0;
}

VOID CNoteDataService::RegisterNoteModelDataLoadCallback(CALLBACK_NOTEDATALOAD NoteDataCallback)
{
    m_NoteDataCallback = NoteDataCallback;
}

VOID CNoteDataService::RegisterNoteChangeCallback(CACHE_CHANGE_CALLBACK ChangeCallback)
{
    if (m_pCache)
    {
        m_pCache->SetChangeCallback(std::move(ChangeCallback));
    }
}

BOOL CNoteDataService::DeleteNote(const NOTE_MODEL_ITEM &stModelItem)
{
    if (!m_pRecycleBuffer)
    {
        return FALSE;
    }
    m_pRecycleBuffer->AppendData(stModelItem);

    if (m_pCache)
    {
        ST_NOTE_DATA stNoteData;
        NoteMapping::ToDomain(stModelItem, stNoteData);
        m_pCache->UpdateNoteDataCache(stNoteData);
    }
    return TRUE;
}

VOID CNoteDataService::UpdateNote(const NOTE_MODEL_ITEM &stModelItem)
{
    if (m_pBuffer)
    {
        m_pBuffer->AppendData(stModelItem);
    }

    if (m_pCache)
    {
        ST_NOTE_DATA stNoteData;
        NoteMapping::ToDomain(stModelItem, stNoteData);
        m_pCache->UpdateNoteDataCache(stNoteData);
    }
}

VOID CNoteDataService::UpDataNextID(INT64 s64NextID)
{
    if (m_pBuffer)
    {
        m_pBuffer->UpDataNextID(s64NextID);
    }
}

void CNoteDataService::NotifyDataLoaded()
{
    if (m_pCache && m_NoteDataCallback)
    {
        m_NoteDataCallback(m_pCache->GetSnapshot());
    }
}
