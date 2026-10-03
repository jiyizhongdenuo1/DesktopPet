/*
 * @file: CNoteDataCollect.cpp
 * @brief: 笔记数据收集器实现
 * @author: nuo
 * @date: 2026/6/23
 * @Detail:
 */

#include "CNoteDataCollect.h"
#include "DServiceBase.h"

using namespace std;

CNoteDataCollect::CNoteDataCollect(INT32 s32BufferSize)
    : CDataCollectBase(s32BufferSize)
{
}

CNoteDataCollect::~CNoteDataCollect()
{
}

// ========== INoteDataBuffer 接口实现 ==========

void CNoteDataCollect::AppendData(const NOTE_MODEL_ITEM &stModelItem)
{
    ST_NOTE_DATA st_NoteData{};

    st_NoteData.m_s32id           = stModelItem.m_s64NoteId;
    st_NoteData.m_eNoteLevel      = stModelItem.m_eNoteLevel;
    st_NoteData.m_eTimeSpanType   = stModelItem.m_eTimeSpanType;
    st_NoteData.m_s64RemindTime   = stModelItem.m_s64RemindTime;
    st_NoteData.m_s64CreateTime     = stModelItem.m_s64WriteTime;
    st_NoteData.m_s64UpdateTime   = stModelItem.m_s64ModifyTime;
    st_NoteData.m_eRemindFrequency= stModelItem.m_eRemindFrequency;
    st_NoteData.m_eNoteType       = stModelItem.m_eNoteType;
    st_NoteData.m_bCompleted      = stModelItem.m_bCompleted;
    st_NoteData.m_bDeleted        = stModelItem.m_bDeleted;

    INT32 n_CopySize = min(
        static_cast<INT32>(stModelItem.m_strContent.size()),
        static_cast<INT32>(NoteSpace::CONTENT_LENGTH_MAX - 1)
    );

    if (n_CopySize > 0)
    {
        memcpy(st_NoteData.m_szContent,
               stModelItem.m_strContent.c_str(),
               n_CopySize + 1);
    }
    else
    {
        st_NoteData.m_szContent[0] = '\0';
    }

    // 直接写入环形缓冲，省掉中间那次拷贝
    SetBuffer(sizeof(st_NoteData), reinterpret_cast<const char *>(&st_NoteData));
}

INT32 CNoteDataCollect::ReadBuffer(INT32 s32GetSize, char *pcBuffer)
{
    return GetBuffer(s32GetSize, pcBuffer);
}

bool CNoteDataCollect::HasData() const
{
    return HasUnsavedData();
}

void CNoteDataCollect::Clear()
{
    // 丢弃未读数据；只能由消费者线程（数据线程）调用
    CDataCollectBase::DiscardAll();
}

INT32 CNoteDataCollect::GetBufferSize() const
{
    return CDataCollectBase::GetBufferSize();
}

BOOL CNoteDataCollect::DeleteNote(const NOTE_MODEL_ITEM &stModelItem)
{
    AppendData(stModelItem);
    return FALSE;
}

void CNoteDataCollect::UpDateNoteData(const NOTE_MODEL_ITEM &stModelItem)
{
    AppendData(stModelItem);
}

VOID CNoteDataCollect::UpDataNextID(INT64 s64NextID)
{
    std::lock_guard<std::mutex> lock(m_mutexNextID);
    m_s64NextID = s64NextID;
}

INT64 CNoteDataCollect::GetNextID() const
{
    std::lock_guard<std::mutex> lock(m_mutexNextID);
    return m_s64NextID;
}
