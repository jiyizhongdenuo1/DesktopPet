#pragma once

#include "DDataMgrBase.h"
#include "DServiceBase.h"
#include <algorithm>
#include <cstring>

namespace NoteMapping
{

/** UI 层模型转换为领域/存储结构 */
inline void ToDomain(const NOTE_MODEL_ITEM &stSrc, ST_NOTE_DATA &stDst)
{
    stDst = {};

    stDst.m_s32id             = stSrc.m_s64NoteId;
    stDst.m_eNoteLevel        = stSrc.m_eNoteLevel;
    stDst.m_eTimeSpanType     = stSrc.m_eTimeSpanType;
    stDst.m_eNoteType         = stSrc.m_eNoteType;
    stDst.m_s64RemindTime     = stSrc.m_s64RemindTime;
    stDst.m_s64CreateTime     = stSrc.m_s64WriteTime;
    stDst.m_s64UpdateTime     = stSrc.m_s64ModifyTime;
    stDst.m_eRemindFrequency  = stSrc.m_eRemindFrequency;
    stDst.m_s16CustomInterval = static_cast<INT16U>(stSrc.m_s32CustomInterval);
    stDst.m_bCompleted        = stSrc.m_bCompleted;
    stDst.m_bDeleted          = stSrc.m_bDeleted;
    stDst.m_bIsTop            = stSrc.m_bIsTop;
    stDst.m_s32TopOrder       = stSrc.m_s32TopOrder;

    const INT32 s32CopySize = std::min(
        static_cast<INT32>(stSrc.m_strContent.size()),
        static_cast<INT32>(NoteSpace::CONTENT_LENGTH_MAX - 1)
    );

    if (s32CopySize > 0)
    {
        memcpy(stDst.m_szContent, stSrc.m_strContent.c_str(), s32CopySize + 1);
    }
    else
    {
        stDst.m_szContent[0] = '\0';
    }
}

/** 领域/存储结构转换为 UI 层模型 */
inline void ToModel(const ST_NOTE_DATA &stSrc, NOTE_MODEL_ITEM &stDst)
{
    stDst = {};

    stDst.m_s64NoteId         = stSrc.m_s32id;
    stDst.m_eNoteLevel        = stSrc.m_eNoteLevel;
    stDst.m_eTimeSpanType     = stSrc.m_eTimeSpanType;
    stDst.m_s64RemindTime     = stSrc.m_s64RemindTime;
    stDst.m_s64WriteTime      = stSrc.m_s64CreateTime;
    stDst.m_s64ModifyTime     = stSrc.m_s64UpdateTime;
    stDst.m_eRemindFrequency  = stSrc.m_eRemindFrequency;
    stDst.m_s32CustomInterval = static_cast<INT32>(stSrc.m_s16CustomInterval);
    stDst.m_eNoteType         = stSrc.m_eNoteType;
    stDst.m_bCompleted        = stSrc.m_bCompleted;
    stDst.m_bDeleted          = stSrc.m_bDeleted;
    stDst.m_bIsTop            = stSrc.m_bIsTop;
    stDst.m_s32TopOrder       = stSrc.m_s32TopOrder;
    stDst.m_strContent        = stSrc.m_szContent;
}

}   // namespace NoteMapping
