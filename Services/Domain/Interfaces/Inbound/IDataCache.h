/*
* @file: IDataCache.h
 * @brief: 笔记数据缓存接口
 * @author: nuo
 * @date: 2026/9/21
 * @Detail:
 */

#pragma once

#include <memory>

#include "DServiceBase.h"
#include "DDataMgrBase.h"
class IDataCache
{
public:
    explicit IDataCache() = default;

    virtual ~IDataCache() = default;

    /** ***********************************************************
     * @brief       获取当前缓存快照（线程安全）
     * @return      缓存快照的共享指针，无数据时返回 nullptr
     * @note        返回的是不可变视图，调用方只读不写
     ************************************************************/
    virtual NOTE_CACHE_SNAPSHOT GetSnapshot() const { return nullptr; }

    /** ***********************************************************
     * @brief       获取缓存中有效数据的字节大小
     * @return      有效数据字节数（有效条数 * sizeof(ST_NOTE_DATA)）
     ************************************************************/
    virtual INT32 GetCacheSize() const { return 0; }

    /** ***********************************************************
     * @brief       根据 id 更新缓存中的一条笔记
     * @param[in]   stNoteData 待更新的笔记数据（含目标 id）
     * @note        若缓存中未找到匹配 id，则不做任何操作
     ************************************************************/
    virtual void UpdateNoteDataCache(const ST_NOTE_DATA& stNoteData) { }
};