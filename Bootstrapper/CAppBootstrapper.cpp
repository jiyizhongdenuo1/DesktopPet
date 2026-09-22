/*
 * @file: CAppCompositionRoot.cpp
 * @brief: 应用启动引导器实现
 * @author: nuo
 * @date: 2026/7/25
 * @Detail:
 */

#include <iostream>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

#include "CAppBootstrapper.h"
#include "CAppSystem.h"
#include "CCommonConfig.h"
#include "CUserConfig.h"
#include "CConfigManager.h"
#include "CNoteDataCache.h"
#include "CNoteDataCollect.h"
#include "INoteDataBuffer.h"
#include "CNoteDataService.h"
#include "CDataSave.h"
#include "CDataRWMgr.h"
#include "CServiceLocator.h"
// #include "CNoteBusiness.h"
#include "CMainNoteListViewModel.h"
#include "CSidebarModel.h"
#include "SCategoryInfo.h"

using namespace std;

CAppBootstrapper::CAppBootstrapper()
{
}

CAppBootstrapper::~CAppBootstrapper() = default;

void CAppBootstrapper::Compose(int& argc, char* argv[])
{
    m_pApp = make_unique<QGuiApplication>(argc, argv);
    cout << "Qt 应用程序初始化完成" << endl;

    InitConfigLayer();
    InitDataLayer();
    InitBusinessLayer();
    InitViewModels();
    InitUiCallbacks();
    InitThreadSystem();
    RegisterSchedules();
}

void CAppBootstrapper::InitConfigLayer()
{
    auto pCommonConfig = make_shared<CCommonConfig>();
    auto pUserConfig   = make_shared<CUserConfig>();
    CConfigManager::Initialize(pCommonConfig, pUserConfig);

    cout << "配置层装配完成" << endl;
}

void CAppBootstrapper::InitDataLayer()
{
    m_pNoteCache = make_shared<CNoteDataCache>();

    m_pNoteCollect = make_shared<CNoteDataCollect>(
        NDataManager::NOTE_BUFFER_ITEM_COUNT_MAX * static_cast<INT32>(sizeof(NOTE_MODEL_ITEM)));

    auto pDataSave = make_shared<CDataSave>(make_unique<NOTE_FILE_HEADER>());
    m_pDataSaverMgr = make_shared<CDataRWMgr>(pDataSave, g_ConfigManager->GetDataPath());

    m_pNoteService = make_shared<CNoteDataService>(m_pNoteCache, m_pNoteCollect);

    g_ServiceLocator.RegisterNoteCache(m_pNoteCache);
    g_ServiceLocator.RegisterNoteCollect(m_pNoteCollect);
    g_ServiceLocator.RegisterDataSaver(m_pDataSaverMgr);
    g_ServiceLocator.RegisterNoteService(m_pNoteService);

    cout << "数据层装配完成" << endl;
}

void CAppBootstrapper::InitBusinessLayer()
{
    // m_pNoteBusiness = make_shared<CNoteBusiness>(m_pNoteCache);

    cout << "领域层装配完成" << endl;
}

void CAppBootstrapper::InitViewModels()
{
    m_pEngine = make_unique<QQmlApplicationEngine>();

    m_pNoteModel = make_unique<CMainNoteListViewModel>();
    m_pEngine->rootContext()->setContextProperty("noteModel", m_pNoteModel.get());

    m_pSidebarModel = make_unique<CSidebarModel>();
    m_pEngine->rootContext()->setContextProperty("sidebarModel", m_pSidebarModel.get());

    QObject::connect(m_pEngine.get(), &QQmlApplicationEngine::objectCreationFailed,
                     m_pApp.get(), []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);
}

void CAppBootstrapper::InitUiCallbacks()
{
    if (!m_pSidebarModel)
    {
        return;
    }

    g_ConfigManager->RegisterCallback(
        CConfigManager::E_BCFUN_TYPE_UICONFIG,
        std::bind(
            static_cast<bool(CSidebarModel::*)(PARAM, PARAM)>(&CSidebarModel::UpdateSidebarItems),
            m_pSidebarModel.get(),
            std::placeholders::_1,
            std::placeholders::_2
        )
    );

    vector<SCategoryInfo> categories = g_ConfigManager->GetCategories();

    if (!categories.empty())
    {
        static vector<SCategoryInfo> staticCategories = categories;
        PARAM param = reinterpret_cast<PARAM>(&staticCategories);

        m_pSidebarModel->UpdateSidebarItems(param, 0);
        qDebug() << "侧边栏初始加载完成，分类数:" << m_pSidebarModel->rowCount();
    }
    else
    {
        qWarning() << "侧边栏初始加载失败：无分类数据";
    }
}

void CAppBootstrapper::InitThreadSystem()
{
    g_CAppSystem;
}

void CAppBootstrapper::RegisterSchedules()
{
    // auto pBusiness = m_pNoteBusiness;

    // g_CAppSystem->RegisterSecEvent([pBusiness]()
    // {
    //     pBusiness->DoSecend();
    // });
}

void CAppBootstrapper::Run()
{
    const QUrl url(u"qrc:/qt/qml/Widgets/MainWidget.qml"_qs);
    m_pEngine->load(url);

    if (m_pApp)
    {
        m_pApp->exec();
    }
}