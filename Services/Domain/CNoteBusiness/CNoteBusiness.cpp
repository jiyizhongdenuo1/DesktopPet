/*
 * @file: CNoteBusiness.h
 * @brief:
 * @author: nuo
 * @date: 2026/7/23
 * @Detail:
 */

#include <ctime>
#include <array>
#include "CNoteBusiness.h"
#include "DTranslation.h"

using namespace std;

/** 一天内的秒数（从 00:00:00 起算）；转换失败返回 -1 */
static INT32 TimeOfDaySeconds(time_t t)
{
    std::tm tmBuf{};
    if (!ToLocalTm(t, tmBuf))
    {
        return -1;
    }

    return tmBuf.tm_hour * 3600 + tmBuf.tm_min * 60 + tmBuf.tm_sec;
}

CNoteBusiness::CNoteBusiness()
{

}

vector<INT32> CNoteBusiness::ProcessTip(NOTE_CACHE_SNAPSHOT parrCache)
{
    // 与"现在"有关的三项只算一次，循环内复用
    SNowTime stNow;
    stNow.m_tNow   = std::time(nullptr);
    stNow.m_s64Day = LocalDayNumber(stNow.m_tNow);
    stNow.m_s32Sec = TimeOfDaySeconds(stNow.m_tNow);
    std::vector<INT32> vec_TipCache;
    vec_TipCache.clear();

    if (stNow.m_s32Sec < 0)
    {
        return vec_TipCache;
    }
    for (size_t i = 0; i < parrCache->size(); ++i)
    {
        const auto &p_Note = (*parrCache)[i];
        if (!p_Note)
        {
            continue;
        }
        const ST_NOTE_DATA &st_Note = *p_Note;
        if (ShouldSkip(st_Note))
        {
            continue;
        }

        // 提醒日还没到
        const INT64 s64DayDiff = stNow.m_s64Day - LocalDayNumber(st_Note.m_s64RemindTime);
        if (s64DayDiff < 0)
        {
            continue;
        }

        // 提醒日就是今天，但设定时刻早于创建时刻（设定时已过）→ 顺延到下一个周期
        if (s64DayDiff == 0 && st_Note.m_s64RemindTime < st_Note.m_s64CreateTime)
        {
            continue;
        }

        if (MatchFrequency(st_Note, stNow))
        {
            vec_TipCache.push_back(i);
        }

    }
    return vec_TipCache;
}

BOOL CNoteBusiness::ShouldSkip(const ST_NOTE_DATA &stNote) const
{
    if (stNote.m_bCompleted || stNote.m_bDeleted || stNote.m_s64RemindTime <= 0)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL CNoteBusiness::MatchFrequency(const ST_NOTE_DATA &stNote, const SNowTime &stNow) const
{
    // 不提醒
    if (stNote.m_eRemindFrequency == E_NOTE_REMIND_NONE)
    {
        return FALSE;
    }

    // 单次：到点且从未提醒过（用精确时刻比较，日期与钟点一次判完）
    if (stNote.m_eRemindFrequency == E_NOTE_REMIND_ONCE)
    {
        return (stNow.m_tNow >= stNote.m_s64RemindTime && stNote.m_S64LastRemindTime <= 0);
    }

    // 周期类：先统一确认"当天钟点已过"，再按周期取阈值
    const INT32 s32RemindSec = TimeOfDaySeconds(stNote.m_s64RemindTime);
    if (s32RemindSec < 0 || stNow.m_s32Sec < s32RemindSec)
    {
        return FALSE;
    }

    const INT64 s64LastDiff = stNow.m_s64Day - LocalDayNumber(stNote.m_S64LastRemindTime);

    switch(stNote.m_eRemindFrequency)
    {
        case E_NOTE_REMIND_DAILY:
        {
            return (s64LastDiff >= 1);
        }
        case E_NOTE_REMIND_WEEKLY:
        {
            return (s64LastDiff >= 7);
        }
        case E_NOTE_REMIND_MONTHLY:
        {
            return (s64LastDiff >= 30);
        }
        case E_NOTE_REMIND_CUSTOM:
        {
            return (s64LastDiff >= stNote.m_s16CustomInterval);
        }
        default:
        {
            return FALSE;
        }
    }
}
