/*
 * @file: INoteCache.h
 * @brief: 笔记缓存接口（应用层读写）
 * @author: nuo
 * @date: 2026/10/9
 * @Detail: 应用层访问缓存能力的出站端口，由基础设施实现
 */

#pragma once

#include "DServiceBase.h"
#include "DDataMgrBase.h"

class INoteCache
{
public:
    virtual ~INoteCache() = default;

    /** 获取当前缓存快照 */
    virtual NOTE_CACHE_SNAPSHOT GetSnapshot() const = 0;

    /** 获取缓存中有效数据的字节大小 */
    virtual INT32 GetCacheSize() const = 0;

    /** 根据 id 更新缓存中的一条笔记 */
    virtual void UpdateNoteDataCache(const ST_NOTE_DATA &stNoteData) = 0;

    /** 清空缓存 */
    virtual void InvalidateCache() = 0;

    /** 注册缓存变更回调 */
    virtual void SetChangeCallback(CACHE_CHANGE_CALLBACK cb) = 0;

    /** 从原始缓冲区解析笔记数据并写入缓存，返回写入条数 */
    virtual INT32 PutBuffer2CacheData(char *pBuffer, INT32 s32BufferSize) = 0;

    /** 从原始缓冲区解析回收站数据并写入缓存，返回写入条数 */
    virtual INT32 PutBuffer2RecycleData(char *pBuffer, INT32 s32BufferSize) = 0;
};
