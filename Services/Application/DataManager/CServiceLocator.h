/*
 * @file: CServiceLocator.h
 * @brief: 全局服务定位器（单例模式）
 * @author: nuo
 * @date: 2026/8/1
 * @Detail:
 */

#pragma once

#include <memory>
#include <mutex>

class CNoteDataCache;
class CNoteDataService;
class INoteDataBuffer;
class CDataRWMgr;

/** ***********************************************************
 * @brief       全局服务定位器（单例模式）
 ************************************************************/
class CServiceLocator
{
public:

    /** ***********************************************************
     * @brief       获取单例实例
     * @return      ServiceLocator 的引用
     ************************************************************/
    static CServiceLocator& GetInstance();

    /** ***********************************************************
     * @brief       禁止拷贝构造
     ************************************************************/
    CServiceLocator(const CServiceLocator&) = delete;

    /** ***********************************************************
     * @brief       禁止赋值操作
     ************************************************************/
    CServiceLocator& operator=(const CServiceLocator&) = delete;

    /** ***********************************************************
     * @brief       注册笔记缓存实例
     * @param[in]   pCache 缓存实例的共享指针
     ************************************************************/
    void RegisterNoteCache(std::shared_ptr<CNoteDataCache> pCache);

    /** ***********************************************************
     * @brief       注册笔记服务实例
     * @param[in]   pService 服务实例的共享指针
     ************************************************************/
    void RegisterNoteService(std::shared_ptr<CNoteDataService> pService);

    /** ***********************************************************
     * @brief       注册数据保存器实例
     * @param[in]   pSaver 保存器实例的共享指针
     ************************************************************/
    void RegisterDataSaver(std::shared_ptr<CDataRWMgr> pSaver);

    /** ***********************************************************
     * @brief       注册笔记数据缓冲区实例（接口类型）
     * @param[in]   pBuffer 缓冲区接口实例的共享指针
     ************************************************************/
    void RegisterNoteCollect(std::shared_ptr<INoteDataBuffer> pBuffer);

    /** ***********************************************************
     * @brief       获取笔记缓存实例
     * @return      缓存实例的 shared_ptr
     ************************************************************/
    std::shared_ptr<CNoteDataCache> GetNoteCache() const;

    /** ***********************************************************
     * @brief       获取笔记服务实例
     * @return      服务实例的 shared_ptr
     ************************************************************/
    std::shared_ptr<CNoteDataService> GetNoteService() const;

    /** ***********************************************************
     * @brief       获取数据保存器实例
     * @return      保存器实例的 shared_ptr
     ************************************************************/
    std::shared_ptr<CDataRWMgr> GetDataRWMgr() const;

    /** ***********************************************************
     * @brief       获取笔记数据缓冲区实例（接口类型）
     * @return      缓冲区接口实例的 shared_ptr
     ************************************************************/
    std::shared_ptr<INoteDataBuffer> GetNoteCollect() const;

private:

    /** ***********************************************************
     * @brief       构造函数（私有）
     ************************************************************/
    CServiceLocator() = default;

    /** ***********************************************************
     * @brief       析构函数
     ************************************************************/
    ~CServiceLocator() = default;

    mutable std::mutex m_mutex;                              ///< 保护注册表读写操作的互斥锁

    std::shared_ptr<CNoteDataCache>       m_pNoteCache;     ///< 笔记缓存实例
    std::shared_ptr<CNoteDataService>   m_pNoteService;   ///< 笔记服务实例
    std::shared_ptr<CDataRWMgr>      m_pDataSaveRWMgr; ///< 数据保存器实例
    std::shared_ptr<INoteDataBuffer>    m_pNoteCollect;   ///< 笔记数据缓冲区（接口类型）
};

/** ***********************************************************
 * @brief       全局便捷宏（简化访问）
 * @detail      推荐使用此宏代替直接调用 GetInstance()
 * @example     g_ServiceLocator.GetNoteCache()
 ************************************************************/
#define g_ServiceLocator CServiceLocator::GetInstance()