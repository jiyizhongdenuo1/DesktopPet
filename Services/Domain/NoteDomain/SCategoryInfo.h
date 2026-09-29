/*
 * @file: SCategoryInfo.h
 * @brief: 分类信息数据结构定义
 * @author: nuo
 * @date: 2026/8/29
 * @Detail:
 */

#pragma once

#include <QString>
#include <QJsonObject>
#include <cstdint>
#include "DNoteCommon.h"

struct SCategoryInfo
{
    QString m_strName;
    QString m_strColor;
    QString m_strIcon;
    INT32U  m_u32Type;
    bool    m_bIsSystem;
    bool    m_bCanDelete;

    SCategoryInfo()
        : m_u32Type(0)
        , m_bIsSystem(false)
        , m_bCanDelete(true)
    {

    }

    static void FromJson(const QJsonObject& obj, SCategoryInfo& objInfo)
    {
        objInfo.m_strName = obj["name"].toString();
        objInfo.m_strColor = obj["color"].toString();
        objInfo.m_strIcon = obj["icon"].toString();
        objInfo.m_u32Type = obj["type"].toInt(-1);
        objInfo.m_bIsSystem = obj["isSystem"].toBool();
        objInfo.m_bCanDelete = obj["canDelete"].toBool();
    }

    static void ToJson(QJsonObject& obj, const SCategoryInfo& objInfo)
    {
        obj["name"] = objInfo.m_strName;
        obj["color"] = objInfo.m_strColor;
        obj["icon"] = objInfo.m_strIcon;
        obj["type"] = static_cast<int>(objInfo.m_u32Type);
        obj["isSystem"] = objInfo.m_bIsSystem;
        obj["canDelete"] = objInfo.m_bCanDelete;
    }

    QJsonObject ToJson() const
    {
        QJsonObject obj;
        obj["name"] = m_strName;
        obj["color"] = m_strColor;
        obj["icon"] = m_strIcon;
        obj["type"] = static_cast<int>(m_u32Type);
        obj["isSystem"] = m_bIsSystem;
        obj["canDelete"] = m_bCanDelete;
        return obj;
    }
};