/*
 * @file: CAppBootstrapper.cpp
 * @brief: 应用启动引导器实现
 * @author: nuo
 * @date: 2026/7/25
 */

#include "CAppBootstrapper.h"
#include <iostream>
#include <cstdlib>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "CAppSystem.h"
#include "CMainNoteListViewModel.h"
#include "CThreadFactory.h"
#include "CNoteDataService.h"
#include "CNoteDataCollect.h"
#include "CNoteDataCache.h"
#include "CServiceLocator.h"
#include "CDataSave.h"
#include "CDataRWMgr.h"

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

CAppBootstrapper::~CAppBootstrapper()
{
    Shutdown();
}

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
    InitializeAppSystem();

    m_bInitialized = true;
    cout << "[" << m_appName << "] 初始化完成！" << endl;
}

void CAppBootstrapper::Execute()
{
    cout << "[" << m_appName << "] 应用运行中..." << endl;
    RunApplicationLogic();
}

void CAppBootstrapper::Shutdown()
{
    if (!m_bInitialized)
        return;

    cout << "[" << m_appName << "] 正在关闭并释放资源..." << endl;

    m_services.noteCache.reset();
    m_services.noteService.reset();
    // m_services.dataSaver.reset();

    m_bInitialized = false;
    cout << "[" << m_appName << "] 已安全退出。" << endl;
}

void CAppBootstrapper::ParseCommandLine(int argc, char* argv[])
{
    if (argc > 0)
        m_appName = argv[0];

    cout << "命令行参数解析完成，数量：" << argc << endl;
}

void CAppBootstrapper::InitLogger()
{
    cout << "日志系统初始化完成" << endl;
}

void CAppBootstrapper::LoadConfiguration()
{
    cout << "配置文件加载完成" << endl;
}

void CAppBootstrapper::RegisterServices()
{
    cout << "开始注册核心服务..." << endl;

    try
    {
        auto noteCache = make_shared<CNoteDataCache>();
        auto noteCollect = make_shared<CNoteDataCollect>(NDataManager::NOTE_BUFFER_ITEM_COUNT_MAX * static_cast<INT32>(sizeof(NOTE_MODEL_ITEM)));

        m_services.noteService = make_shared<CNoteDataService>(noteCache, noteCollect);
        cout << "✓ CNoteDataService 创建完成（已注入缓存和待写队列实例）" << endl;

        auto dataSave = make_shared<CDataSave>(make_unique<NOTE_FILE_HEADER>());
        auto dataSaverMgr = make_shared<CDataRWMgr>(dataSave);
        g_ServiceLocator.RegisterDataSaver(dataSaverMgr);
        cout << "✓ CDataRWMgr 创建完成（已注入 DataSave 实例）" << endl;

        g_ServiceLocator.RegisterNoteCache(noteCache);
        g_ServiceLocator.RegisterNoteCollect(noteCollect);  // ← 注册到全局定位器
        g_ServiceLocator.RegisterNoteService(m_services.noteService);
        cout << "✓ 核心服务已注册到 ServiceLocator（全局可访问）" << endl;

        cout << "─────────────────────────────" << endl;
        cout << "应用层服务注册完成：" << endl;
        cout << "  • NoteCache:   ✓ (缓存 - 用于显示)" << endl;
        cout << "  • NoteCollect: ✓ (待写队列 - 供读写线程)" << endl;
        cout << "  • NoteService: ✓ (数据转换和分发)" << endl;
        cout << "  • DataSaver:   ✓ (文件写入 - 供异步线程)" << endl;
        cout << "─────────────────────────────" << endl;
    }
    catch (const exception& e)
    {
        cerr << " 服务注册失败：" << e.what() << endl;
        throw;
    }
}

void CAppBootstrapper::InitializeQtApplication(int argc, char* argv[])
{
    m_pApp = make_unique<QGuiApplication>(argc, argv);
    cout << "Qt 应用程序初始化完成" << endl;
}

void CAppBootstrapper::InitializeQmlEngine()
{
    m_pEngine = make_unique<QQmlApplicationEngine>();

    m_pNoteModel = make_unique<CMainNoteListViewModel>();

    m_pEngine->rootContext()->setContextProperty("noteModel", m_pNoteModel.get());

    QObject::connect(m_pEngine.get(), &QQmlApplicationEngine::objectCreationFailed,
                     m_pApp.get(), []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);

    cout << "QML 引擎初始化完成" << endl;
}

void CAppBootstrapper::RunApplicationLogic()
{
    cout << "========================================" << endl;
    cout << "        应用核心业务运行中...           " << endl;
    cout << "========================================" << endl;

    const QUrl url(u"qrc:/qt/qml/Widgets/MainWidget.qml"_qs);
    m_pEngine->load(url);

    if (m_pApp)
    {
        m_pApp->exec();
    }
}

void CAppBootstrapper::HandleFatalError(const char* errorMsg)
{
    cerr << "[" << m_appName << "] 致命错误：" << errorMsg << endl;
}

void CAppBootstrapper::InitializeAppSystem()
{
    g_CAppSystem->IniAppFrame();
}