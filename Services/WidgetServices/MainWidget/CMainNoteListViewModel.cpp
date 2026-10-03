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

CMainNoteListViewModel::CMainNoteListViewModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_pNoteService(nullptr)
    , m_eCurNoteType(E_NOTE_TYPE_ALL)
    , m_s64NextID(0)
{
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
    st_NoteModelItem st_NoteModelData;
    st_NoteModelData.m_s64NoteId     = m_s64NextID++;
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
        m_pNoteService->UpDataNextID(m_s64NextID);
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
    if (index >= 0 && index < m_vecNote.size())
    {
        st_NoteModelItem &stItem = m_vecNote[index];
        stItem.m_s64RemindTime = time.toSecsSinceEpoch();

        PushContainer(stItem);

        const QModelIndex qIndex = createIndex(index, 0);
        emit dataChanged(qIndex, qIndex, {RoleNoteRemindTime});
    }
}

void CMainNoteListViewModel::SetNoteLevel(int index, int level)
{
    if (index >= 0 && index < m_vecNote.size())
    {
        st_NoteModelItem &stItem = m_vecNote[index];
        stItem.m_eNoteLevel = static_cast<E_NOTE_EVENT_WAKEUP_LEVEL>(level);

        PushContainer(stItem);

        const QModelIndex qIndex = createIndex(index, 0);
        emit dataChanged(qIndex, qIndex, {RoleNoteLevel});
    }
}

void CMainNoteListViewModel::SetNoteType(int index, int type)
{
    if (index >= 0 && index < m_vecNote.size())
    {
        st_NoteModelItem &stItem = m_vecNote[index];
        stItem.m_eNoteType = static_cast<E_NOTE_TYPE> (type);

        PushContainer(stItem);

        const QModelIndex qIndex = createIndex(index, 0);
        emit dataChanged(qIndex, qIndex, {RoleNoteType});
    }
}

void CMainNoteListViewModel::DeleteNote(int index)
{
    if (index >= 0 && index < m_vecNote.size())
    {
        INT64 s64NoteID = m_vecNote[index].m_s64NoteId;

        m_vecNote[index].m_bDeleted = true;
        st_NoteModelItem stItem = m_vecNote[index];
        beginRemoveRows(QModelIndex(), index, index);
        m_vecNote.remove(index);
        endRemoveRows();
        m_hashNoteData[E_NOTE_TYPE_ALL].remove(index);
        if (m_pNoteService)
        {
            m_pNoteService->DeleteNote(stItem);
        }
    }
}

void CMainNoteListViewModel::PinTop(int index)
{
    //未完成
    if (index >= 0 && index < m_vecNote.size())
    {
        const INT64 s64NoteId = m_vecNote[index].m_s64NoteId;
        rotate(m_vecNote.begin(), m_vecNote.begin() + index, m_vecNote.begin() + index + 1);

        m_vecNote[0].m_bIsTop = TRUE;
        m_vecNote[0].m_s32TopOrder = TRUE;

        const auto &stItem = m_vecNote[0];
        m_hashNoteData[stItem.m_eNoteType].insert(stItem.m_s64NoteId, stItem);
        m_hashNoteData[E_NOTE_TYPE_ALL].insert(stItem.m_s64NoteId, stItem);
    }
}

void CMainNoteListViewModel::PutArrNoteData(NOTE_CACHE_SNAPSHOT pSnapshot)
{
    if (!pSnapshot)
    {
        return;
    }

    m_hashNoteData.clear();
    INT64 s64MaxId = 0;

    for (const auto &p_Src : *pSnapshot)
    {
        if (!p_Src || p_Src->m_eNoteType == 0)
        {
            continue;
        }
        const st_NoteModelItem st_ModelItem = ConvertToModel(*p_Src);
        PushContainer(st_ModelItem);
        s64MaxId = std::max(s64MaxId, st_ModelItem.m_s64NoteId);
    }

    if (m_s64NextID <= s64MaxId)
    {
        m_s64NextID = s64MaxId + 1;
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

void CMainNoteListViewModel::SetNextID(INT64 s64NextID)
{
    m_s64NextID = s64NextID;
}

void CMainNoteListViewModel::Init(std::shared_ptr<CNoteDataService> pService)
{
    m_pNoteService = std::move(pService);
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

        m_pNoteService->RegisterNoteChangeCallback(
            [this](const SNoteCacheChange &stChange)
            {
                QMetaObject::invokeMethod(this, [this, stChange]()
                {
                    ApplyChange(stChange);
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

st_NoteModelItem CMainNoteListViewModel::ConvertToModel(const ST_NOTE_DATA &st_Src) const
{
    st_NoteModelItem st_ModelItem;
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
    st_ModelItem.m_bIsTop           = st_Src.m_bIsTop;
    st_ModelItem.m_s32TopOrder      = st_Src.m_s32TopOrder;

    return st_ModelItem;
}

INT32 CMainNoteListViewModel::FindRow(INT64 s64NoteId) const
{
    for (INT32 i = 0; i < m_vecNote.size(); ++i)
    {
        if (m_vecNote.at(i).m_s64NoteId == s64NoteId)
        {
            return i;
        }
    }
    return -1;
}

void CMainNoteListViewModel::ApplyChange(const SNoteCacheChange &stChange)
{
    switch (stChange.eType)
    {
        case E_CACHE_CHANGE_ADD:
        {
            if (!stChange.pData || stChange.pData->m_eNoteType == 0)
            {
                return;
            }
            const st_NoteModelItem st_Item = ConvertToModel(*stChange.pData);
            PushContainer(st_Item);

            if (m_eCurNoteType == E_NOTE_TYPE_ALL || m_eCurNoteType == st_Item.m_eNoteType)
            {
                const INT32 nRow = m_vecNote.size();
                beginInsertRows(QModelIndex(), nRow, nRow);
                m_vecNote.append(st_Item);
                endInsertRows();
            }

            if (m_s64NextID <= st_Item.m_s64NoteId)
            {
                m_s64NextID = st_Item.m_s64NoteId + 1;
            }
            break;
        }
        case E_CACHE_CHANGE_UPDATE:
        {
            if (!stChange.pData)
            {
                return;
            }
            const INT32 nRow = FindRow(stChange.pData->m_s32id);
            if (nRow < 0)
            {
                return;
            }
            const st_NoteModelItem st_Item = ConvertToModel(*stChange.pData);
            m_vecNote[nRow] = st_Item;
            PushContainer(st_Item);

            const QModelIndex qIndex = index(nRow, 0);
            emit dataChanged(qIndex, qIndex);
            break;
        }
        case E_CACHE_CHANGE_REMOVE:
        {
            const INT32 nRow = FindRow(stChange.s32NoteId);
            if (nRow < 0)
            {
                return;
            }
            const st_NoteModelItem st_Item = m_vecNote.at(nRow);
            m_hashNoteData[st_Item.m_eNoteType].remove(st_Item.m_s64NoteId);
            m_hashNoteData[E_NOTE_TYPE_ALL].remove(st_Item.m_s64NoteId);

            beginRemoveRows(QModelIndex(), nRow, nRow);
            m_vecNote.remove(nRow);
            endRemoveRows();
            break;
        }
        default:
        {
            break;
        }
    }
}
