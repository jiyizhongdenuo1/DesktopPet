/*
 * @file: CNoteDataService.cpp
 * @brief: 笔记数据服务层实现
 * @author: nuo
 * @date: 2026/7/24
 * @Detail:
 */

#include "CNoteDataService.h"
#include "CNoteDataCache.h"
#include "INoteDataBuffer.h"
#include "DDataMgrBase.h"
#include "DServiceBase.h"

using namespace std;

CNoteDataService::CNoteDataService(std::shared_ptr<CNoteDataCache> pCache,
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

int CNoteDataService::LoadFromBuffer(char *pBuffer, INT32 s32BufferSize)
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

int CNoteDataService::LoadRecycleFromBuffer(char *pBuffer, INT32 s32BufferSize)
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
        ConvertUIToDomain(stModelItem, stNoteData);
        m_pCache->UpdateNoteDataCache(stNoteData);
    }
    return TRUE;
}

VOID CNoteDataService::UpDataNextID(INT64 s64NextID)
{
    if (m_pBuffer)
    {
        m_pBuffer->UpDataNextID(s64NextID);
    }
}

void CNoteDataService::ConvertUIToDomain(const NOTE_MODEL_ITEM &stModelItem, ST_NOTE_DATA &stNoteData)
{
    stNoteData = {};

    stNoteData.m_s32id           = stModelItem.m_s64NoteId;
    stNoteData.m_eNoteLevel      = stModelItem.m_eNoteLevel;
    stNoteData.m_eTimeSpanType   = stModelItem.m_eTimeSpanType;
    stNoteData.m_s64RemindTime   = stModelItem.m_s64RemindTime;
    stNoteData.m_s64CreateTime   = stModelItem.m_s64WriteTime;
    stNoteData.m_s64UpdateTime   = stModelItem.m_s64ModifyTime;
    stNoteData.m_eRemindFrequency= stModelItem.m_eRemindFrequency;
    stNoteData.m_eNoteType        = stModelItem.m_eNoteType;
    stNoteData.m_bCompleted      = stModelItem.m_bCompleted;
    stNoteData.m_bDeleted        = stModelItem.m_bDeleted;

    INT32 n_CopySize = min(
        static_cast<INT32>(stModelItem.m_strContent.size()),
        static_cast<INT32>(NoteSpace::CONTENT_LENGTH_MAX - 1)
    );

    if (n_CopySize > 0)
    {
        memcpy(stNoteData.m_szContent,
               stModelItem.m_strContent.c_str(),
               n_CopySize + 1);
    }
    else
    {
        stNoteData.m_szContent[0] = '\0';
    }
}

void CNoteDataService::NotifyDataLoaded()
{
    if (m_pCache && m_NoteDataCallback)
    {
        m_NoteDataCallback(m_pCache->GetSnapshot());
    }
}
