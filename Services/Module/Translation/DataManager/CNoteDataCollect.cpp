/*
 * @file: CNoteDataCollect.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/23
 * @Detail:
 */
#include <queue>
#include <mutex>
#include <list>

#include "CNoteDataCollect.h"
#include "DServiceBase.h"

using namespace std;

class CNoteDataColloctPrivate
{
    friend class CNoteDataCollect;
public:
    CNoteDataColloctPrivate() = default;
    ~CNoteDataColloctPrivate() = default;
    void TransformNoteDataToModelItem(const NOTE_MODEL_ITEM &noteData, ST_NOTE_DATA &modelItem);

private:
    queue<NOTE_MODEL_ITEM> m_queueUnSaveNote;      // 存储容器（需要加锁）
    mutex m_queueUnSaveNoteMutex;
};

CNoteDataCollect::CNoteDataCollect(INT32 s32BufferSize)
    : CDataCollectBase(s32BufferSize)
    , d_ptr(make_unique<CNoteDataColloctPrivate>())
{
}

CNoteDataCollect::~CNoteDataCollect()
{
}

void CNoteDataCollect::AddNoteData(const NOTE_MODEL_ITEM &noteData)
{
    ST_NOTE_DATA st_NoteDataCollect;
    d_ptr->TransformNoteDataToModelItem(noteData, st_NoteDataCollect);
    char *pNoteData = new char[sizeof(st_NoteDataCollect)];
    memcpy(pNoteData, &st_NoteDataCollect, sizeof(st_NoteDataCollect));
    SetBuffer(sizeof(st_NoteDataCollect), pNoteData);
    delete[] pNoteData;
}

void CNoteDataCollect::ClearUnSaveNoteData()
{
    std::queue<NOTE_MODEL_ITEM> temp;
    {
        lock_guard<mutex> lock(d_ptr->m_queueUnSaveNoteMutex);
        d_ptr->m_queueUnSaveNote.swap(temp);
    }
}

BOOL CNoteDataCollect::TickEvent()
{

    return TRUE;
}

void CNoteDataColloctPrivate::TransformNoteDataToModelItem(const NOTE_MODEL_ITEM &noteData, ST_NOTE_DATA &modelItem)
{
    modelItem.m_s32id           = noteData.m_s64NoteId;
    modelItem.m_eNoteLevel      = noteData.m_eNoteLevel;
    modelItem.m_s64RemindTime   = noteData.m_s64RemindTime;
    modelItem.m_s64NoteTime     = noteData.m_s64WriteTime;
    modelItem.m_s64UpdateTime   = noteData.m_s64ModifyTime;
    modelItem.m_s32RemindLevel  = noteData.m_s32RemindLevel;
    modelItem.m_eEventType      = noteData.m_eEventType;
    modelItem.m_bCompleted      = noteData.m_bCompleted;
    modelItem.m_bDeleted        = noteData.m_bDeleted;
    INT32 s32_CopyDataSize = min(sizeof(NOTE_MODEL_ITEM), static_cast<size_t>(NoteSpace::CONTENT_LENGTH_MAX));
    memcpy(modelItem.m_szContent, noteData.m_strContent.toStdString().c_str(), s32_CopyDataSize);
}