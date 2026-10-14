/*
 * @file: NoteListStore.cpp
 * @brief: 笔记列表数据存储与业务规则实现
 * @author: nuo
 * @date: 2026/10/9
 * @Detail:
 */

#include "NoteListStore.h"

INT32 NoteListStore::FindRow(INT64 s64NoteId) const
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

BOOL NoteListStore::ShouldShow(E_NOTE_TYPE eType) const
{
    return m_eCurNoteType == E_NOTE_TYPE_ALL || m_eCurNoteType == eType;
}

void NoteListStore::PushContainer(const st_NoteModelItem &Item)
{
    m_hashNoteData[Item.m_eNoteType].insert(Item.m_s64NoteId, Item);

    if (Item.m_eNoteType != E_NOTE_TYPE_ALL)
    {
        m_hashNoteData[E_NOTE_TYPE_ALL].insert(Item.m_s64NoteId, Item);
    }
}

void NoteListStore::SortNoteData()
{
    std::stable_sort(m_vecNote.begin(), m_vecNote.end(),
                     [](const st_NoteModelItem &a, const st_NoteModelItem &b) -> bool
    {
        if (a.m_bIsTop == TRUE && b.m_bIsTop == TRUE)
        {
            return a.m_s32TopOrder > b.m_s32TopOrder;
        }
        else if (a.m_bIsTop == FALSE && b.m_bIsTop == FALSE)
        {
            return a.m_s64NoteId < b.m_s64NoteId;
        }
        return a.m_bIsTop;
    });
}

void NoteListStore::Rebuild(const QVector<st_NoteModelItem> &vecItems)
{
    m_hashNoteData.clear();
    m_vecNote.clear();

    for (const auto &stItem : vecItems)
    {
        PushContainer(stItem);
        m_s64NextID = std::max(m_s64NextID, stItem.m_s64NoteId + 1);
        m_s32LastTopIndex = std::max(m_s32LastTopIndex, stItem.m_s32TopOrder);
    }

    m_vecNote = m_hashNoteData[m_eCurNoteType].values();
    SortNoteData();
}

void NoteListStore::SetCurNoteType(E_NOTE_TYPE eType)
{
    m_eCurNoteType = eType;
    m_vecNote = m_hashNoteData[m_eCurNoteType].values();
    SortNoteData();
}

st_NoteModelItem NoteListStore::Add(const std::string &strContent, INT64 s64WriteTime)
{
    st_NoteModelItem stItem;
    stItem.m_s64NoteId     = m_s64NextID++;
    stItem.m_s64WriteTime  = s64WriteTime;
    stItem.m_s64ModifyTime = s64WriteTime;
    stItem.m_strContent    = strContent;
    stItem.m_eNoteType     = m_eCurNoteType;

    PushContainer(stItem);
    m_vecNote.append(stItem);

    return stItem;
}

st_NoteModelItem NoteListStore::UpdateContent(INT32 s32Row, const std::string &strContent, INT64 s64ModifyTime)
{
    st_NoteModelItem &stItem = m_vecNote[s32Row];
    stItem.m_strContent    = strContent;
    stItem.m_s64ModifyTime = s64ModifyTime;

    PushContainer(stItem);

    return stItem;
}

st_NoteModelItem NoteListStore::SetRemind(INT32 s32Row, INT64 s64RemindTime,
                                          E_NOTE_REMIND_FREQUENCY eFreq, INT32 s32Interval)
{
    st_NoteModelItem &stItem = m_vecNote[s32Row];
    stItem.m_s64RemindTime     = s64RemindTime;
    stItem.m_eRemindFrequency  = eFreq;
    stItem.m_s32CustomInterval = s32Interval;

    PushContainer(stItem);

    return stItem;
}

st_NoteModelItem NoteListStore::SetLevel(INT32 s32Row, INT32 s32Level)
{
    st_NoteModelItem &stItem = m_vecNote[s32Row];
    stItem.m_eNoteLevel = static_cast<E_NOTE_EVENT_WAKEUP_LEVEL>(s32Level);

    PushContainer(stItem);

    return stItem;
}

st_NoteModelItem NoteListStore::PinTop(INT32 s32Row)
{
    st_NoteModelItem &stItem = m_vecNote[s32Row];
    stItem.m_bIsTop = TRUE;
    stItem.m_s32TopOrder = ++m_s32LastTopIndex;

    PushContainer(stItem);

    const st_NoteModelItem stChanged = stItem;
    SortNoteData();

    return stChanged;
}

st_NoteModelItem NoteListStore::ChangeCategory(INT32 s32Row, INT32 s32Type)
{
    const E_NOTE_TYPE eOldType = m_vecNote[s32Row].m_eNoteType;
    const E_NOTE_TYPE eNewType = static_cast<E_NOTE_TYPE>(s32Type);
    const INT64 s64NoteId = m_vecNote[s32Row].m_s64NoteId;

    st_NoteModelItem stItem = m_vecNote[s32Row];
    stItem.m_eNoteType = eNewType;

    m_hashNoteData[eOldType].remove(s64NoteId);
    PushContainer(stItem);

    if (ShouldShow(stItem.m_eNoteType))
    {
        m_vecNote[s32Row] = stItem;
    }
    else
    {
        m_vecNote.remove(s32Row);
    }

    return stItem;
}

st_NoteModelItem NoteListStore::Delete(INT32 s32Row)
{
    const INT64 s64NoteId = m_vecNote[s32Row].m_s64NoteId;

    m_vecNote[s32Row].m_bDeleted = TRUE;
    const st_NoteModelItem stRemoved = m_vecNote[s32Row];

    m_hashNoteData[stRemoved.m_eNoteType].remove(s64NoteId);
    m_hashNoteData[E_NOTE_TYPE_ALL].remove(s64NoteId);
    m_vecNote.remove(s32Row);

    return stRemoved;
}

BOOL NoteListStore::ApplyAdd(const st_NoteModelItem &stItem)
{
    PushContainer(stItem);

    m_s64NextID = std::max(m_s64NextID, stItem.m_s64NoteId + 1);

    if (ShouldShow(stItem.m_eNoteType))
    {
        m_vecNote.append(stItem);
        return TRUE;
    }
    return FALSE;
}

INT32 NoteListStore::ApplyUpdate(const st_NoteModelItem &stItem)
{
    const INT32 s32_Row = FindRow(stItem.m_s64NoteId);
    if (s32_Row < 0)
    {
        return -1;
    }

    m_vecNote[s32_Row] = stItem;
    PushContainer(stItem);

    return s32_Row;
}

INT32 NoteListStore::ApplyRemove(INT64 s64NoteId)
{
    const INT32 s32_Row = FindRow(s64NoteId);
    if (s32_Row < 0)
    {
        return -1;
    }

    const st_NoteModelItem stItem = m_vecNote.at(s32_Row);
    m_hashNoteData[stItem.m_eNoteType].remove(stItem.m_s64NoteId);
    m_hashNoteData[E_NOTE_TYPE_ALL].remove(stItem.m_s64NoteId);
    m_vecNote.remove(s32_Row);

    return s32_Row;
}
