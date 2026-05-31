/*
* @file: CMainNoteListViewModel.h
 * @brief: 笔记列表视图模型
 * @author: nuo
 * @date: 2026/5/10
 * @Detail: 负责管理 st_NoteData 数据并与 QML 进行交互
 */

#pragma once

#include <QAbstractListModel>

#include "datatype.h"

class QDataTime;

typedef  struct st_ListNoteItem
{
    INT64       m_s64NoteId;
    INT32       m_s32NoteLevel;
    INT64       m_s64RemindTime;
    INT64       m_s64WriteTime;
    INT32       m_s32RemindLevel;
    INT32       m_s32NoteType;
    BOOL        m_bCompleted;
    BOOL        m_bDeleted;
    QString     m_strContent;
    st_ListNoteItem()
        : m_s64NoteId(0), m_s32NoteLevel(0), m_s64RemindTime(0)
        , m_s64WriteTime(0), m_s32RemindLevel(0), m_s32NoteType(0)
        , m_bCompleted(FALSE), m_bDeleted(FALSE) {}
}LIST_NOTE_ITEM;

class CMainNoteListViewModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit CMainNoteListViewModel(QObject *parent = nullptr);
    ~CMainNoteListViewModel();

    enum ENUM_ROLE_NOTELIST
    {
        RoleNoteId = Qt::UserRole + 1,
        RoleNoteContent,
        RoleNoteLevel,
        RoleNoteWriteTime,
        RoleNoteRemindTime,
        RoleNoteRemindType,
        RoleNoteCompleted,
        RoleNoteDeleted,
        RoleNoteType
    };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void AddNote(const QString &strContent, const QDateTime &time);
    Q_INVOKABLE void UpdateNoteContent(int index, const QString &newContent);
private:
    QList<st_ListNoteItem> m_listNote;
};


