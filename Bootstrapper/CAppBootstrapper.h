/*
 * @file: CAppBootstrapper.h
 * @brief: 应用启动引导器
 * @author: nuo
 * @date: 2026/6/5
 * @Detail: 负责应用的初始化、执行和关闭流程
 */

#pragma once

#include <memory>

class QGuiApplication;
class QQmlApplicationEngine;
class CMainNoteListViewModel;

class CAppBootstrapper
{
public:
    // 单例
    static CAppBootstrapper& Instance();

    // 禁用拷贝
    CAppBootstrapper(const CAppBootstrapper&) = delete;
    CAppBootstrapper& operator=(const CAppBootstrapper&) = delete;

    // 启动入口
    int Run(int argc, char* argv[]);

private:
    // 私有构造/析构
    CAppBootstrapper();
    ~CAppBootstrapper();

    // 生命周期
    void Initialize(int argc, char* argv[]);
    void Execute();
    void Shutdown();

    // 子功能
    void ParseCommandLine(int argc, char* argv[]);
    void InitLogger();
    void LoadConfiguration();
    void RegisterServices();
    void InitializeQtApplication(int argc, char* argv[]);
    void InitializeQmlEngine();
    void RunApplicationLogic();
    void HandleFatalError(const char* errorMsg);
    void InitializeAppSystem();

private:
    bool                                    m_bInitialized;
    const char*                             m_appName;
    std::unique_ptr<QGuiApplication>        m_pApp;
    std::unique_ptr<QQmlApplicationEngine>  m_pEngine;
    std::unique_ptr<CMainNoteListViewModel> m_pNoteModel;
};
#define  g_CAppBootstrapper CAppBootstrapper::Instance()