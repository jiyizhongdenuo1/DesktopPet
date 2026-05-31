/*
 * @file: CMainNoteListViewModel.h
 * @brief: 笔记列表视图模型
 * @author: nuo
 * @date: 2026/5/10
 * @Detail: 负责管理 st_NoteData 数据并与 QML 进行交互
 */
#include <QVariant>
#include <QDateTime>

#include "CMainNoteListViewModel.h"

CMainNoteListViewModel::CMainNoteListViewModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

CMainNoteListViewModel::~CMainNoteListViewModel()
{

}

int CMainNoteListViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_listNote.size();
}

QVariant CMainNoteListViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_listNote.size())
    {
        return QVariant();
    }
    const st_ListNoteItem &note = m_listNote.at(index.row());

    switch (role)
    {
        case RoleNoteId:            return note.m_s64NoteId;
        case RoleNoteLevel:         return note.m_s32NoteLevel;
        case RoleNoteContent:       return note.m_strContent;
        case RoleNoteWriteTime:     return QDateTime::fromSecsSinceEpoch(note.m_s64WriteTime).toString("yyyy-MM-dd hh:mm");
        case RoleNoteRemindTime:    return QDateTime::fromSecsSinceEpoch(note.m_s64RemindTime).toString("yyyy-MM-dd hh:mm");
        case RoleNoteRemindType:    return note.m_s32NoteType;
        case RoleNoteCompleted:     return note.m_bCompleted;
        case RoleNoteDeleted:       return note.m_bDeleted; // 简单取第一行作为标题
        case RoleNoteType:          return note.m_s32NoteType;
        default:                    return QVariant();
    }
}

QHash<int, QByteArray> CMainNoteListViewModel::roleNames() const
{
        QHash<int, QByteArray> roles;
        roles[RoleNoteId]           = "Id";
        roles[RoleNoteContent]      = "noteContent";
        roles[RoleNoteType]         = "noteType";
        roles[RoleNoteLevel]        = "noteLevel";
        roles[RoleNoteWriteTime]    = "writeTime";
        roles[RoleNoteRemindTime]   = "remindTime";
        roles[RoleNoteRemindType]   = "remindType";
        roles[RoleNoteCompleted]    = "isCompleted";
        roles[RoleNoteDeleted]      = "isDeleted";
        return roles;
}

void CMainNoteListViewModel::AddNote(const QString &strContent, const QDateTime &time)
{
    beginInsertRows(QModelIndex(), m_listNote.size(), m_listNote.size());

    st_ListNoteItem newNote;
    newNote.m_s64NoteId    = m_listNote.size() + 1; // 简单的 ID 递增
    newNote.m_s64WriteTime = time.toMSecsSinceEpoch();
    newNote.m_strContent   = strContent;
    m_listNote.append(newNote);

    endInsertRows();

//    emit countChanged(); // 触发信号通知 QML

}

void CMainNoteListViewModel::UpdateNoteContent(int index, const QString &newContent)
{
    if (index >= 0 && index < m_listNote.size())
    {
        m_listNote[index].m_strContent = newContent;

        // [关键] 通知 QML 这一行的内容变了，界面会立即刷新
        QModelIndex qIndex = createIndex(index, 0);
        emit dataChanged(qIndex, qIndex, {RoleNoteContent});
    }
}
