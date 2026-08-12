/*
 * @file: CNoteDataService.h
 * @brief: 笔记数据服务层，负责数据转换、缓存管理和待写队列
 * @author: nuo
 * @date: 2026/7/24
 * @Detail: 接收UI层数据转换为存储格式，同时写入缓存和待写队列(Collect)
 */

#pragma once
#include <memory>
#include <functional>
#include "DServiceBase.h"
#include "DDataMgrBase.h"
#include "DDataCache.h"

class CNoteDataCache;
class INoteDataBuffer;

using CALLBACK_NOTEDATALOAD = std::function<void(std::shared_ptr<std::array<ST_NOTE_DATA, DDataCache::MAX_CACHE_SIZE>> pArrData, INT32 s32Count)>;

class CNoteDataService
{
public:
    /** ***********************************************************
     * @brief       构造函数
     * @param[in]   pCache 缓存实例
     * @param[in]   pBuffer 缓冲区接口实例（待写队列）
     ************************************************************/
    CNoteDataService(std::shared_ptr<CNoteDataCache> pCache,
                     std::shared_ptr<INoteDataBuffer> pBuffer);

    /** ***********************************************************
     * @brief       析构函数
     ************************************************************/
    ~CNoteDataService();

    /** ***********************************************************
     * @brief       从UI层添加笔记数据（自动转换格式并分发到缓存和待写队列）
     * @param[in]   stModelItem  UI层数据结构
     * @note        数据会同时写入缓存(用于显示)和Collect(供读写线程持久化)
     ************************************************************/
    void AddNote(const NOTE_MODEL_ITEM &stModelItem);

    /** ***********************************************************
     * @brief       失效缓存
     * @return      void
     ************************************************************/
    void InvalidateCache();

    /** ***********************************************************
     * @brief       将原始字节缓冲区解析为笔记数据并存入缓存
     * @param[in]   pBuffer        原始字节数据缓冲区
     * @param[in]   s32BufferSize  缓冲区大小（字节）
     * @return      成功存入缓存的数据条数，失败返回0
     * @note        内部按 sizeof(ST_NOTE_DATA) 切分缓冲区，
     *              逐条调用 SaveNoteDataCache 存入缓存
     ************************************************************/
    int LoadFromBuffer(char *pBuffer, INT32 s32BufferSize);

    VOID RegisterNoteModelDataLoadCallback(CALLBACK_NOTEDATALOAD NoteDataCallback);
private:
    /** ***********************************************************
     * @brief       将 UI 层数据转换为领域层数据格式
     * @param[in]   stModelItem  UI层数据结构
     * @param[out]  stNoteData   输出的领域层数据结构
     ************************************************************/
    static void ConvertUIToDomain(const NOTE_MODEL_ITEM &stModelItem, ST_NOTE_DATA &stNoteData);

    void NotifyDataLoaded(INT32 s32Count);

    std::shared_ptr<CNoteDataCache>   m_pCache;      ///< 缓存实例（用于显示）
    std::shared_ptr<INoteDataBuffer>  m_pBuffer;     ///< 缓冲区接口实例（待写队列）
    CALLBACK_NOTEDATALOAD m_NoteDataCallback;    ///< 加载回调函数
};