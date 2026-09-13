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

struct SCategoryInfo
{
    QString strName;
    QString strColor;
    QString strIcon;
    int32_t s32Type;
    bool bIsSystem;
    bool bCanDelete;

    SCategoryInfo()
        : s32Type(-1)
        , bIsSystem(false)
        , bCanDelete(true) {}

    static void FromJson(const QJsonObject& obj, SCategoryInfo& objInfo)
    {
        objInfo.strName = obj["name"].toString();
        objInfo.strColor = obj["color"].toString();
        objInfo.strIcon = obj["icon"].toString();
        objInfo.s32Type = obj["type"].toInt(-1);
        objInfo.bIsSystem = obj["isSystem"].toBool();
        objInfo.bCanDelete = obj["canDelete"].toBool();
    }

    static void ToJson(QJsonObject& obj, const SCategoryInfo& objInfo)
    {
        obj["name"] = objInfo.strName;
        obj["color"] = objInfo.strColor;
        obj["icon"] = objInfo.strIcon;
        obj["type"] = objInfo.s32Type;
        obj["isSystem"] = objInfo.bIsSystem;
        obj["canDelete"] = objInfo.bCanDelete;
    }

    QJsonObject ToJson() const
    {
        QJsonObject obj;
        obj["name"] = strName;
        obj["color"] = strColor;
        obj["icon"] = strIcon;
        obj["type"] = s32Type;
        obj["isSystem"] = bIsSystem;
        obj["canDelete"] = bCanDelete;
        return obj;
    }
};