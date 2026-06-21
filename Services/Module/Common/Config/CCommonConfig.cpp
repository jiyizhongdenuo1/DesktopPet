/*
 * @file: CCommonConfig.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/14
 * @Detail:
 */

#include "CCommonConfig.h"
#include <map>
#include <QString>
#include  <QJsonObject>
#include <QJsonDocument>
#include <QFile>

#include "CommonDefine.h"
#include "DConfig.h"

using namespace CPath;
using namespace std;
using namespace CConfigSection;

class CCommonConfigPrivate
{
public:
    friend class CCommonConfig;
    explicit CCommonConfigPrivate();
    ~CCommonConfigPrivate()
    {

    }
    void Json2Map(const QJsonObject &rootObj);
    void GetConfig(const string &strConfigSection, const string &strConfigKey, string &strConfigValue);
    void SaveConfig(const string &strConfigSection, const string &strConfigKey, const string &strConfigValue);

public:
    QString m_strConfigPath;
    QString m_strConfigFileName;
    QString m_strConfigContent;
    QString m_strConfigSection;

    map<string, map<string, string>> m_mapConfigSection;
};

void CCommonConfigPrivate::Json2Map(const QJsonObject &rootObj)
{
    m_mapConfigSection.clear();
    for (auto it = rootObj.begin(); it != rootObj.end(); ++it)
    {
        const QString str_Section = it.key();
        const QJsonValue value = it.value();
        if (value.isObject())
        {
            const QJsonObject obj = value.toObject();
            for (auto it = obj.begin(); it != obj.end(); ++it)
            {
                const QString str_Key = it.key();
                const QString str_Value = it.value().toString();
                m_mapConfigSection[str_Section.toStdString()][str_Key.toStdString()] = str_Value.toStdString();
            }
        }
    }
}

CCommonConfigPrivate::CCommonConfigPrivate()
    : m_strConfigPath(STORE_PATH)
    , m_strConfigFileName(COMMON_COFNIG_FILE_NAME)
    , m_strConfigContent("")
    , m_strConfigSection("")
{

}

void CCommonConfigPrivate::GetConfig(const string &strConfigSection, const string &strConfigKey,
    string &strConfigValue)
{
    if (m_mapConfigSection.find(strConfigSection) == m_mapConfigSection.end())
    {
        qDebug() << "CCommonConfigPrivate::GetConfig, config section not found";
        return;
    }
    if (m_mapConfigSection[strConfigSection].find(strConfigKey) == m_mapConfigSection[strConfigSection].end())
    {
        qDebug() << "CCommonConfigPrivate::GetConfig, config key not found";
        return;
    }
    strConfigValue = m_mapConfigSection[strConfigSection][strConfigKey];
}

void CCommonConfigPrivate::SaveConfig(const string &strConfigSection, const string &strConfigKey, const string &strConfigValue)
{
    m_mapConfigSection[strConfigSection][strConfigKey] = strConfigValue;
}

CCommonConfig::CCommonConfig()
    : d_ptr(make_unique<CCommonConfigPrivate>())
{

}

CCommonConfig::~CCommonConfig()
{

}

void CCommonConfig::InitConfig()
{
    QJsonObject rootObj;
    QFile file(QString(d_ptr->m_strConfigPath + d_ptr->m_strConfigFileName));
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

void CCommonConfig::UpdateSystemConfig(const std::string &strConfigKey, std::string &strConfigValue)
{
    d_ptr->SaveConfig(SYSTEM_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::UpdateBusinessConfig(const std::string &strConfigKey, std::string &strConfigValue)
{
    d_ptr->SaveConfig(BUSINESS_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::UpdateLogConfig(const std::string &strConfigKey, std::string &strConfigValue)
{
    d_ptr->SaveConfig(LOG_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::UpdateDataConfig(const std::string &strConfigKey, std::string &strConfigValue)
{
    d_ptr->SaveConfig(DATA_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::UpdateDefaultConfig(const std::string &strConfigKey, std::string &strConfigValue)
{
    d_ptr->SaveConfig(DEFAULT_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::UpdateUIConfig(const std::string &strConfigKey, std::string &strConfigValue)
{
    d_ptr->SaveConfig(UI_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::GetSystemConfig(const std::string &strConfigKey, std::string &strConfigValue)
{
    d_ptr->GetConfig(SYSTEM_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::GetBusinessConfig(const std::string &strConfigKey, std::string &strConfigValue)
{
    d_ptr->GetConfig(BUSINESS_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::GetLogConfig(const std::string &strConfigKey, std::string &strConfigValue)
{
    d_ptr->GetConfig(LOG_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::GetDataConfig(const std::string &strConfigKey, std::string &strConfigValue)
{
    d_ptr->GetConfig(DATA_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::GetDefaultConfig(const std::string &strConfigKey, std::string &strConfigValue)
{
    d_ptr->GetConfig(DEFAULT_SECTION, strConfigKey, strConfigValue);
}

void CCommonConfig::GetUIConfig(const std::string &strConfigKey, std::string &strConfigValue)
{
    d_ptr->GetConfig(UI_SECTION, strConfigKey, strConfigValue);
}


