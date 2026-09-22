/*
 * @file: CAppCompositionRoot.h
 * @brief: 应用启动引导器
 * @author: nuo
 * @date: 2026/6/5
 * @Detail: 负责应用的初始化、执行和关闭流程
 *          作为组合根(Composition Root)，集中创建和管理核心服务依赖关系
 *          确保程序启动时所需的核心服务在此处统一组装和注入
 */

#pragma once

#include <memory>

class QGuiApplication;
class QQmlApplicationEngine;
class CNoteDataCache;
class INoteDataBuffer;
class CDataRWMgr;
class CNoteDataService;
class CNoteBusiness;
class CMainNoteListViewModel;
class CSidebarModel;

class CAppBootstrapper
{
public:
    CAppBootstrapper();
    ~CAppBootstrapper();

    /** ***********************************************************
     * @brief       按层依次装配
     * @param[in]   argc 命令行参数数量
     * @param[in]   argv 命令行参数数组
     * @note        顺序：Qt → 配置层 → 数据层 → 领域层 → 表现层 → 线程与调度
     ************************************************************/
    void Compose(int& argc, char* argv[]);

    /** ***********************************************************
     * @brief       加载主界面并进入事件循环
     ************************************************************/
    void Run();

private:
    /** 配置层：CCommonConfig / CUserConfig / CConfigManager */
    void InitConfigLayer();

    /** 数据层：缓存、待写队列、文件读写器、数据服务，并注册到 ServiceLocator */
    void InitDataLayer();

    /** 领域层：CNoteBusiness（注入只读数据端口） */
    void InitBusinessLayer();

    /** 表现层：ViewModel 创建与 QML 上下文注入 */
    void InitViewModels();

    /** 表现层：配置变更回调与初始数据加载 */
    void InitUiCallbacks();

    /** 线程系统：CAppSystem（构造即创建并启动线程） */
    void InitThreadSystem();

    /** 调度：把业务挂到系统线程的秒事件上 */
    void RegisterSchedules();

private:
    int                                     m_argc = 0;
    char**                                  m_argv = nullptr;

    std::unique_ptr<QGuiApplication>        m_pApp;
    std::unique_ptr<QQmlApplicationEngine>  m_pEngine;

    std::shared_ptr<CNoteDataCache>         m_pNoteCache;
    std::shared_ptr<INoteDataBuffer>        m_pNoteCollect;
    std::shared_ptr<CDataRWMgr>             m_pDataSaverMgr;
    std::shared_ptr<CNoteDataService>       m_pNoteService;
    // std::shared_ptr<CNoteBusiness>          m_pNoteBusiness;

    std::unique_ptr<CMainNoteListViewModel> m_pNoteModel;
    std::unique_ptr<CSidebarModel>          m_pSidebarModel;
};