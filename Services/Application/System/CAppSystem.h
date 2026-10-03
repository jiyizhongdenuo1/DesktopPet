/*
 * @file: CAppSystem.h
 * @brief: 应用系统管理类（线程管理、任务调度）
 * @author: nuo
 * @date: 2026/6/4
 * @update: 2026/9/4 - 移除 QObject 依赖，优化为纯 C++ 类
 */

#pragma once

#include <memory>

#include "datatype.h"
#include "GlobalEnums.h"

class CThread;
class CThreadHandler;
class CAppSystemPrivate;
class IDataCache;
class CDataRWMgr;
class CNoteDataCache;
class CNoteDataService;
class INoteDataBuffer;
class CAppSystem
{
public:
    /** ***********************************************************
     * @brief       构造函数
     * @param[in]   pDataRWMgr     数据读写管理器
     * @param[in]   pBuffer        笔记数据缓冲区
     * @param[in]   pRecycleBuffer 回收站数据缓冲区
     * @param[in]   pCache         笔记缓存
     * @param[in]   pService       笔记数据服务
     ************************************************************/
    explicit CAppSystem(std::shared_ptr<CDataRWMgr>       pDataRWMgr,
                        std::shared_ptr<INoteDataBuffer>  pBuffer,
                        std::shared_ptr<INoteDataBuffer>  pRecycleBuffer,
                        std::shared_ptr<CNoteDataCache>   pCache,
                        std::shared_ptr<CNoteDataService> pService);

    /** ***********************************************************
     * @brief       析构函数
     ************************************************************/
    ~CAppSystem();

    /** ***********************************************************
     * @brief       初始化应用框架：创建线程、注册回调、启动线程、初始化模块
     * @param[in]   无
     * @return      void
     ************************************************************/
    void IniAppFrame();

    /** ***********************************************************
     * @brief       设置指定线程的 Handler（接管所有权）
     * @param[in]   eThreadId      线程 id
     * @param[in]   pThreadHandler 线程 Handler 指针（所有权转移给 CAppSystem）
     * @return      void
     * @note        eThreadId 越界或 pThreadHandler 为空时直接返回
     ************************************************************/
    VOID SetThreadHandler(E_THREAD_ID eThreadId, CThreadHandler *pThreadHandler);

    /** ***********************************************************
     * @brief       每秒驱动事件（由系统线程周期调用）
     * @param[in]   无
     * @return      非零-成功，零-失败
     ************************************************************/
    INT32 DoSecEvent();

    /** ***********************************************************
     * @brief       注入数据缓存依赖
     * @param[in]   pCache 缓存实例的共享指针
     * @return      void
     ************************************************************/
    VOID SetCache(std::shared_ptr<IDataCache> pCache);

private:
    /** ***********************************************************
     * @brief       向持久化线程投递一条"保存笔记数据"任务
     * @param[in]   无
     * @return      void
     ************************************************************/
    void AddSaveDataTask();

    /** ***********************************************************
     * @brief       通过工厂批量创建线程与 Handler
     * @param[in]   无
     * @return      void
     ************************************************************/
    void CreateThread();

    /** ***********************************************************
     * @brief       获取指定线程 id 对应的 Handler
     * @param[in]   eThreadId 线程 id
     * @return      Handler 的共享指针；id 越界或未设置时返回 nullptr
     ************************************************************/
    shared_ptr<CThreadHandler> GetThreadHandler(E_THREAD_ID eThreadId);

    /** ***********************************************************
     * @brief       启动所有已创建的线程
     * @param[in]   无
     * @return      void
     ************************************************************/
    void StartThread();

    /** ***********************************************************
     * @brief       停止并回收所有线程与 Handler
     * @param[in]   无
     * @return      void
     * @note        先释放线程（触发 CThread 析构等待线程退出），
     *              再释放 Handler，避免 Handler 析构时等待仍在运行的线程
     ************************************************************/
    void StopThread();

    /** ***********************************************************
     * @brief       创建业务模块实例（预留扩展点）
     * @param[in]   无
     * @return      void
     ************************************************************/
    void CreateModule();

    /** ***********************************************************
     * @brief       系统初始化：向持久化线程投递"读取笔记数据"任务
     * @param[in]   无
     * @return      void
     ************************************************************/
    void InitSystem();

public:
    std::unique_ptr<CAppSystemPrivate>  d_ptr;
};
