/*
 * @file: CMainNoteListViewModel.cpp
 * @brief: 笔记列表视图模型
 * @author: nuo
 * @date: 2026/5/10
 * @Detail: 负责管理 st_NoteModelData 数据并与 QML 进行交互
 */
#include <QVariant>
#include <QVariantList>
#include <QDateTime>
#include <QMetaObject>

#include "CMainNoteListViewModel.h"
#include "CNoteDataService.h"
#include "DServiceBase.h"
#include "DNoteMapping.h"

CMainNoteListViewModel::CMainNoteListViewModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_pNoteService(nullptr)
{
    RegisterNoteActions();
}

int CMainNoteListViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_store.Count();
}

QVariant CMainNoteListViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_store.Count())
    {
        return QVariant();
    }
    const st_NoteModelItem &note = m_store.ItemAt(index.row());

    switch (role)
    {
        case RoleNoteId:            return note.m_s64NoteId;
        case RoleNoteLevel:         return note.m_eNoteLevel;
        case RoleNoteTimeSpanType:  return note.m_eTimeSpanType;
        case RoleNoteContent:       return QString::fromStdString(note.m_strContent);
        case RoleNoteWriteTime:     return QDateTime::fromSecsSinceEpoch(note.m_s64WriteTime).toString("yyyy-MM-dd hh:mm");
        case RoleNoteModifyTime:    return QDateTime::fromSecsSinceEpoch(note.m_s64ModifyTime).toString("yyyy-MM-dd hh:mm");
        case RoleNoteRemindTime:
        {
            // m_s64RemindTime 为 0 表示未设置提醒，直接返回空串，避免显示 1970 年
            return note.m_s64RemindTime > 0
                ? QDateTime::fromSecsSinceEpoch(note.m_s64RemindTime).toString("yyyy-MM-dd hh:mm")
                : QString();
        }
        case RoleNoteRemindFrequency: return note.m_eRemindFrequency;
        case RoleNoteCompleted:     return note.m_bCompleted;
        case RoleNoteDeleted:       return note.m_bDeleted;
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

void CMainNoteListViewModel::RegisterNoteActions()
{
    m_mapActions[static_cast<INT32>(E_NOTE_ACTION::E_NOTE_ACTION_CATEGORY)] = [this](INT32 index, const QVariant &value)
    {
        ChangeNoteCategory(index, value.toInt());
    };

    m_mapActions[static_cast<INT32>(E_NOTE_ACTION::E_NOTE_ACTION_REMIND)] = [this](INT32 index, const QVariant &value)
    {
        // QML 传入的是 [提醒时间, 重复类型索引, 自定义间隔] 三个值
        const QVariantList vecArgs = value.toList();
        if (vecArgs.size() < 3)
        {
            return;
        }

        SetNoteRemind(index, vecArgs.at(0).toDateTime(),
                      vecArgs.at(1).toInt(), vecArgs.at(2).toInt());
    };

    m_mapActions[static_cast<INT32>(E_NOTE_ACTION::E_NOTE_ACTION_SET_LEVEL)] = [this](INT32 index, const QVariant &value)
    {
        SetNoteLevel(index, value.toInt());
    };

    m_mapActions[static_cast<INT32>(E_NOTE_ACTION::E_NOTE_ACTION_PIN_NOTE)] = [this](INT32 index, const QVariant &)
    {
        PinTop(index);
    };

    m_mapActions[static_cast<INT32>(E_NOTE_ACTION::E_NOTE_ACTION_DELETE)] = [this](INT32 index, const QVariant &)
    {
        DeleteNote(index);
    };
}

void CMainNoteListViewModel::ExecNoteAction(INT32 s32Action, INT32 index, const QVariant &value)
{
    if (m_mapActions.contains(s32Action))
    {
        m_mapActions.value(s32Action)(index, value);
    }
}

void CMainNoteListViewModel::AddNote(const QString &strContent, const QDateTime &time)
{
    const INT32 s32Row = m_store.Count();

    beginInsertRows(QModelIndex(), s32Row, s32Row);
    const st_NoteModelItem stItem = m_store.Add(strContent.toStdString(), time.toSecsSinceEpoch());
    endInsertRows();

    if (m_pNoteService)
    {
        m_pNoteService->AddNote(stItem);
        m_pNoteService->UpDataNextID(m_store.NextId());
    }
}

void CMainNoteListViewModel::UpdateNoteContent(int index, const QString &newContent)
{
    if (index < 0 || index >= m_store.Count())
    {
        return;
    }

    const st_NoteModelItem stItem =
        m_store.UpdateContent(index, newContent.toStdString(), QDateTime::currentSecsSinceEpoch());

    UpdateNoteFile(stItem);

    const QModelIndex qIndex = createIndex(index, 0);
    emit dataChanged(qIndex, qIndex, {RoleNoteContent, RoleNoteModifyTime});
}

void CMainNoteListViewModel::SetNoteRemind(int index, const QDateTime &time,
                                           int repeatIndex, int interval)
{
    if (index < 0 || index >= m_store.Count())
    {
        return;
    }

    // QML 的 ComboBox 索引 0 是"单次"，对应 E_NOTE_REMIND_ONCE(1)，
    // 故整体 +1 跳过 E_NOTE_REMIND_NONE；QML 的 model 增删选项时需同步调整。
    const E_NOTE_REMIND_FREQUENCY eFreq =
        static_cast<E_NOTE_REMIND_FREQUENCY>(repeatIndex + 1);

    const st_NoteModelItem stItem =
        m_store.SetRemind(index, time.toSecsSinceEpoch(), eFreq, interval);

    UpdateNoteFile(stItem);

    const QModelIndex qIndex = createIndex(index, 0);
    emit dataChanged(qIndex, qIndex, {RoleNoteRemindTime, RoleNoteRemindFrequency});
}

void CMainNoteListViewModel::SetNoteLevel(int index, int level)
{
    if (index < 0 || index >= m_store.Count())
    {
        return;
    }

    const st_NoteModelItem stItem = m_store.SetLevel(index, level);

    UpdateNoteFile(stItem);

    const QModelIndex qIndex = createIndex(index, 0);
    emit dataChanged(qIndex, qIndex, {RoleNoteLevel});
}

void CMainNoteListViewModel::ChangeNoteCategory(int index, int type)
{
    if (index < 0 || index >= m_store.Count())
    {
        return;
    }

    const E_NOTE_TYPE eNewType = static_cast<E_NOTE_TYPE>(type);
    if (m_store.ItemAt(index).m_eNoteType == eNewType)
    {
        return;
    }

    const BOOL bRemoveRow = !m_store.ShouldShow(eNewType);
    if (bRemoveRow)
    {
        beginRemoveRows(QModelIndex(), index, index);
    }

    const st_NoteModelItem stItem = m_store.ChangeCategory(index, type);

    if (bRemoveRow)
    {
        endRemoveRows();
    }

    UpdateNoteFile(stItem);

    if (!bRemoveRow)
    {
        const QModelIndex qIndex = createIndex(index, 0);
        emit dataChanged(qIndex, qIndex, {RoleNoteType});
    }
}

void CMainNoteListViewModel::DeleteNote(int index)
{
    if (index < 0 || index >= m_store.Count())
    {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    const st_NoteModelItem stItem = m_store.Delete(index);
    endRemoveRows();

    if (m_pNoteService)
    {
        m_pNoteService->DeleteNote(stItem);
    }
}

void CMainNoteListViewModel::PinTop(int index)
{
    if (index < 0 || index >= m_store.Count())
    {
        return;
    }

    beginResetModel();
    const st_NoteModelItem stItem = m_store.PinTop(index);
    endResetModel();

    UpdateNoteFile(stItem);
}

void CMainNoteListViewModel::PutArrNoteData(NOTE_CACHE_SNAPSHOT pSnapshot)
{
    if (!pSnapshot)
    {
        return;
    }

    QVector<st_NoteModelItem> vecItems;
    vecItems.reserve(pSnapshot->size());

    for (const auto &p_Src : *pSnapshot)
    {
        if (!p_Src || p_Src->m_eNoteType == 0)
        {
            continue;
        }

        st_NoteModelItem stItem;
        NoteMapping::ToModel(*p_Src, stItem);
        vecItems.append(stItem);
    }

    beginResetModel();
    m_store.Rebuild(vecItems);
    endResetModel();
}

void CMainNoteListViewModel::UpdateNoteIndex(INT32U u32Index)
{
    beginResetModel();
    m_store.SetCurNoteType(static_cast<E_NOTE_TYPE>(u32Index));
    endResetModel();
}

void CMainNoteListViewModel::SetNextID(INT64 s64NextID)
{
    m_store.SetNextId(s64NextID);
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

void CMainNoteListViewModel::ApplyChange(const SNoteCacheChange &stChange)
{
    switch (stChange.eType)
    {
        case E_CACHE_CHANGE_TYPE::E_CACHE_CHANGE_ADD:
        {
            if (!stChange.pData || stChange.pData->m_eNoteType == 0)
            {
                return;
            }

            st_NoteModelItem stItem;
            NoteMapping::ToModel(*stChange.pData, stItem);

            const BOOL bAppend = m_store.ShouldShow(stItem.m_eNoteType);
            if (bAppend)
            {
                const INT32 s32Row = m_store.Count();
                beginInsertRows(QModelIndex(), s32Row, s32Row);
            }

            m_store.ApplyAdd(stItem);

            if (bAppend)
            {
                endInsertRows();
            }
            break;
        }
        case E_CACHE_CHANGE_TYPE::E_CACHE_CHANGE_UPDATE:
        {
            if (!stChange.pData)
            {
                return;
            }

            st_NoteModelItem stItem;
            NoteMapping::ToModel(*stChange.pData, stItem);

            const INT32 s32Row = m_store.ApplyUpdate(stItem);
            if (s32Row >= 0)
            {
                const QModelIndex qIndex = index(s32Row, 0);
                emit dataChanged(qIndex, qIndex);
            }
            break;
        }
        case E_CACHE_CHANGE_TYPE::E_CACHE_CHANGE_REMOVE:
        {
            const INT32 s32Row = m_store.FindRow(stChange.s32NoteId);
            if (s32Row < 0)
            {
                return;
            }

            beginRemoveRows(QModelIndex(), s32Row, s32Row);
            m_store.ApplyRemove(stChange.s32NoteId);
            endRemoveRows();
            break;
        }
        default:
        {
            break;
        }
    }
}

void CMainNoteListViewModel::UpdateNoteFile(const st_NoteModelItem &Item)
{
    if (m_pNoteService)
    {
        m_pNoteService->UpdateNote(Item);
    }
}