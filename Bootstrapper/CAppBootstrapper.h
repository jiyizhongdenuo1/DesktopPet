/*
 * @file: CAppBootstrapper.h
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
class CMainNoteListViewModel;
class CNoteDataService;
class CDataRWMgr;
class CNoteDataCache;

class CAppBootstrapper
{
public:
    /** ***********************************************************
     * @brief       核心服务注册表结构体
     * @note        存储应用程序启动时必须初始化的所有核心服务实例
     *              作为组合根的服务容器，统一管理服务生命周期
     ************************************************************/
    struct ServiceRegistry
    {
        std::shared_ptr<CNoteDataCache>       noteCache;     ///< 笔记数据缓存（无锁读取，容量500个）
        std::shared_ptr<CNoteDataService>   noteService;   ///< 笔记数据服务（封装缓存策略与文件读写）
        std::shared_ptr<CDataRWMgr>      dataSaver;    ///< 数据保存服务（使用共享DataSave实例）
    };

    /** ***********************************************************
     * @brief       获取单例实例
     * @param[in]   无
     * @return      应用启动引导器的引用
     * @note        线程安全的单例模式实现
     ************************************************************/
    static CAppBootstrapper& Instance();

    /** ***********************************************************
     * @brief       启动入口
     * @param[in]   argc 命令行参数数量
     * @param[in]   argv 命令行参数数组
     * @return      应用退出码（EXIT_SUCCESS 或 EXIT_FAILURE）
     * @note        统一管理整个应用的生命周期，捕获所有未处理异常
     ************************************************************/
    int Run(int argc, char* argv[]);

    /** ***********************************************************
     * @brief       获取核心服务注册表
     * @param[in]   无
     * @return      const 引用，防止外部意外修改服务实例
     * @note        提供对已注册核心服务的只读访问接口
     ************************************************************/
    const ServiceRegistry& GetServices() const { return m_services; }

    /** ***********************************************************
     * @brief       获取 NoteService 实例（便捷方法）
     * @param[in]   无
     * @return      笔记数据服务的共享指针
     * @note        供外部快速访问笔记数据服务
     ************************************************************/
    std::shared_ptr<CNoteDataService> GetNoteService() const { return m_services.noteService; }

    /** ***********************************************************
     * @brief       获取 DataSaver 实例（便捷方法）
     * @param[in]   无
     * @return      数据保存服务的共享指针
     * @note        供外部快速访问数据保存服务
     ************************************************************/
    std::shared_ptr<CDataRWMgr> GetDataSaver() const { return m_services.dataSaver; }

    /** ***********************************************************
     * @brief       获取 NoteCache 实例（便捷方法）
     * @param[in]   无
     * @return      笔记数据缓存的共享指针
     * @note        供外部快速访问缓存服务
     ************************************************************/
    std::shared_ptr<CNoteDataCache> GetNoteCache() const { return m_services.noteCache; }

private:
    CAppBootstrapper();
    ~CAppBootstrapper();

    /** ***********************************************************
     * @brief       初始化函数
     * @param[in]   argc 命令行参数数量
     * @param[in]   argv 命令行参数数组
     * @note        按照正确的依赖顺序依次初始化各子系统
     *              核心服务在RegisterServices()中集中创建，确保依赖关系正确
     ************************************************************/
    void Initialize(int argc, char* argv[]);

    /** ***********************************************************
     * @brief       执行主逻辑
     * @param[in]   无
     * @return      void
     * @note        加载QML界面并进入事件循环
     ************************************************************/
    void Execute();

    /** ***********************************************************
     * @brief       关闭函数
     * @param[in]   无
     * @return      void
     * @note        安全释放所有资源，重置初始化状态
     ************************************************************/
    void Shutdown();

    /** ***********************************************************
     * @brief       解析命令行参数
     * @param[in]   argc 参数数量
     * @param[in]   argv 参数数组
     * @note        提取应用程序名称用于日志标识
     ************************************************************/
    void ParseCommandLine(int argc, char* argv[]);

    /** ***********************************************************
     * @brief       日志系统初始化
     * @param[in]   无
     * @return      void
     * @note        预留接口，未来可集成具体日志框架
     ************************************************************/
    void InitLogger();

    /** ***********************************************************
     * @brief       加载配置文件
     * @param[in]   无
     * @return      void
     * @note        预留接口，未来可从配置文件读取应用设置
     ************************************************************/
    void LoadConfiguration();

    /** ***********************************************************
     * @brief       注册核心服务（⭐ 组合根的核心功能）
     * @param[in]   无
     * @return      void
     * @note        集中创建和管理应用程序的所有核心服务实例
     *              按照依赖顺序从底层到上层依次创建：
     *              1. 缓存层：NoteDataCache
     *              2. 数据访问层：CNoteDataService（注入缓存）
     *              3. 业务逻辑层：CDataRWMgr（使用共享DataSave实例）
     *
     *              关键设计决策：
     *              - 所有服务通过shared_ptr管理生命周期
     *              - RWMgr实例通过CNoteDataService统一获取，确保唯一性
     *              - 服务创建失败会立即抛出异常，避免运行时空指针
     ************************************************************/
    void RegisterServices();

    /** ***********************************************************
     * @brief       初始化 Qt 应用程序
     * @param[in]   argc 命令行参数数量
     * @param[in]   argv 命令行参数数组
     * @note        创建QGuiApplication实例，作为Qt框架的入口点
     ************************************************************/
    void InitializeQtApplication(int argc, char* argv[]);

    /** ***********************************************************
     * @brief       初始化 QML 引擎和 ViewModel
     * @param[in]   无
     * @return      void
     * @note        创建QML引擎和主界面ViewModel
     *              将ViewModel注册为QML上下文属性，供QML界面访问
     *              未来可通过GetServices()向ViewModel注入已创建的服务
     ************************************************************/
    void InitializeQmlEngine();

    /** ***********************************************************
     * @brief       运行应用主逻辑
     * @param[in]   无
     * @return      void
     * @note        加载QML主界面并进入Qt事件循环
     *              这是应用程序的主循环，直到窗口关闭才返回
     ************************************************************/
    void RunApplicationLogic();

    /** ***********************************************************
     * @brief       处理致命错误
     * @param[in]   errorMsg 错误信息
     * @param[out]  无
     * @return      void
     * @note        输出错误详情并返回失败退出码
     ************************************************************/
    void HandleFatalError(const char* errorMsg);

    /** ***********************************************************
     * @brief       初始化应用系统框架
     * @param[in]   无
     * @return      void
     * @note        启动CAppSystem框架，初始化线程池等基础设施
     ************************************************************/
    void InitializeAppSystem();

private:
    bool                                    m_bInitialized;
    const char*                             m_appName;
    ServiceRegistry                         m_services;      ///< 核心服务注册表（组合根容器）
    std::unique_ptr<QGuiApplication>        m_pApp;
    std::unique_ptr<QQmlApplicationEngine>  m_pEngine;
    std::unique_ptr<CMainNoteListViewModel> m_pNoteModel;
};
#define  g_CAppBootstrapper CAppBootstrapper::Instance()