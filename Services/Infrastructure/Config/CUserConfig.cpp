/*
 * @file: CUserConfig.cpp
 * @brief: 
 * @author: nuo
 * @date: 2026/8/27
 * @Detail:
 */

// ========== 系统头文件 ==========
#include <map>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

// ========== 项目头文件 ==========
#include "CUserConfig.h"
#include "SCategoryInfo.h"
#include "CommonDefine.h"
#include "DConfig.h"

using namespace std;
using namespace CPath;
using namespace NConfigSection;

static std::shared_mutex s_userConfigMutex;

class CUserConfigPrivate
{
    friend class CUserConfig;
public:
    explicit CUserConfigPrivate();
    ~CUserConfigPrivate() = default;

    void Json2Map(const QJsonObject &obj);
private:
    string m_strConfigPath; // 配置文件路径
    string m_strConfigName; // 配置文件名称
    vector<SCategoryInfo> m_vecCategoryInfo;

    map<string, map<string, string>> m_mapConfig;
};

CUserConfigPrivate::CUserConfigPrivate()
    : m_strConfigPath(STORE_PATH)
    , m_strConfigName(USER_CONFIG_FILE_NAME)
{
}

void CUserConfigPrivate::Json2Map(const QJsonObject &obj)
{
    m_mapConfig.clear();
    for (auto it = obj.begin(); it != obj.end(); ++it)
    {
        const string str_Section = it.key().toStdString();
        const QJsonValue value = it.value();
        if (str_Section == UICONTENT_SECTION && it.value().isObject())
        {
            QJsonObject objChild = it.value().toObject();
            for (auto itSecond = objChild.begin(); itSecond != objChild.end(); ++itSecond)
            {
                const string str_Key = itSecond.key().toStdString();
                if (str_Key == "noteCategories" && itSecond.value().isArray())
                {
                    QJsonArray arr = itSecond.value().toArray();
                    for (const auto& item: arr)
                    {
                        if (!item.isObject())
                        {
                            continue;
                        }
                        SCategoryInfo objInfo;
                        SCategoryInfo::FromJson(item.toObject(), objInfo);
                        m_vecCategoryInfo.push_back(objInfo);
                    }
                }
                else if (itSecond.value().isObject())
                {
                    QJsonObject sub_Obj = itSecond.value().toObject();
                    for (auto itThird = sub_Obj.begin();
                        itThird != sub_Obj.end(); ++itThird)
                    {
                        const string str_SubKey = itThird.key().toStdString();
                        const string str_SubValue = itThird.value().toString().toStdString();
                        m_mapConfig[str_Section][str_SubKey] = str_SubValue;
                    }
                }
            }
        }
        else if (value.isObject())
        {
            for (auto itSecond = value.toObject().begin();
                itSecond != value.toObject().end(); ++itSecond)
            {
                const string str_Key = itSecond.key().toStdString();
                const string str_Value = itSecond.value().toString().toStdString();
                m_mapConfig[str_Section][str_Key] = str_Value;
            }
        }
    }
}


CUserConfig::CUserConfig()
    : d_ptr(make_unique<CUserConfigPrivate>())
{
    InitConfig();
}

CUserConfig::~CUserConfig() = default;

void CUserConfig::GetUIContentConfig(vector<SCategoryInfo> &vecCategoryInfo) const
{
    std::shared_lock<std::shared_mutex> lock(s_userConfigMutex);
    vecCategoryInfo = d_ptr->m_vecCategoryInfo;
}

void CUserConfig::GetValue(const std::string &strFullKey, const std::string &strConfigKey, std::string &strConfigValue) const
{
    std::shared_lock<std::shared_mutex> lock(s_userConfigMutex);

    auto itSection = d_ptr->m_mapConfig.find(strFullKey);
    if (itSection == d_ptr->m_mapConfig.end())
    {
        return;
    }
    
    auto itKey = itSection->second.find(strConfigKey);
    if (itKey == itSection->second.end())
    {
        return;
    }
    
    strConfigValue = itKey->second;
}

bool CUserConfig::HasKey(const std::string &strSection, const std::string &strKey) const
{
    std::shared_lock<std::shared_mutex> lock(s_userConfigMutex);
    
    if (d_ptr->m_mapConfig.find(strSection) == d_ptr->m_mapConfig.end())
    {
        qDebug() << "CUserConfigPrivate::HasKey, config section not found";
        return false;
    }
    if (d_ptr->m_mapConfig[strSection].find(strKey) == d_ptr->m_mapConfig[strSection].end())
    {
        qDebug() << "CUserConfigPrivate::HasKey, config key not found";
        return false;
    }
    return true;
}

void CUserConfig::GetUserConfig(const string &strConfigKey, string &strConfigValue) const
{
    GetValue(UICONTENT_SECTION, strConfigKey, strConfigValue);
}

void CUserConfig::GetWindowConfig(const string &strConfigKey, string &strConfigValue) const
{
    GetValue(UICONTENT_SECTION, strConfigKey, strConfigValue);
}

void CUserConfig::GetShortcutConfig(const string &strConfigKey, string &strConfigValue) const
{
    GetValue(UICONTENT_SECTION, strConfigKey, strConfigValue);
}

void CUserConfig::GetPreferencesConfig(const string &strConfigKey, string &strConfigValue) const
{
    GetValue(UICONTENT_SECTION, strConfigKey, strConfigValue);
}

void CUserConfig::SetValue(const std::string &strSection, const std::string &strKey, const std::string &strValue)
{
    std::unique_lock<std::shared_mutex> lock(s_userConfigMutex);

    d_ptr->m_mapConfig[strSection][strKey] = strValue;
    Write2File();
}

bool CUserConfig::Write2File()
{
    QJsonObject rootobj;
    QJsonObject objChild;
    for (const auto &item: d_ptr->m_vecCategoryInfo)
    {
         QJsonArray arr;
         QJsonObject objItem;
         SCategoryInfo::ToJson(objItem, item);
         arr.append(objItem);
         objChild["noteCategories"] = arr;
    }
    rootobj[UICONTENT_SECTION] = objChild;
    QJsonObject json_Node;
    for (auto it = d_ptr->m_mapConfig.begin(); it != d_ptr->m_mapConfig.end(); ++it)
    {
        for (auto itSecond = it->second.begin(); itSecond != it->second.end(); ++itSecond)
        {
            json_Node[QString::fromStdString(itSecond->first)] = QString::fromStdString(itSecond->second);
        }
        rootobj[QString::fromStdString(it->first)] = json_Node;
    }
    
    string str_FullPath = d_ptr->m_strConfigPath + d_ptr->m_strConfigName;
    QFile file(QString::fromStdString(str_FullPath));
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
    {
        qDebug() << "CUserConfig::Write2File, open config file failed";
        return false;
    }
    file.write(QJsonDocument(rootobj).toJson());
    file.close();
    return true;
}

bool CUserConfig::AddValue(const std::string &strSection, const std::string &strKey, const std::string &strValue)
{
    std::unique_lock<std::shared_mutex> lock(s_userConfigMutex);

    auto itSection = d_ptr->m_mapConfig.find(strSection);
    if (itSection != d_ptr->m_mapConfig.end())
    {
        if (itSection->second.find(strKey) != itSection->second.end())
        {
            return false;
        }
    }
    
    d_ptr->m_mapConfig[strSection][strKey] = strValue;
    Write2File();
    return true;
}

bool CUserConfig::DeleteValue(const std::string &strSection, const std::string &strKey)
{
    std::unique_lock<std::shared_mutex> lock(s_userConfigMutex);

    auto itSection = d_ptr->m_mapConfig.find(strSection);
    if (itSection == d_ptr->m_mapConfig.end())
    {
        return false;
    }
    
    auto itKey = itSection->second.find(strKey);
    if (itKey == itSection->second.end())
    {
        return false;
    }
    
    d_ptr->m_mapConfig[strSection].erase(itKey);
    Write2File();
    return true;
}


void CUserConfig::InitConfig()
{
    std::unique_lock<std::shared_mutex> lock(s_userConfigMutex);
    
    string str_FullPath = d_ptr->m_strConfigPath + d_ptr->m_strConfigName;

    QFile file(QString::fromStdString(str_FullPath));
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qWarning() << "打开配置文件失败：" << file.errorString();
        return ;
    }
    QByteArray data= file.readAll();
    file.close();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError)
    {
        qWarning() << error.errorString();
        return ;
    }
    if (!doc.isObject())
    {
        qWarning() << "json root fail!";
        return ;
    }
    QJsonObject obj = doc.object();
    d_ptr->Json2Map(obj);

}

// ========== 实现分类相关接口方法 ==========

void CUserConfig::GetCategories(vector<SCategoryInfo> &vecCategories)
{
    std::shared_lock<std::shared_mutex> lock(s_userConfigMutex);
    vecCategories = d_ptr->m_vecCategoryInfo;
}

bool CUserConfig::AddCategory(const SCategoryInfo &categoryInfo)
{
    std::unique_lock<std::shared_mutex> lock(s_userConfigMutex);
    
    for (const auto& cat : d_ptr->m_vecCategoryInfo)
    {
        if (cat.strName == categoryInfo.strName)
        {
            return false;
        }
    }
    
    d_ptr->m_vecCategoryInfo.push_back(categoryInfo);
    Write2File();
    
    return true;
}

bool CUserConfig::DeleteCategory(const string& strName)
{
    std::unique_lock<std::shared_mutex> lock(s_userConfigMutex);

    for (auto it = d_ptr->m_vecCategoryInfo.begin();
         it != d_ptr->m_vecCategoryInfo.end(); ++it)
    {
        if (it->strName == strName)
        {
            if (it->bIsSystem)
            {
                return false;
            }

            d_ptr->m_vecCategoryInfo.erase(it);
            Write2File();

            return true;
        }
    }

    return false;
}

void CUserConfig::GetUIContentConfig(PARAM &param)
{
    vector<SCategoryInfo> vecCategories;
    GetUIContentConfig(vecCategories);
    param = reinterpret_cast<PARAM>(&vecCategories);
}