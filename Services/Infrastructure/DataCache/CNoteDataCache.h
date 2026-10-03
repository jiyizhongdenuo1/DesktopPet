/*
 * @file: CNoteDataCache.h
 * @brief: 笔记数据缓存（Copy-on-Write 模式）
 * @author: nuo
 * @date: 2026/7/25
 * @Detail:
 */

#pragma once

#include <memory>

#include "DServiceBase.h"
#include "IDataCache.h"
#include "DDataMgrBase.h"

class CNoteDataCachePrivate;
/** ***********************************************************
 * @brief       笔记数据缓存（Copy-on-Write 模式）
 ************************************************************/
class CNoteDataCache : public IDataCache
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
     * @brief       保存单条数据到缓存（环形写入）
     * @param[in]   stNoteData 笔记数据
     * @return      void
     * @note        写满 MAX_CACHE_SIZE 后覆盖最旧数据
     ************************************************************/
    void SaveNoteDataCache(const ST_NOTE_DATA& stNoteData);

    /** ***********************************************************
     * @brief       根据 id 更新缓存中的一条笔记
     * @param[in]   stNoteData 待更新的笔记数据（含目标 id）
     * @return      void
     * @note        若缓存中未找到匹配 id，则不做任何操作
     ************************************************************/
    void UpdateNoteDataCache(const ST_NOTE_DATA& stNoteData) override;

    /** ***********************************************************
     * @brief       从原始缓冲区写入缓存
     * @param[in]   pBuffer        原始数据缓冲区
     * @param[in]   s32BufferSize  缓冲区大小（字节）
     * @return      实际写入的条数（按 sizeof(ST_NOTE_DATA) 切分）
     * @note        超出 MAX_CACHE_SIZE 的部分会被截断
     ************************************************************/
    INT32 PutBuffer2CacheData(char *pBuffer, INT32 s32BufferSize);

    /** ***********************************************************
     * @brief       从原始缓冲区写入回收站缓存
     * @param[in]   pBuffer        原始数据缓冲区
     * @param[in]   s32BufferSize  缓冲区大小（字节）
     * @return      实际写入的条数（按 sizeof(ST_NOTE_DATA) 切分）
     * @note        超出 MAX_CACHE_SIZE 的部分会被截断
     ************************************************************/
    INT32 PutBuffer2RecycleData(char *pBuffer, INT32 s32BufferSize);

    /** ***********************************************************
     * @brief       批量追加笔记数据到缓存（环形写入）
     * @param[in]   pArrNote 笔记数据数组
     * @param[in]   s32Count  数组条数
     * @return      void
     * @note        整批只发布一次快照；写满后覆盖最旧数据
     ************************************************************/
    void AppendNoteData(const ST_NOTE_DATA* pArrNote, INT32 s32Count);

    /** ***********************************************************
     * @brief       批量追加回收站数据到缓存（环形写入）
     * @param[in]   pArrNote 笔记数据数组
     * @param[in]   s32Count  数组条数
     * @return      void
     * @note        整批只发布一次回收站快照；写满后覆盖最旧数据
     ************************************************************/
    void AppendRecycleNoteData(const ST_NOTE_DATA* pArrNote, INT32 s32Count);

    /** ***********************************************************
     * @brief       获取当前缓存快照（线程安全）
     * @return      缓存快照的共享指针，无数据时返回 nullptr
     ************************************************************/
    NOTE_CACHE_SNAPSHOT GetSnapshot() const override;

    /** ***********************************************************
     * @brief       获取回收站缓存快照（线程安全）
     * @return      回收站快照的共享指针，无数据时返回 nullptr
     ************************************************************/
    NOTE_CACHE_SNAPSHOT GetRecycleSnapshot() const;

    /** ***********************************************************
     * @brief       注册缓存变更回调（新增/更新）
     * @param[in]   cb 变更回调
     * @note        回调在写接口内部于锁外触发，执行在调用者线程；
     *              跨线程使用时由回调方自行 marshal 到目标线程
     ************************************************************/
    void SetChangeCallback(CACHE_CHANGE_CALLBACK cb);

    /** ***********************************************************
     * @brief       获取缓存中有效数据的字节大小
     * @return      有效数据字节数（有效条数 * sizeof(ST_NOTE_DATA)）
     ************************************************************/
    INT32 GetCacheSize() const override;

private:
    /** ***********************************************************
     * @brief       将当前有效数据发布为新的快照（原子替换）
     * @param[in]   无
     * @return      void
     * @note        内部已加锁，调用方需已持有写锁或保证独占
     ************************************************************/
    void PublishSnapshot();

    /** ***********************************************************
     * @brief       将回收站有效数据发布为新的快照（原子替换）
     * @param[in]   无
     * @return      void
     * @note        内部已加锁，调用方需已持有写锁或保证独占
     ************************************************************/
    void PublishRecycleSnapshot();

    /** ***********************************************************
     * @brief       清空缓存数组、写指针、计数，并发布快照
     * @param[in]   无
     * @return      void
     ************************************************************/
    void ClearCache();

private:
    std::unique_ptr<CNoteDataCachePrivate> d_ptr;
};
