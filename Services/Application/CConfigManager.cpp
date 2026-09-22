/*
 * @file: CConfigManager.cpp
 * @brief: 配置管理应用服务实现
 * @detail:
 *
 */

#include "CConfigManager.h"
#include "IConfigBase.h"
#include "DConfig.h"
#include "SCategoryInfo.h"

#include <stdexcept>
#include <map>
#include <QDebug>

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

string CConfigManager::GetLanguage() const
{
    string rawValue;
    m_pCommonConfig->GetValue(SYSTEM_SECTION, COMMON_CONFIG_LANGUAGE, rawValue);
    return TranslateLanguageCode(rawValue);
}

string CConfigManager::GetTheme() const
{
    string value;
    m_pCommonConfig->GetValue(UI_SECTION, COMMON_CONFIG_THEME, value);
    return value.empty() ? "light" : value;
}

void CConfigManager::SetTheme(const string& strTheme)
{
    if (!IsValidTheme(strTheme))
    {
        throw invalid_argument("Invalid theme: " + strTheme + ". Must be 'light' or 'dark'");
    }

    SetSystemConfig(UI_SECTION, COMMON_CONFIG_THEME, strTheme);
}

bool CConfigManager::IsDebugMode() const
{
    string logLevel, env;
    
    m_pCommonConfig->GetValue(LOG_SECTION, COMMON_CONFIG_LOG_LEVEL, logLevel);
    m_pCommonConfig->GetValue(SYSTEM_SECTION, COMMON_CONFIG_ENVIRONMENT, env);
    
    return (logLevel == "DEBUG" || logLevel == "TRACE") 
           && (env == "development");
}

string CConfigManager::GetAppVersion() const
{
    string version;
    m_pCommonConfig->GetValue(SYSTEM_SECTION, COMMON_CONFIG_VERSION, version);
    return version.empty() ? "1.0.0" : version;
}

int CConfigManager::GetMaxRetryCount() const
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

int CConfigManager::GetTimeoutSeconds() const
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

string CConfigManager::GetDataPath() const
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

void CConfigManager::GetWindowPosition(int& nX, int& nY) const
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

    SetUserConfig(UICONTENT_SECTION, USER_CONFIG_WINDOW_POSITION_X, to_string(nX));
    SetUserConfig(UICONTENT_SECTION, USER_CONFIG_WINDOW_POSITION_Y, to_string(nY));

    string strPos = to_string(nX) + "," + to_string(nY);
    NotifyConfigChanged(E_BCFUN_TYPE_WINDOW_POS,
                        reinterpret_cast<PARAM>(strPos.c_str()));
}

void CConfigManager::GetWindowSize(int& nWidth, int& nHeight) const
{
    string strW, strH;

    m_pUserConfig->GetValue(UICONTENT_SECTION, USER_CONFIG_WINDOW_WIDTH, strW);
    m_pUserConfig->GetValue(UICONTENT_SECTION, USER_CONFIG_WINDOW_HEIGHT, strH);

    nWidth  = strW.empty() ? 600 : stoi(strW);
    nHeight = strH.empty() ? 400 : stoi(strH);
}

bool CConfigManager::GetWindowIsMaximized() const
{
    string value;
    m_pUserConfig->GetValue(UICONTENT_SECTION, USER_CONFIG_WINDOW_IS_MAXIMIZED, value);
    return value == "true" || value == "1";
}

string CConfigManager::GetBackgroundPath() const
{
    string path;
    m_pUserConfig->GetValue(UICONTENT_SECTION, USER_CONFIG_WINDOW_BACKGROUND_PATH, path);
    
    if (!path.empty() && path[0] != '/' && path[0] != '.')
    {
        return "./" + path;
    }
    
    return path;
}

vector<SCategoryInfo> CConfigManager::GetCategories() const
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

    bool b_IsSuccess = m_pUserConfig->AddCategory(info);

    E_BCFUN_TYPE eType = ShouldNotifyCallback(UICONTENT_SECTION);

    if (eType != E_BCFUN_TYPE_MAX && b_IsSuccess)
    {
        PARAM param = 0;
        m_pUserConfig->GetUIContentConfig(param);
        NotifyConfigChanged(eType, param);
    }

    return b_IsSuccess;
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

string CConfigManager::GetShortcut(const string& strAction) const
{
    string shortcut;
    string key = string(USER_CONFIG_SHORTCUTS_PREFIX) + strAction;
    m_pUserConfig->GetValue(UICONTENT_SECTION, key, shortcut);
    return shortcut;
}

void CConfigManager::SetShortcut(const string& strAction, const string& strKey)
{
    string fullKey = string(USER_CONFIG_SHORTCUTS_PREFIX) + strAction;
    SetUserConfig(UICONTENT_SECTION, fullKey, strKey);
}

// ========== 偏好设置 ==========

bool CConfigManager::IsAutoSaveEnabled() const
{
    string value;
    m_pUserConfig->GetValue(UICONTENT_SECTION, USER_CONFIG_PREFERENCES_AUTO_SAVE_ENABLED, value);
    
    return value == "true" || value == "1";
}

int CConfigManager::GetAutoSaveInterval() const
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

bool CConfigManager::RegisterCallback(E_BCFUN_TYPE eType, Func_ConfigChange func)
{
    if (eType >= E_BCFUN_TYPE_THEME && eType < E_BCFUN_TYPE_MAX)
    {
        m_puomapFunc[eType] = func;
        return true;
    }
    return false;
}

bool CConfigManager::UnregisterCallback(E_BCFUN_TYPE eType)
{
    if (eType >= E_BCFUN_TYPE_THEME && eType < E_BCFUN_TYPE_MAX)
    {
        m_puomapFunc.erase(eType);
        return true;
    }
    return false;
}

string CConfigManager::TranslateLanguageCode(const string& strRawCode) const
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

// ========== 私有方法实现 ==========

bool CConfigManager::IsValidTheme(const string& strTheme) const
{
    return strTheme == "light" || strTheme == "dark";
}

CConfigManager::E_BCFUN_TYPE CConfigManager::ShouldNotifyCallback(const string& strSection,
                                                   const string& strKey) const
{
    if (strSection == UI_SECTION && strKey == COMMON_CONFIG_THEME)
    {
        return E_BCFUN_TYPE_THEME;
    }
    if (strSection == UICONTENT_SECTION )
    {
        return E_BCFUN_TYPE_UICONFIG;
    }

    if (strSection == UICONTENT_SECTION &&
        (strKey == USER_CONFIG_WINDOW_POSITION_X ||
         strKey == USER_CONFIG_WINDOW_POSITION_Y))
    {
        return E_BCFUN_TYPE_WINDOW_POS;
    }

    if (strSection == UICONTENT_SECTION &&
        strKey.find(USER_CONFIG_SHORTCUTS_PREFIX) == 0)
    {
        return E_BCFUN_TYPE_SHORTCUT;
    }

    return E_BCFUN_TYPE_MAX;  // 不需要通知
}

void CConfigManager::NotifyConfigChanged(E_BCFUN_TYPE eType, PARAM param1, PARAM param2)
{
    auto it = m_puomapFunc.find(eType);

    if (it != m_puomapFunc.end() && it->second)
    {
        it->second(param1, param2);
    }
}

// ========== 统一配置设置接口实现 ==========

bool CConfigManager::SetSystemConfig(const string& strSection,
                                     const string& strKey,
                                     const string& strValue)
{
    if (strSection.empty() || strKey.empty())
    {
        return false;
    }

    try
    {
        m_pCommonConfig->SetValue(strSection, strKey, strValue);

        // 内部自动判断是否需要回调
        E_BCFUN_TYPE eType = ShouldNotifyCallback(strSection, strKey);

        if (eType != E_BCFUN_TYPE_MAX)
        {
            NotifyConfigChanged(eType, reinterpret_cast<PARAM>(strValue.c_str()));
        }

        return true;
    }
    catch (const exception& e)
    {
        qDebug() << "SetSystemConfig failed:" << e.what();
        return false;
    }
}

bool CConfigManager::SetUserConfig(const string& strSection,
                                   const string& strKey,
                                   const string& strValue)
{
    if (strSection.empty() || strKey.empty())
    {
        return false;
    }

    try
    {
        m_pUserConfig->SetValue(strSection, strKey, strValue);

        // 内部自动判断是否需要回调
        E_BCFUN_TYPE eType = ShouldNotifyCallback(strSection, strKey);

        if (eType != E_BCFUN_TYPE_MAX)
        {
            NotifyConfigChanged(eType, reinterpret_cast<PARAM>(strValue.c_str()));
        }

        return true;
    }
    catch (const exception& e)
    {
        qDebug() << "SetUserConfig failed:" << e.what();
        return false;
    }
}