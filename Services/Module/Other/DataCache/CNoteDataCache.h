/*
 * @file: CNoteDataCache.h
 * @brief: 
 * @author: nuo
 * @date: 2026/7/25
 * @Detail:
 */

#pragma once

#include <memory>
#include <array>
#include <vector>

#include "DServiceBase.h"
#include "DDataCache.h"

class CNoteDataCachePrivate;
/** ***********************************************************
 * @brief       笔记数据缓存（Copy-on-Write 模式）
 ************************************************************/
class CNoteDataCache
{
public:
    /** ***********************************************************
     * @brief       构造函数
     ************************************************************/
    explicit CNoteDataCache();

    /** ***********************************************************
     * @brief       析构函数
     ************************************************************/
    ~CNoteDataCache();

    /** ***********************************************************
     * @brief       清空当前缓存
     * @param[in]   无
     * @return      void
     ************************************************************/
    void InvalidateCache();

    /** ***********************************************************
     * @brief       获取缓存快照
     * @param[in]   无
     * @return      缓存数据的共享指针
     ************************************************************/
    std::shared_ptr<std::array<ST_NOTE_DATA, DDataCache::MAX_CACHE_SIZE>> GetCache();

    /** ***********************************************************
     * @brief       保存单条数据到缓存
     * @param[in]   stNoteData 笔记数据
     * @return      void
     ************************************************************/
    void SaveNoteDataCache(const ST_NOTE_DATA& stNoteData);

    /** ***********************************************************
     * @brief       更新缓存内容
     * @param[in]   vecNoteData 新的数据向量
     * @return      void
     ************************************************************/
    void UpdateNoteDataCache(const std::vector<ST_NOTE_DATA>& vecNoteData);

    /** ***********************************************************
     * @brief       从原始缓冲区写入缓存
     * @param[in]   pBuffer        原始数据缓冲区
     * @param[in]   s32BufferSize  缓冲区大小（字节）
     * @return      TRUE-成功，FALSE-失败
     * @note        按 sizeof(ST_NOTE_DATA) 切分后直接 memcpy 到固定数组缓存，
     *              超出 MAX_CACHE_SIZE 的部分会被截断
     ************************************************************/
    BOOL PutBuffer2CacheData(char *pBuffer, INT32 s32BufferSize);

    /** ***********************************************************
     * @brief       获取缓存中有效数据的字节大小
     * @param[in]   无
      * @return      有效数据字节数（m_s32Count * sizeof(ST_NOTE_DATA)）
     * @note        返回的是实际写入的数据量，非数组总容量；m_s32Count 上限 MAX_CACHE_SIZE
     ************************************************************/
    INT32 GetCacheSize();

private:
    std::unique_ptr<CNoteDataCachePrivate> d_ptr;
};