 /*
 * @file: CMainConfigModel.h
 * @brief: 
 * @author: nuo
 * @date: 2026/9/18
 * @Detail:
 */

#pragma once
#include <QAbstractListModel>
#include <memory>
class CMainConfigModelPrivate;
class CMainConfigModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit CMainConfigModel();

    ~CMainConfigModel();

    enum E_SIDEBAR_ITEM_TYPE
    {
        RoleName = Qt::UserRole + 1,
        RoleColor,
        RoleIsSystem,

        RoleWindowWidth,
        RoleWindowHeight,
        RoleWindowX,
        RoleWindowY,
        RoleWindowMaximized,
        RoleWindowBgPath,

        RoleShortcutSave,
        RoleShortcutNew,
        RoleShortcutDelete,
        RoleShortcutSearch,
        RoleShortcutToggleCategory,

        RolePreferenceLanguage,
        RolePreferenceTheme,
        RolePreferenceAutoSave,
        RolePreferenceAutoSaveInterval,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    std::shared_ptr<CMainConfigModelPrivate> d_ptr;
};