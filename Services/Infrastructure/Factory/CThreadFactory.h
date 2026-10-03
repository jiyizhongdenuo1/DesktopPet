/*
 * @file: CThreadFactory.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/8
 * @Detail:
 */

#pragma once

#include <memory>

#include "IFactory.h"
#include "CThread.h"

class CThreadHandler;
class CDataRWMgr;
class CNoteDataCache;
class CNoteDataService;
class INoteDataBuffer;

/**
 * @brief 线程工厂类
 * @detail 负责创建 CThread 对象，封装线程创建逻辑
 */
class DLL_EXPORT CThreadFactory : public IFactory<CThread>
{
public:
    /**
     * @brief 构造函数
     * @param pDataRWMgr     数据读写管理器（注入给数据保存 Handler）
     * @param pBuffer        笔记数据缓冲区（注入给数据保存 Handler）
     * @param pRecycleBuffer 回收站数据缓冲区（注入给数据保存 Handler）
     * @param pCache         笔记缓存（注入给数据保存 Handler）
     * @param pService       笔记数据服务（注入给数据保存 Handler）
     */
    explicit CThreadFactory(std::shared_ptr<CDataRWMgr>       pDataRWMgr,
                            std::shared_ptr<INoteDataBuffer>  pBuffer,
                            std::shared_ptr<INoteDataBuffer>  pRecycleBuffer,
                            std::shared_ptr<CNoteDataCache>   pCache,
                            std::shared_ptr<CNoteDataService> pService);

    /**
     * @brief 析构函数
     */
    ~CThreadFactory();

    /**
     * @brief 创建线程对象（需要手动指定参数）
     * @param strName 线程名称
     * @param pHandle 线程处理器
     * @param s32Interval 唤醒间隔（毫秒）
     * @return 返回创建的线程对象指针
     */
    std::unique_ptr<CThread> Create(const QString &strName, std::shared_ptr<CThreadHandler> pHandle, INT32 s32Interval = 1000);

    /**
     * @brief 创建线程对象（无参版本，返回空指针）
     * @note 此方法为接口实现，实际应使用带参数的 Create 方法
     * @return 返回 nullptr
     */
    // CThread* Create() override;

public:
    /**
     * @brief 转移线程所有权
     * @return 返回包含所有线程的 vector，调用后工厂不再持有线程
     */
    std::vector<std::shared_ptr<CThread>> ReleaseThreads()
    {
        return std::move(m_vecpThread);
    }

    /**
     * @brief 转移线程处理器所有权
     * @return 返回包含所有处理器的 vector，调用后工厂不再持有处理器
     */
    std::vector<std::shared_ptr<CThreadHandler>> ReleaseHandlers()
    {
        return std::move(m_vecpThreadHanders);
    }

private:
    // 禁用拷贝
    CThreadFactory(const CThreadFactory&) = delete;
    CThreadFactory& operator=(const CThreadFactory&) = delete;

    void InitInstances();
    void InitThread();
    void InitThreadHanders();
    void ThreadModule();
private:
    std::vector<std::shared_ptr<CThread>>                   m_vecpThread;
    std::vector<std::shared_ptr<CThreadHandler>>            m_vecpThreadHanders;

    std::shared_ptr<CDataRWMgr>                             m_pDataRWMgr;
    std::shared_ptr<INoteDataBuffer>                        m_pBuffer;
    std::shared_ptr<INoteDataBuffer>                        m_pRecycleBuffer;
    std::shared_ptr<CNoteDataCache>                         m_pCache;
    std::shared_ptr<CNoteDataService>                       m_pService;
};
