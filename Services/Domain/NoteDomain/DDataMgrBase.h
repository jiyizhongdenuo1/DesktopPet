/*
 * @file: DDataMgrBase.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/24
 * @Detail:
 */

#pragma once

#include <string>
#include <memory>
#include "datatype.h"
#include <DServiceBase.h>

typedef  struct st_NoteModelItem
{
    INT64                           m_s64NoteId;
    E_NOTE_EVENT_WAKEUP_LEVEL       m_eNoteLevel;
    E_NOTE_TIME_SPAN_TYPE           m_eTimeSpanType;
    INT64                           m_s64RemindTime;
    INT64                           m_s64WriteTime;
    INT64                           m_s64ModifyTime;
    E_NOTE_REMIND_FREQUENCY         m_eRemindFrequency;
    E_NOTE_TYPE                     m_eNoteType;
    BOOL                            m_bIsTop;                   ///< 置顶标记 (TRUE: 已置顶 / FALSE: 普通)
    INT32                           m_s32TopOrder;
    BOOL                            m_bCompleted;
    BOOL                            m_bDeleted;
    std::string m_strContent;
    st_NoteModelItem()
        : m_s64NoteId(0)
        , m_eNoteLevel(E_NOTE_EVENT_WAKEUP_LEVEL_NORMAL)
        , m_eTimeSpanType(E_NOTE_TIME_SPAN_ONCE)
        , m_s64RemindTime(0)
        , m_s64WriteTime(0)
        , m_s64ModifyTime(0)
        , m_eRemindFrequency(E_NOTE_REMIND_NONE)
        , m_eNoteType(E_NOTE_TYPE_ALL)
        , m_bIsTop(FALSE)
        , m_s32TopOrder(-1)
        , m_bCompleted(FALSE)
        , m_bDeleted(FALSE) {}
}NOTE_MODEL_ITEM;

namespace NDataManager
{
    constexpr INT32 NOTE_BUFFER_ITEM_COUNT_MAX = 128;

}
