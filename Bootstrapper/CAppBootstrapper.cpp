/*
 * @file: CAppBootstrapper.cpp
 * @brief: 应用启动引导器实现
 * @author: nuo
 * @date: 2026/6/5
 * @Detail: 负责应用的初始化、执行和关闭流程
 */

#include "CAppBootstrapper.h"
#include <iostream>
#include <cstdlib>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDateTime>
#include "CAppSystem.h"
#include "CMainNoteListViewModel.h"
#include "CThreadFactory.h"

using namespace std;
CAppBootstrapper& CAppBootstrapper::Instance()
{
    static CAppBootstrapper s_instance;
    return s_instance;
}

CAppBootstrapper::CAppBootstrapper()
    : m_bInitialized(false)
    , m_appName("DefaultApp")
    , m_pApp(nullptr)
    , m_pEngine(nullptr)
    , m_pNoteModel(nullptr)
{
}

// 析构
CAppBootstrapper::~CAppBootstrapper()
{
    Shutdown();
}

// 启动入口
int CAppBootstrapper::Run(int argc, char* argv[])
{
    try
    {
        Initialize(argc, argv);
        Execute();
        return EXIT_SUCCESS;
    }
    catch (const exception& ex)
    {
        HandleFatalError(ex.what());
        return EXIT_FAILURE;
    }
}

// 初始化
void CAppBootstrapper::Initialize(int argc, char* argv[])
{
    if (m_bInitialized)
        return;

    cout << "[" << m_appName << "] 开始初始化..." << endl;

    ParseCommandLine(argc, argv);
    InitLogger();
    LoadConfiguration();
    RegisterServices();
    InitializeQtApplication(argc, argv);
    InitializeQmlEngine();

    m_bInitialized = true;
    cout << "[" << m_appName << "] 初始化完成！" << endl;
}

// 执行主逻辑
void CAppBootstrapper::Execute()
{
    cout << "[" << m_appName << "] 应用运行中..." << endl;
    RunApplicationLogic();
}

// 关闭
void CAppBootstrapper::Shutdown()
{
    if (!m_bInitialized)
        return;

    cout << "[" << m_appName << "] 正在关闭并释放资源..." << endl;
    m_bInitialized = false;
    cout << "[" << m_appName << "] 已安全退出。" << endl;
}

// 解析命令行
void CAppBootstrapper::ParseCommandLine(int argc, char* argv[])
{
    if (argc > 0)
        m_appName = argv[0];

    cout << "命令行参数解析完成，数量：" << argc << endl;
}

// 日志初始化
void CAppBootstrapper::InitLogger()
{
    cout << "日志系统初始化完成" << endl;
}

// 加载配置
void CAppBootstrapper::LoadConfiguration()
{
    cout << "配置文件加载完成" << endl;
}

// 注册服务
void CAppBootstrapper::RegisterServices()
{
    cout << "服务注册完成" << endl;
}

// 初始化 Qt 应用
void CAppBootstrapper::InitializeQtApplication(int argc, char* argv[])
{
    m_pApp = make_unique<QGuiApplication>(argc, argv);
    cout << "Qt 应用程序初始化完成" << endl;
}

// 初始化 QML 引擎
void CAppBootstrapper::InitializeQmlEngine()
{
    m_pEngine = make_unique<QQmlApplicationEngine>();
    m_pNoteModel = make_unique<CMainNoteListViewModel>();

    // 添加初始笔记
    m_pNoteModel->AddNote("欢迎使用 DesktopPet!", QDateTime::currentDateTime());
    m_pNoteModel->AddNote("点击我可以改变状态哦~", QDateTime::currentDateTime());

    // 设置上下文属性
    m_pEngine->rootContext()->setContextProperty("noteModel", m_pNoteModel.get());

    // 连接信号，防止加载失败静默退出
    QObject::connect(m_pEngine.get(), &QQmlApplicationEngine::objectCreationFailed,
                     m_pApp.get(), []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);

    cout << "QML 引擎初始化完成" << endl;
}

// 业务逻辑
void CAppBootstrapper::RunApplicationLogic()
{
    cout << "========================================" << endl;
    cout << "        应用核心业务运行中...           " << endl;
    cout << "========================================" << endl;

    // 加载 QML 文件
    const QUrl url(u"qrc:/qt/qml/Widgets/MainWidget.qml"_qs);
    m_pEngine->load(url);

    // 执行 Qt 事件循环
    if (m_pApp) {
        m_pApp->exec();
    }
}

// 错误处理
void CAppBootstrapper::HandleFatalError(const char* errorMsg)
{
    cerr << "[" << m_appName << "] 致命错误：" << errorMsg << endl;
}

void CAppBootstrapper::InitializeAppSystem()
{
    // g_CAppSystem->InitAppFrame();
}