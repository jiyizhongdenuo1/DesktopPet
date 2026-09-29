/*
 * @file: CMainNoteListViewModel.h
 * @brief: 笔记列表视图模型
 * @author: nuo
 * @date: 2026/5/10
 * @Detail: 负责管理 st_NoteModelData 数据并与 QML 进行交互
 */
#include <QVariant>
#include <QDateTime>
#include <QMetaObject>

#include "CMainNoteListViewModel.h"
#include "CNoteDataService.h"
#include "CServiceLocator.h"

CMainNoteListViewModel::CMainNoteListViewModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_pNoteService(nullptr)
    , m_eCurNoteType(E_NOTE_TYPE_ALL)
{
    Init();
}

int CMainNoteListViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_vecNote.size();
}

QVariant CMainNoteListViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_vecNote.size())
    {
        return QVariant();
    }
    const st_NoteModelItem &note = m_vecNote.at(index.row());

    switch (role)
    {
        case RoleNoteId:            return note.m_s64NoteId;
        case RoleNoteLevel:         return note.m_eNoteLevel;
        case RoleNoteTimeSpanType:  return note.m_eTimeSpanType;
        case RoleNoteContent:       return QString::fromStdString(note.m_strContent);
        case RoleNoteWriteTime:     return QDateTime::fromSecsSinceEpoch(note.m_s64WriteTime).toString("yyyy-MM-dd hh:mm");
        case RoleNoteModifyTime:    return QDateTime::fromSecsSinceEpoch(note.m_s64ModifyTime).toString("yyyy-MM-dd hh:mm");
        case RoleNoteRemindTime:    return QDateTime::fromSecsSinceEpoch(note.m_s64RemindTime).toString("yyyy-MM-dd hh:mm");
        case RoleNoteRemindFrequency: return note.m_eRemindFrequency;
        case RoleNoteCompleted:     return note.m_bCompleted;
        case RoleNoteDeleted:       return note.m_bDeleted; // 简单取第一行作为标题
        case RoleNoteType:          return note.m_eNoteType;
        default:                    return QVariant();
    }
}

QHash<int, QByteArray> CMainNoteListViewModel::roleNames() const
{
        QHash<int, QByteArray> roles;
        roles[RoleNoteId]              = "Id";
        roles[RoleNoteContent]         = "noteContent";
        roles[RoleNoteType]            = "noteType";
        roles[RoleNoteLevel]           = "noteLevel";
        roles[RoleNoteTimeSpanType]    = "timeSpanType";
        roles[RoleNoteWriteTime]       = "writeTime";
        roles[RoleNoteModifyTime]      = "modifyTime";
        roles[RoleNoteRemindTime]      = "remindTime";
        roles[RoleNoteRemindFrequency] = "remindFrequency";
        roles[RoleNoteCompleted]       = "isCompleted";
        roles[RoleNoteDeleted]         = "isDeleted";
        return roles;
}

void CMainNoteListViewModel::AddNote(const QString &strContent, const QDateTime &time)
{
    const auto &mapAll = m_hashNoteData[E_NOTE_TYPE_ALL];
    const INT64 s64NewId = mapAll.isEmpty() ? 1 : mapAll.lastKey() + 1;

    st_NoteModelItem st_NoteModelData;
    st_NoteModelData.m_s64NoteId     = s64NewId;
    st_NoteModelData.m_s64WriteTime  = time.toSecsSinceEpoch();
    st_NoteModelData.m_s64ModifyTime = st_NoteModelData.m_s64WriteTime;
    st_NoteModelData.m_strContent    = strContent.toStdString();
    st_NoteModelData.m_eNoteType     = m_eCurNoteType;

    PushContainer(st_NoteModelData);

    beginInsertRows(QModelIndex(), m_vecNote.size(), m_vecNote.size());
    m_vecNote.append(st_NoteModelData);
    endInsertRows();

    if (m_pNoteService)
    {
        m_pNoteService->AddNote(st_NoteModelData);
    }
}

void CMainNoteListViewModel::UpdateNoteContent(int index, const QString &newContent)
{
    if (index < 0 || index >= m_vecNote.size())
    {
        return;
    }

    st_NoteModelItem &stItem = m_vecNote[index];
    stItem.m_strContent    = newContent.toStdString();
    stItem.m_s64ModifyTime = QDateTime::currentSecsSinceEpoch();

    PushContainer(stItem);

    const QModelIndex qIndex = createIndex(index, 0);
    emit dataChanged(qIndex, qIndex, {RoleNoteContent, RoleNoteModifyTime});
}

void CMainNoteListViewModel::SetNoteRemind(int index, const QDateTime &time)
{
}

void CMainNoteListViewModel::SetNoteLevel(int index, int level)
{
}

void CMainNoteListViewModel::SetNoteType(int index, int type)
{
}

void CMainNoteListViewModel::DeleteNote(int index)
{
}

void CMainNoteListViewModel::PutArrNoteData(NOTE_CACHE_SNAPSHOT pSnapshot)
{
    if (!pSnapshot)
    {
        return;
    }

    m_hashNoteData.clear();

    st_NoteModelItem st_ModelItem;
    for (const auto &st_Src : *pSnapshot)
    {
        if (st_Src.m_eNoteType == 0)
        {
            continue;
        }

        st_ModelItem.m_s64NoteId        = st_Src.m_s32id;
        st_ModelItem.m_eNoteLevel       = st_Src.m_eNoteLevel;
        st_ModelItem.m_eTimeSpanType    = st_Src.m_eTimeSpanType;
        st_ModelItem.m_s64RemindTime    = st_Src.m_s64RemindTime;
        st_ModelItem.m_s64WriteTime     = st_Src.m_s64CreateTime;
        st_ModelItem.m_s64ModifyTime    = st_Src.m_s64UpdateTime;
        st_ModelItem.m_eRemindFrequency = st_Src.m_eRemindFrequency;
        st_ModelItem.m_eNoteType        = st_Src.m_eNoteType;
        st_ModelItem.m_bCompleted       = st_Src.m_bCompleted;
        st_ModelItem.m_bDeleted         = st_Src.m_bDeleted;
        st_ModelItem.m_strContent       = st_Src.m_szContent;
        PushContainer(st_ModelItem);
    }

    beginResetModel();
    m_vecNote = m_hashNoteData[m_eCurNoteType].values();
    endResetModel();
}

void CMainNoteListViewModel::UpdateNoteIndex(INT32U u32Index)
{
    m_eCurNoteType = static_cast<E_NOTE_TYPE>(u32Index);
    beginResetModel();
    m_vecNote = m_hashNoteData[m_eCurNoteType].values();
    endResetModel();
}

void CMainNoteListViewModel::Init()
{
    InitService();
}

void CMainNoteListViewModel::InitService()
{
    m_pNoteService = g_ServiceLocator.GetNoteService();
    if (m_pNoteService)
    {
        /*
         * 注册数据加载回调，处理数据线程读取完成后通知 ViewModel 更新。
         *
         * 注意：PutArrNoteData() 内部调用了 QAbstractItemModel 的
         * beginResetModel() / endResetModel()，这些方法只能在主线程调用。
         * 数据线程的回调通过 QMetaObject::invokeMethod + Qt::QueuedConnection
         * 将数据 marshal 到主线程事件队列执行，避免跨线程操作模型导致 SIGSEGV。
         */
//        m_pNoteService->RegisterNoteModelDataLoadCallback(
//            [this](std::shared_ptr<std::array<ST_NOTE_DATA, DDataCache::MAX_CACHE_SIZE>> pArrData, INT32 s32Count)
//            {
//                QMetaObject::invokeMethod(this, [this, pArrData, s32Count]()
//                {
//                    PutArrNoteData(pArrData, s32Count);
//                }, Qt::QueuedConnection);
//            });
        m_pNoteService->RegisterNoteModelDataLoadCallback(
            [this](NOTE_CACHE_SNAPSHOT pSnapshot)
            {
                QMetaObject::invokeMethod(this, [this, pSnapshot]()
                {
                    PutArrNoteData(pSnapshot);
                }, Qt::QueuedConnection);
            });
    }
}

void CMainNoteListViewModel::PushContainer(const st_NoteModelItem &Item)
{
    m_hashNoteData[Item.m_eNoteType].insert(Item.m_s64NoteId, Item);

    if (Item.m_eNoteType != E_NOTE_TYPE_ALL)
    {
        m_hashNoteData[E_NOTE_TYPE_ALL].insert(Item.m_s64NoteId, Item);
    }
}