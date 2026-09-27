/*
 * @file: CNoteApp.h
 * @brief: 笔记应用层入口
 * @author: nuo
 * @date: 2026/7/23
 * @Detail:
 */

#pragma once

#include <memory>
#include <functional>
#include <map>
#include <vector>
#include <ctime>
#include "datatype.h"
#include "DServiceBase.h"

class IDataCache;
class CNoteBusiness;

/** ***********************************************************
 * @brief       笔记应用层：持有缓存与业务逻辑，周期性触发提醒
 ************************************************************/
class CNoteApp
{
public:
    /** ***********************************************************
     * @brief       构造函数
     ************************************************************/
    explicit CNoteApp();

    /** ***********************************************************
     * @brief       析构函数
     ************************************************************/
    ~CNoteApp();

    /** ***********************************************************
     * @brief       每秒驱动一次：扫描缓存、判定提醒、推送回调
     * @param[in]   无
     * @return      void
     ************************************************************/
    void DoSecend();

    /** ***********************************************************
     * @brief       注册提示回调，把"该提示哪条"推给上层
     * @param[in]   fnTip 提示回调函数
     * @return      void
     ************************************************************/
    VOID RegisterTipCallback(std::function<void(const ST_NOTE_DATA&)> fnTip);

    /** ***********************************************************
     * @brief       注入数据缓存依赖
     * @param[in]   pCache 缓存实例的共享指针
     * @return      void
     ************************************************************/
    VOID SetCache(std::shared_ptr<IDataCache> pCache);

private:
    std::shared_ptr<IDataCache>            m_pCache;
    std::unique_ptr<CNoteBusiness>         m_pBusiness;
    std::map<INT32, time_t>                m_mapLastTipTime;   // 节流：每条上次提示时间
    std::vector< std::function<void(const ST_NOTE_DATA&)>> m_vecfnTip;
};