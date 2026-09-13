/*
 * @file: CSidebarModel.h
 * @brief: 
 * @author: nuo
 * @date: 2026/9/7
 * @Detail:
 */

#pragma once
#include <QObject>
#include <QAbstractListModel>
#include <vector>

#include "datatype.h"
#include "SCategoryInfo.h"

class CSidebarModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int currentSelection READ currentSelection WRITE setCurrentSelection NOTIFY currentSelectionChanged )
public:
    explicit CSidebarModel(QObject* parent = nullptr);

     ~CSidebarModel() = default;

    enum E_SIDEBAR_ITEM_TYPE
    {
        RoleName = Qt::UserRole + 1,
        RoleColor,
        RoleIsSystem,
    };

    int currentSelection() const;
    void setCurrentSelection(int currentSelection);

    bool UpdateSidebarItems(PARAM param1, PARAM param2);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void currentSelectionChanged(int index);

private:
    void RestoreSettings();
    void SaveSettings(INT32 s32Index);
    INT32 FindIndexByName(const QString& strName) const;
   private:
    std::vector<SCategoryInfo> m_vecCategoryItems;
    INT32                      m_s32CurrentSelection;
    QString                    m_strCurrentSelectionName;

};