/*
 * @file: CSidebarModel.h
 * @brief: 
 * @author: nuo
 * @date: 2026/9/7
 * @Detail:
 */

#include "CSidebarModel.h"

#include "SCategoryInfo.h"

using namespace std;

CSidebarModel::CSidebarModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_s32CurrentSelection(0)
{
}

int CSidebarModel::currentSelection() const
{
    return m_s32CurrentSelection;
}

void CSidebarModel::setCurrentSelection(int currentSelection)
{
    if (m_s32CurrentSelection != currentSelection)
    {
        if (currentSelection >= 0 && currentSelection < m_vecCategoryItems.size())
        {
            m_s32CurrentSelection = currentSelection;
            m_strCurrentSelectionName = m_vecCategoryItems[currentSelection].strName;
            emit currentSelectionChanged(m_s32CurrentSelection);
        }
    }
}

void CSidebarModel::RestoreSettings()
{
    if (!m_strCurrentSelectionName.isEmpty())
    {
        INT32 s32Index = FindIndexByName(m_strCurrentSelectionName);
        if (s32Index >= 0)
        {
            setCurrentSelection(s32Index);
        }
        else
        {
            setCurrentSelection(0);
        }
    }
    else
    {
        setCurrentSelection(0);
    }
}

void CSidebarModel::SaveSettings(INT32 s32Index)
{
    if (s32Index >= 0 && s32Index < m_vecCategoryItems.size())
    {
        m_strCurrentSelectionName = m_vecCategoryItems[s32Index].strName;
    }
    else
    {
        m_strCurrentSelectionName.clear();
    }
}

INT32 CSidebarModel::FindIndexByName(const QString &strName) const
{
    if (strName.isEmpty())
    {
        return -1;
    }
    for (INT32 s32_Idx = 0; s32_Idx < m_vecCategoryItems.size(); ++s32_Idx)
    {
        if (m_vecCategoryItems[s32_Idx].strName == strName)
        {
            return s32_Idx;
        }
    }
    return -1;
}

bool CSidebarModel::UpdateSidebarItems(PARAM param1, PARAM param2)
{
    SaveSettings(currentSelection());
    auto* pVecCategories = reinterpret_cast<vector<SCategoryInfo>*>(param1);
    if (pVecCategories)
    {
        beginResetModel();
        m_vecCategoryItems = *pVecCategories;
        endResetModel();
    }
    RestoreSettings();
    return true;
}

int CSidebarModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return static_cast<int>(m_vecCategoryItems.size());
}

QVariant CSidebarModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    int row = index.row();
    if (row < 0 || row >= static_cast<int>(m_vecCategoryItems.size()))
    {
        return QVariant();
    }

    const SCategoryInfo& category = m_vecCategoryItems[row];

    switch (role)
    {
    case RoleName:
        return category.strName;
    case RoleColor:
        return category.strColor;
    case RoleIsSystem:
        return category.bIsSystem;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> CSidebarModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[RoleName] = "name";
    roles[RoleColor] = "color";
    roles[RoleIsSystem] = "isSystem";
    return roles;
}