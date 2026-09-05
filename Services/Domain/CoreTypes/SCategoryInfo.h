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
        objInfo.strName = obj["strName"].toString();
        objInfo.strColor = obj["strColor"].toString();
        objInfo.strIcon = obj["strIcon"].toString();
        objInfo.s32Type = obj["s32Type"].toInt();
        objInfo.bIsSystem = obj["bIsSystem"].toBool();
        objInfo.bCanDelete = obj["bCanDelete"].toBool();
    }

    static void ToJson(QJsonObject& obj, const SCategoryInfo& objInfo)
    {
        obj["strName"] = objInfo.strName;
        obj["strColor"] = objInfo.strColor;
        obj["strIcon"] = objInfo.strIcon;
        obj["s32Type"] = objInfo.s32Type;
        obj["bIsSystem"] = objInfo.bIsSystem;
        obj["bCanDelete"] = objInfo.bCanDelete;
    }

    QJsonObject ToJson() const
    {
        QJsonObject obj;
        obj["strName"] = strName;
        obj["strColor"] = strColor;
        obj["strIcon"] = strIcon;
        obj["s32Type"] = s32Type;
        obj["bIsSystem"] = bIsSystem;
        obj["bCanDelete"] = bCanDelete;
        return obj;
    }
};