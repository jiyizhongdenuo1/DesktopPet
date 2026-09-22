/*
* @file: CNoteBusiness.h
 * @brief:
 * @author: nuo
 * @date: 2026/7/23
 * @Detail:
 */

#pragma once

#include <memory>
#include <map>
#include <array>
#include "datatype.h"
#include "DServiceBase.h"

class CNoteBusiness
{
public:
    explicit CNoteBusiness();

    INT32 ProcessTip(const std::array<ST_NOTE_DATA, DDataCache::MAX_CACHE_SIZE> &arrstNoteData, INT32 s32Count);

private:
    /** 过滤：已删除 / 已完成 / 不提醒 / 无提醒时间 */
    BOOL ShouldSkip(const ST_NOTE_DATA& stNote) const;

    /** 频率判定：单次 / 每天 / 每周 / 每月 */
    BOOL MatchFrequency(const ST_NOTE_DATA& stNote, time_t s64Now) const;
private:
    std::map<INT32, time_t>     m_mapLastTipTime;   // 节流（业务状态，留在这里）
};
