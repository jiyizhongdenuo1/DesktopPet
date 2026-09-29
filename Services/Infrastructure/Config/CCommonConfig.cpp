/*
 * @file: CCommonConfig.cpp
 * @brief: 
 * @author: nuo
 * @date: 2026/6/14
 * @Detail:
 */

// ========== 系统头文件 ==========
#include <map>
#include <mutex>
#include <shared_mutex>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>

// ========== 项目头文件 ==========
#include "CCommonConfig.h"
#include "CommonDefine.h"
#include "DConfig.h"

using namespace CPath;
using namespace std;
using namespace NConfigSection;

static std::shared_mutex s_configMutex;

class CCommonConfigPrivate
{
public:
    friend class CCommonConfig;
    explicit CCommonConfigPrivate();
    ~CCommonConfigPrivate()
    {

    }
    void Json2Map(const QJsonObject &rootObj);
    void GetConfig(const string &strConfigSection, const string &strConfigKey, string &strConfigValue) const;

public:
    string m_strConfigPath;
    string m_strConfigFileName;

    map<string, map<string, string>> m_mapConfigSection;
};

void CCommonConfigPrivate::Json2Map(const QJsonObject &rootObj)
{
    std::shared_lock<std::shared_mutex> lock(s_configMutex);
    
    m_mapConfigSection.clear();
    for (auto it = rootObj.begin(); it != rootObj.end(); ++it)
    {
        const string str_Section = it.key().toStdString();
        const QJsonValue value = it.value();
        if (value.isObject())
        {
            const QJsonObject obj = value.toObject();
            for (auto itSecond = obj.begin(); itSecond != obj.end(); ++itSecond)
            {
                const string str_Key = itSecond.key().toStdString();
                const string str_Value = itSecond.value().toString().toStdString();
                m_mapConfigSection[str_Section][str_Key] = str_Value;
            }
        }
    }
}

CCommonConfigPrivate::CCommonConfigPrivate()
    : m_strConfigPath(STORE_PATH)
    , m_strConfigFileName(COMMON_COFNIG_FILE_NAME)
{

}

void CCommonConfigPrivate::GetConfig(const string &strConfigSection, const string &strConfigKey,
    string &strConfigValue) const
{
    std::shared_lock<std::shared_mutex> lock(s_configMutex);
    
    auto it_Section = m_mapConfigSection.find(strConfigSection);
    if (it_Section == m_mapConfigSection.end())
    {
        qDebug() << "CCommonConfigPrivate::GetConfig, config section not found";
        return;
    }
    
    auto it_Key = it_Section->second.find(strConfigKey);
    if (it_Key == it_Section->second.end())
    {
        qDebug() << "CCommonConfigPrivate::GetConfig, config key not found";
        return;
    }
    
    strConfigValue = it_Key->second;
}

CCommonConfig::CCommonConfig()
    : d_ptr(make_unique<CCommonConfigPrivate>())
{
    InitConfig();
}

CCommonConfig::~CCommonConfig()
{

}

void CCommonConfig::InitConfig()
{
    string str_FullPath = d_ptr->m_strConfigPath + d_ptr->m_strConfigFileName;

    QJsonObject rootObj;
    QFile file(QString::fromStdString(str_FullPath));
    if (!file.open(QFile::ReadOnly))
    {
        qDebug() << "CCommonConfig::InitConfig, open config file failed";
        return;
    }
    QByteArray jsonData = file.readAll();
    file.close();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
    if (error.error != QJsonParseError::NoError)
    {
        qDebug()<<"Json error: "<<error.errorString();
        return ;
    }
    if (!doc.isObject())
    {
        qDebug()<<"json root fail!";
        return ;
    }

    rootObj = doc.object();
    d_ptr->Json2Map(rootObj);
}

void CCommonConfig::UpdateSystemConfig(const string &strConfigKey, string &strConfigValue)
{
    SetValue(SYSTEM_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::UpdateBusinessConfig(const string &strConfigKey, string &strConfigValue)
{
    SetValue(BUSINESS_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::UpdateLogConfig(const string &strConfigKey, string &strConfigValue)
{
    SetValue(LOG_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::UpdateDataConfig(const string &strConfigKey, string &strConfigValue)
{
    SetValue(DATA_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::UpdateDefaultConfig(const string &strConfigKey, string &strConfigValue)
{
    SetValue(DEFAULT_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::UpdateUIConfig(const string &strConfigKey, string &strConfigValue)
{
    SetValue(UI_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::GetSystemConfig(const string &strConfigKey, string &strConfigValue) const
{
    d_ptr->GetConfig(SYSTEM_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::GetBusinessConfig(const string &strConfigKey, string &strConfigValue) const
{
    d_ptr->GetConfig(BUSINESS_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::GetLogConfig(const string &strConfigKey, string &strConfigValue) const
{
    d_ptr->GetConfig(LOG_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::GetDataConfig(const string &strConfigKey, string &strConfigValue) const
{
    d_ptr->GetConfig(DATA_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::GetDefaultConfig(const string &strConfigKey, string &strConfigValue) const
{
    d_ptr->GetConfig(DEFAULT_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::GetUIConfig(const string &strConfigKey, string &strConfigValue) const
{
    d_ptr->GetConfig(UI_SECTION, strConfigKey, strConfigValue);
}

// ========== 实现 IConfigReader 接口 ==========

void CCommonConfig::GetValue(const string &strSection, const string &strKey, string &strValue) const
{
    d_ptr->GetConfig(strSection, strKey, strValue);
}

bool CCommonConfig::HasKey(const string &strSection, const string &strKey) const
{
    string tempValue;
    d_ptr->GetConfig(strSection, strKey, tempValue);
    return !tempValue.empty();
}

// ========== 实现 IConfigUpdater 接口 ==========

void CCommonConfig::SetValue(const string &strSection, const string &strKey, const string &strValue)
{
    std::unique_lock<std::shared_mutex> lock(s_configMutex);

    d_ptr->m_mapConfigSection[strSection][strKey] = strValue;
    Write2File();
}

bool CCommonConfig::Write2File()
{
    QJsonObject rootObj;

    {
        std::unique_lock<std::shared_mutex> lock(s_configMutex);

        for (const auto& sectionPair : d_ptr->m_mapConfigSection)
        {
            QJsonObject sectionObj;

            for (const auto& kv : sectionPair.second)
            {
                sectionObj[QString::fromStdString(kv.first)] =
                    QString::fromStdString(kv.second);
            }

            rootObj[QString::fromStdString(sectionPair.first)] = sectionObj;
        }
    }
    
    string fullPath = d_ptr->m_strConfigPath + d_ptr->m_strConfigFileName;
    
    QFile file(QString::fromStdString(fullPath));
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
    {
        qDebug() << "CCommonConfig::Write2File, open file failed:" << file.errorString();
        return false;
    }
    
    file.write(QJsonDocument(rootObj).toJson());
    file.close();
    
    qDebug() << "CCommonConfig::Write2File success";
    return true;
}