/*
 * @file: CNoteApp.h
 * @brief:
 * @author: nuo
 * @date: 2026/7/23
 * @Detail:
 */

#pragma once

#include <memory>
#include <function.h>
#include "datatype.h"
#include "CNoteBusiness.h"

class IDataCache;
class CNoteApp
{
public:
    explicit CNoteApp(std::shared_ptr<IDataCache> pCache);   // 注入

    ~CNoteApp() = default;

    void DoSecend();

    /** 注册提示回调，把"该提示哪条"推给上层 */
    VOID RegisterTipCallback(std::function<void(const ST_NOTE_DATA&)> fnTip);

    VOID UpdataData();

private:
    /** 单条判定：到点 + 未完成 + 未删除 */
    static BOOL IsDue(const ST_NOTE_DATA& stNote, time_t s64Now);

    /** 频率判定：单次 / 每天 / 每周 / 每月 是否命中 */
    static BOOL MatchFrequency(E_NOTE_REMIND_FREQUENCY eFreq,
                               time_t s64RemindTime,
                               time_t s64Now);

    std::unique_ptr<CNoteBusiness>         m_pBusiness;
    std::shared_ptr<IDataCache>            m_pCache;
    std::map<INT32, time_t>                m_mapLastTipTime;   // 节流：每条上次提示时间
    std::vector< std::function<void(const ST_NOTE_DATA&)>> m_vecfnTip;
};
