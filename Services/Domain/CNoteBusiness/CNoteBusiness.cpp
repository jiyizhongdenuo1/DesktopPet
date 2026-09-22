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
static BOOL IsTimeOfDayPassed(time_t tNow, time_t tRemind)
{
    std::tm now{}, rmd{};
    if (!ToLocalTm(tNow, now) || !ToLocalTm(tRemind, rmd))
    {
        return FALSE;
    }

    const INT32 s32NowSec = now.tm_hour * 3600 + now.tm_min * 60 + now.tm_sec;
    const INT32 s32RmdSec = rmd.tm_hour * 3600 + rmd.tm_min * 60 + rmd.tm_sec;

    return s32NowSec >= s32RmdSec;
}

static INT64 DayInterval(time_t t1, time_t t2)
{
    return LocalDayNumber(t2) - LocalDayNumber(t1);
}

CNoteBusiness::CNoteBusiness()
{

}

INT32 CNoteBusiness::ProcessTip(const array<ST_NOTE_DATA, DDataCache::MAX_CACHE_SIZE> &arrstNoteData, INT32 s32Count)
{
    std::time_t t_Current = std::time(nullptr);

    for (INT32 i = 0; i < s32Count; ++i)
    {
        if (!ShouldSkip(arrstNoteData[i]) && MatchFrequency(arrstNoteData[i], t_Current) && DayInterval(arrstNoteData[i].m_s64RemindTime, t_Current) > 0)
        {
            return arrstNoteData[i].m_s32id;
        }
    }
    return -1;
}

BOOL CNoteBusiness::ShouldSkip(const ST_NOTE_DATA &stNote) const
{
    if (stNote.m_bCompleted || stNote.m_bDeleted || stNote.m_s64RemindTime <= 0)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL CNoteBusiness::MatchFrequency(const ST_NOTE_DATA &stNote, time_t s64Now) const
{
    switch(stNote.m_eRemindFrequency)
    {
        case E_NOTE_REMIND_NONE:
        {
            return FALSE;
        }
        break;
        case E_NOTE_REMIND_ONCE:
        {
            return (IsTimeOfDayPassed(s64Now, stNote.m_s64RemindTime) && 0 >= stNote.m_S64LastRemindTime);
        }
        break;
        case E_NOTE_REMIND_DAILY:
        {
            return IsTimeOfDayPassed(s64Now, stNote.m_s64RemindTime);
        }
        break;
        case E_NOTE_REMIND_WEEKLY:
        {
            return (DayInterval(stNote.m_S64LastRemindTime, s64Now) >= 7 && IsTimeOfDayPassed(s64Now, stNote.m_s64RemindTime));
        }
        break;
        case E_NOTE_REMIND_MONTHLY:
        {
            return (DayInterval(stNote.m_S64LastRemindTime, s64Now) >= 30 && IsTimeOfDayPassed(s64Now, stNote.m_s64RemindTime));
        }
        break;
        case E_NOTE_REMIND_CUSTOM:
        {
            return (DayInterval(stNote.m_S64LastRemindTime, s64Now) >= stNote.m_s16CustomInterval && IsTimeOfDayPassed(s64Now, stNote.m_s64RemindTime));
        }
        break;
        default:
        {
            return FALSE;
        }
        break;
    }
}
