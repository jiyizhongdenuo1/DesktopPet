/*
 * @file: CConfigManager.cpp
 * @brief: 配置管理应用服务实现
 * @detail: 
 *   持有 IConfigUpdater 和 IConfigEditor 基类接口（依赖倒置原则）
 *   通过基类接口访问配置，不依赖具体实现类
 *   使用 DConfig.h 中定义的配置常量，避免硬编码字符串
 * 
 * 使用方式：
 *   1. 程序启动时调用 Initialize() 注入配置实例
 *   2. 之后通过 GetInstance() 或 g_ConfigManager 宏使用
 */

#include "CConfigManager.h"
#include "IConfigBase.h"
#include "DConfig.h"

#include <stdexcept>
#include <map>

using namespace std;
using namespace NConfigSection;
using namespace NCommonConfig;
using namespace NUserConfig;

shared_ptr<CConfigManager> CConfigManager::s_pInstance = nullptr;

void CConfigManager::Initialize(
    shared_ptr<IConfigUpdater> pCommonConfig,
    shared_ptr<IConfigEditor> pUserConfig)
{
    if (!pCommonConfig || !pUserConfig)
    {
        throw invalid_argument("Config instances cannot be null");
    }
    
    if (s_pInstance == nullptr)
    {
        s_pInstance = make_shared<CConfigManager>(pCommonConfig, pUserConfig);
    }
}

shared_ptr<CConfigManager> CConfigManager::GetInstance()
{
    if (s_pInstance == nullptr)
    {
        throw runtime_error("ConfigManager not initialized. Call Initialize() first with valid config instances");
    }
    return s_pInstance;
}

CConfigManager::CConfigManager(
    shared_ptr<IConfigUpdater> pCommonConfig,
    shared_ptr<IConfigEditor> pUserConfig)
    : m_pCommonConfig(pCommonConfig)
    , m_pUserConfig(pUserConfig)
{
    if (!m_pCommonConfig || !m_pUserConfig)
    {
        throw invalid_argument("Config instances cannot be null");
    }
}

CConfigManager::~CConfigManager() = default;

// ========== 系统配置实现（通过 IConfigUpdater 基类接口）==========

string CConfigManager::GetLanguage()
{
    string rawValue;
    m_pCommonConfig->GetValue(SYSTEM_SECTION, COMMON_CONFIG_LANGUAGE, rawValue);
    return TranslateLanguageCode(rawValue);
}

string CConfigManager::GetTheme()
{
    string value;
    m_pCommonConfig->GetValue(UI_SECTION, COMMON_CONFIG_THEME, value);
    return value.empty() ? "light" : value;
}

void CConfigManager::SetTheme(const string& strTheme)
{
    ValidateTheme(strTheme);
    m_pCommonConfig->SetValue(UI_SECTION, COMMON_CONFIG_THEME, strTheme);
    NotifyThemeChanged(strTheme);
}

bool CConfigManager::IsDebugMode()
{
    string logLevel, env;
    
    m_pCommonConfig->GetValue(LOG_SECTION, COMMON_CONFIG_LOG_LEVEL, logLevel);
    m_pCommonConfig->GetValue(SYSTEM_SECTION, COMMON_CONFIG_ENVIRONMENT, env);
    
    return (logLevel == "DEBUG" || logLevel == "TRACE") 
           && (env == "development");
}

string CConfigManager::GetAppVersion()
{
    string version;
    m_pCommonConfig->GetValue(SYSTEM_SECTION, COMMON_CONFIG_VERSION, version);
    return version.empty() ? "1.0.0" : version;
}

int CConfigManager::GetMaxRetryCount()
{
    string value;
    m_pCommonConfig->GetValue(BUSINESS_SECTION, COMMON_CONFIG_MAX_RETRY_COUNT, value);
    
    try
    {
        return stoi(value);
    }
    catch (...)
    {
        return 3;
    }
}

int CConfigManager::GetTimeoutSeconds()
{
    string value;
    m_pCommonConfig->GetValue(BUSINESS_SECTION, COMMON_CONFIG_TIMEOUT_SECONDS, value);
    
    try
    {
        return stoi(value);
    }
    catch (...)
    {
        return 30;
    }
}

string CConfigManager::GetDataPath()
{
    string strDataPath;
    m_pCommonConfig->GetValue(DATA_SECTION, COMMON_CONFIG_DATA_PATH, strDataPath);
    
    if (strDataPath.empty())
    {
        return "./data/";
    }
    
    if (strDataPath.back() != '/')
    {
        strDataPath += '/';
    }
    
    return strDataPath;
}

// ========== 用户配置实现（通过 IConfigEditor 基类接口）==========

void CConfigManager::GetWindowPosition(int& nX, int& nY)
{
    string strX, strY;
    
    m_pUserConfig->GetValue(UICONTENT_SECTION, USER_CONFIG_WINDOW_POSITION_X, strX);
    m_pUserConfig->GetValue(UICONTENT_SECTION, USER_CONFIG_WINDOW_POSITION_Y, strY);
    
    nX = strX.empty() ? 100 : stoi(strX);
    nY = strY.empty() ? 100 : stoi(strY);
}

void CConfigManager::SetWindowPosition(int nX, int nY)
{
    if (nX < 0 || nY < 0)
    {
        throw out_of_range("Window position cannot be negative");
    }
    
    m_pUserConfig->SetValue(UICONTENT_SECTION, USER_CONFIG_WINDOW_POSITION_X, to_string(nX));
    m_pUserConfig->SetValue(UICONTENT_SECTION, USER_CONFIG_WINDOW_POSITION_Y, to_string(nY));
}

string CConfigManager::GetBackgroundPath()
{
    string path;
    m_pUserConfig->GetValue(UICONTENT_SECTION, USER_CONFIG_WINDOW_BACKGROUND_PATH, path);
    
    if (!path.empty() && path[0] != '/' && path[0] != '.')
    {
        return "./" + path;
    }
    
    return path;
}

vector<SCategoryInfo> CConfigManager::GetCategories()
{
    vector<SCategoryInfo> categories;
    m_pUserConfig->GetCategories(categories);
    return categories;
}

bool CConfigManager::AddCategory(const SCategoryInfo& info)
{
    auto existingCategories = GetCategories();
    
    for (const auto& cat : existingCategories)
    {
        if (cat.strName == info.strName)
        {
            return false;
        }
    }
    
    return m_pUserConfig->AddCategory(info);
}

bool CConfigManager::DeleteCategory(const string& strName)
{
    auto existingCategories = GetCategories();
    
    for (const auto& cat : existingCategories)
    {
        if (cat.strName == strName && cat.bIsSystem)
        {
            return false;  // 不允许删除系统分类
        }
    }
    
    return m_pUserConfig->DeleteCategory(strName);
}

string CConfigManager::GetShortcut(const string& strAction)
{
    string shortcut;
    string key = string(USER_CONFIG_SHORTCUTS_PREFIX) + strAction;
    m_pUserConfig->GetValue(UICONTENT_SECTION, key, shortcut);
    return shortcut;
}

void CConfigManager::SetShortcut(const string& strAction, const string& strKey)
{
    string fullKey = string(USER_CONFIG_SHORTCUTS_PREFIX) + strAction;
    m_pUserConfig->SetValue(UICONTENT_SECTION, fullKey, strKey);
}

// ========== 偏好设置 ==========

bool CConfigManager::IsAutoSaveEnabled()
{
    string value;
    m_pUserConfig->GetValue(UICONTENT_SECTION, USER_CONFIG_PREFERENCES_AUTO_SAVE_ENABLED, value);
    
    return value == "true" || value == "1";
}

int CConfigManager::GetAutoSaveInterval()
{
    string value;
    m_pUserConfig->GetValue(UICONTENT_SECTION, USER_CONFIG_PREFERENCES_AUTO_SAVE_INTERVAL_SECONDS, value);
    
    try
    {
        return stoi(value);
    }
    catch (...)
    {
        return 30;
    }
}

// ========== 私有辅助方法 ==========

string CConfigManager::TranslateLanguageCode(const string& strRawCode)
{
    static map<string, string> translationMap = {
        {"zh_CN", "简体中文"},
        {"zh_TW", "繁體中文"},
        {"en_US", "English"},
        {"ja_JP", "日本語"},
        {"ko_KR", "한국어"}
    };
    
    auto it = translationMap.find(strRawCode);
    if (it != translationMap.end())
    {
        return it->second;
    }
    
    return strRawCode;
}

void CConfigManager::ValidateTheme(const string& strTheme)
{
    if (strTheme != "light" && strTheme != "dark")
    {
        throw invalid_argument("Invalid theme: " + strTheme + ". Must be 'light' or 'dark'");
    }
}

void CConfigManager::NotifyThemeChanged(const string& strTheme)
{
    // TODO: 可扩展为观察者模式，通知UI层刷新主题
     
}