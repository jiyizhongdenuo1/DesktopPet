/*
* @file: CNoteBusiness.h
 * @brief: 笔记业务逻辑层
 * @author: nuo
 * @date: 2026/7/23
 * @Detail:
 */

#pragma once

#include <memory>
#include <vector>
#include "datatype.h"
#include "DServiceBase.h"
#include "DDataMgrBase.h"

/** 判定用的"当前时间"上下文：三个值取自同一时刻，循环内复用，避免重复换算 */
struct SNowTime
{
    time_t m_tNow;      ///< 当前时刻
    INT64  m_s64Day;    ///< 当前本地日历日序号
    INT32  m_s32Sec;    ///< 当前时刻在当天的秒数（从 00:00:00 起算）
};

/** ***********************************************************
 * @brief       笔记业务逻辑：提醒判定、频率匹配等
 ************************************************************/
class CNoteBusiness
{
public:
    /** ***********************************************************
     * @brief       构造函数
     ************************************************************/
    explicit CNoteBusiness();

    /** ***********************************************************
     * @brief       扫描缓存，返回本次应触发提醒的笔记下标集合
     * @param[in]   parrCache  笔记缓存快照（只读）
     * @return      需要提醒的笔记在快照中的下标，空向量表示无提醒
     ************************************************************/
    std::vector<INT32> ProcessTip(NOTE_CACHE_SNAPSHOT parrCache);

private:
    /** ***********************************************************
     * @brief       过滤：已删除 / 已完成 / 不提醒 / 无提醒时间
     * @param[in]   stNote 待判定笔记
     * @return      TRUE-应跳过，FALSE-进入后续判定
     ************************************************************/
    BOOL ShouldSkip(const ST_NOTE_DATA& stNote) const;

    /** ***********************************************************
     * @brief       频率判定：单次 / 每天 / 每周 / 每月
     * @param[in]   stNote 待判定笔记
     * @param[in]   stNow   当前时间上下文（循环内复用，避免重复换算）
     * @return      TRUE-当前时刻应提醒，FALSE-不在提醒周期内
     ************************************************************/
    BOOL MatchFrequency(const ST_NOTE_DATA& stNote, const SNowTime& stNow) const;
};
