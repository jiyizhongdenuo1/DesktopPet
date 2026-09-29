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

CMainConfigModel::CMainConfigModel()
{
}

CMainConfigModel::~CMainConfigModel()
{
}

int CMainConfigModel::windowWidth() const
{
    int w = 0, h = 0;
    g_ConfigManager->GetWindowSize(w, h);
    return w;
}

int CMainConfigModel::windowHeight() const
{
    int w = 0, h = 0;
    g_ConfigManager->GetWindowSize(w, h);
    return h;
}

int CMainConfigModel::windowX() const
{
    int x = 0, y = 0;
    g_ConfigManager->GetWindowPosition(x, y);
    return x;
}

int CMainConfigModel::windowY() const
{
    int x = 0, y = 0;
    g_ConfigManager->GetWindowPosition(x, y);
    return y;
}

int CMainConfigModel::setWindowWidth(int s32Width) const
{
    g_ConfigManager->SetWindowGeometry(s32Width, -1);
}

int CMainConfigModel::setWindowHeight(int s32Height) const
{
    g_ConfigManager->SetWindowGeometry(-1, s32Height);
}

int CMainConfigModel::setWindowX(int s32X) const
{
    g_ConfigManager->SetWindowPosition(s32X, -1);
}

int CMainConfigModel::setWindowY(int s32Y) const
{
    g_ConfigManager->SetWindowPosition(-1, s32Y);
}

int CMainConfigModel::windowMaximized() const
{

    return g_ConfigManager->GetWindowIsMaximized();
}

QString CMainConfigModel::shortcutSave() const
{
    return QString::fromStdString(g_ConfigManager->GetShortcut("saveNote"));
}

QString CMainConfigModel::shortcutNew() const
{
    return QString::fromStdString(g_ConfigManager->GetShortcut("newNote"));
}

QString CMainConfigModel::shortcutDelete() const
{
    return QString::fromStdString(g_ConfigManager->GetShortcut("deleteNote"));
}

QString CMainConfigModel::shortcutSearch() const
{
    return QString::fromStdString(g_ConfigManager->GetShortcut("searchNotes"));
}

QString CMainConfigModel::shortcutToggleCategory() const
{
    return QString::fromStdString(g_ConfigManager->GetShortcut("toggleCategory"));
}

void CMainConfigModel::setShortcutSave(const QString& strKey)
{
    g_ConfigManager->SetShortcut("saveNote", strKey.toStdString());
    emit configChanged();
}

void CMainConfigModel::setShortcutNew(const QString& strKey)
{
    g_ConfigManager->SetShortcut("newNote", strKey.toStdString());
    emit configChanged();
}

void CMainConfigModel::setShortcutDelete(const QString& strKey)
{
    g_ConfigManager->SetShortcut("deleteNote", strKey.toStdString());
    emit configChanged();
}

void CMainConfigModel::setShortcutSearch(const QString& strKey)
{
    g_ConfigManager->SetShortcut("searchNotes", strKey.toStdString());
    emit configChanged();
}

void CMainConfigModel::setShortcutToggleCategory(const QString& strKey)
{
    g_ConfigManager->SetShortcut("toggleCategory", strKey.toStdString());
    emit configChanged();
}
