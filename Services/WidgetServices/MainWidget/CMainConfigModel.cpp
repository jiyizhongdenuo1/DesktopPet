/*
 * @file: CMainConfigModel.h
 * @brief: 
 * @author: nuo
 * @date: 2026/9/18
 * @Detail:
 */

#include "CMainConfigModel.h"
#include "CConfigManager.h"
#include "SCategoryInfo.h"

#include <QDebug>

CMainConfigModel::CMainConfigModel()
{
}

CMainConfigModel::~CMainConfigModel()
{
}

int CMainConfigModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 0;
}

QVariant CMainConfigModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index);

    switch (role)
    {
    case RoleName:
    case RoleColor:
    case RoleIsSystem:
    {
        auto categories = g_ConfigManager->GetCategories();
        int row = index.row();
        if (row < 0 || row >= static_cast<int>(categories.size()))
            return QVariant();

        const auto& cat = categories[row];
        if (role == RoleName)     return cat.strName;
        if (role == RoleColor)    return cat.strColor;
        if (role == RoleIsSystem) return cat.bIsSystem;
        break;
    }

    case RoleWindowWidth:
    case RoleWindowHeight:
    {
        int w = 0, h = 0;
        g_ConfigManager->GetWindowSize(w, h);
        if (role == RoleWindowWidth)  return w;
        if (role == RoleWindowHeight) return h;
        break;
    }

    case RoleWindowX:
    case RoleWindowY:
    {
        int x = 0, y = 0;
        g_ConfigManager->GetWindowPosition(x, y);
        if (role == RoleWindowX) return x;
        if (role == RoleWindowY) return y;
        break;
    }

    case RoleWindowMaximized:
        return g_ConfigManager->GetWindowIsMaximized();

    case RoleWindowBgPath:
        return QString::fromStdString(g_ConfigManager->GetBackgroundPath());

    case RoleShortcutSave:
        return QString::fromStdString(g_ConfigManager->GetShortcut("saveNote"));
    case RoleShortcutNew:
        return QString::fromStdString(g_ConfigManager->GetShortcut("newNote"));
    case RoleShortcutDelete:
        return QString::fromStdString(g_ConfigManager->GetShortcut("deleteNote"));
    case RoleShortcutSearch:
        return QString::fromStdString(g_ConfigManager->GetShortcut("searchNotes"));
    case RoleShortcutToggleCategory:
        return QString::fromStdString(g_ConfigManager->GetShortcut("toggleCategory"));

    case RolePreferenceLanguage:
        return QString::fromStdString(g_ConfigManager->GetLanguage());
    case RolePreferenceTheme:
        return QString::fromStdString(g_ConfigManager->GetTheme());
    case RolePreferenceAutoSave:
        return g_ConfigManager->IsAutoSaveEnabled();
    case RolePreferenceAutoSaveInterval:
        return g_ConfigManager->GetAutoSaveInterval();

    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> CMainConfigModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[RoleName]     = "name";
    roles[RoleColor]    = "color";
    roles[RoleIsSystem] = "isSystem";

    roles[RoleWindowWidth]    = "windowWidth";
    roles[RoleWindowHeight]   = "windowHeight";
    roles[RoleWindowX]        = "windowX";
    roles[RoleWindowY]        = "windowY";
    roles[RoleWindowMaximized] = "windowMaximized";
    roles[RoleWindowBgPath]   = "windowBgPath";

    roles[RoleShortcutSave]           = "shortcutSave";
    roles[RoleShortcutNew]            = "shortcutNew";
    roles[RoleShortcutDelete]         = "shortcutDelete";
    roles[RoleShortcutSearch]         = "shortcutSearch";
    roles[RoleShortcutToggleCategory] = "shortcutToggleCategory";

    roles[RolePreferenceLanguage]       = "preferenceLanguage";
    roles[RolePreferenceTheme]          = "preferenceTheme";
    roles[RolePreferenceAutoSave]       = "preferenceAutoSave";
    roles[RolePreferenceAutoSaveInterval] = "preferenceAutoSaveInterval";

    return roles;
}