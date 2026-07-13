/*
 * @file: DDataMgrBase.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/24
 * @Detail:
 */

#pragma once

#include  <QString>
#include  "datatype.h"
#include  <DServiceBase.h>

typedef  struct st_NoteModelItem
{
    INT64       m_s64NoteId;
    E_NOTE_EVENT_WAKEUP_LEVEL m_eNoteLevel;
    INT64       m_s64RemindTime;
    INT64       m_s64WriteTime;
    INT64       m_s64ModifyTime;
    INT32       m_s32RemindLevel;
    E_NOTE_EVENT_TYPE m_eEventType;
    BOOL        m_bCompleted;
    BOOL        m_bDeleted;
    QString     m_strContent;
    st_NoteModelItem()
        : m_s64NoteId(0), m_eNoteLevel(E_NOTE_EVENT_WAKEUP_LEVEL_NORMAL), m_s64RemindTime(0)
        , m_s64WriteTime(0), m_s64ModifyTime(0), m_s32RemindLevel(0)
        , m_eEventType(E_NOTE_EVENT_NONE), m_bCompleted(FALSE), m_bDeleted(FALSE) {}
}NOTE_MODEL_ITEM;

namespace NDataManager
{
    constexpr INT32 NOTE_BUFFER_ITEM_COUNT_MAX = 128;

}