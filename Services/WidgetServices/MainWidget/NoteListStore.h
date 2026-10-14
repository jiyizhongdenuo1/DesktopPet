/*
 * @file: NoteListStore.h
 * @brief: 笔记列表数据存储与业务规则
 * @author: nuo
 * @date: 2026/10/9
 * @Detail: 持有笔记数据与分类索引，提供增删改查与排序；不涉及 Qt 的信号槽机制
 */

#pragma once

#include <algorithm>
#include <string>

#include <QVector>
#include <QHash>
#include <QMap>

#include "DDataMgrBase.h"
#include "DServiceBase.h"

class NoteListStore
{
public:
    NoteListStore() = default;

    /** 当前显示列表的行数 */
    INT32 Count() const { return m_vecNote.size(); }

    /** 取指定行的数据 */
    const st_NoteModelItem &ItemAt(INT32 s32Row) const { return m_vecNote.at(s32Row); }

    /** 在已显示列表中查找指定 id 的行号，未找到返回 -1 */
    INT32 FindRow(INT64 s64NoteId) const;

    /** 当前视图分类 */
    E_NOTE_TYPE CurNoteType() const { return m_eCurNoteType; }

    /** 下一条笔记 id */
    INT64 NextId() const { return m_s64NextID; }

    /** 判断指定分类是否属于当前视图 */
    BOOL ShouldShow(E_NOTE_TYPE eType) const;

    /** 用快照数据整体重建 */
    void Rebuild(const QVector<st_NoteModelItem> &vecItems);

    /** 切换视图分类并重建显示列表 */
    void SetCurNoteType(E_NOTE_TYPE eType);

    void SetNextId(INT64 s64NextId) { m_s64NextID = s64NextId; }

    /** 新增一条笔记，返回新数据 */
    st_NoteModelItem Add(const std::string &strContent, INT64 s64WriteTime);

    /** 更新笔记内容，返回更新后的数据 */
    st_NoteModelItem UpdateContent(INT32 s32Row, const std::string &strContent, INT64 s64ModifyTime);

    /** 设置提醒时间与频率，返回更新后的数据 */
    st_NoteModelItem SetRemind(INT32 s32Row, INT64 s64RemindTime,
                               E_NOTE_REMIND_FREQUENCY eFreq, INT32 s32Interval);

    /** 设置优先级，返回更新后的数据 */
    st_NoteModelItem SetLevel(INT32 s32Row, INT32 s32Level);

    /** 置顶并重排，返回更新后的数据 */
    st_NoteModelItem PinTop(INT32 s32Row);

    /** 修改分类；新分类不属于当前视图时从列表移除。返回更新后的数据 */
    st_NoteModelItem ChangeCategory(INT32 s32Row, INT32 s32Type);

    /** 删除一条笔记，返回被删除的数据 */
    st_NoteModelItem Delete(INT32 s32Row);

    /** 应用一条新增变更，返回是否加入当前显示列表 */
    BOOL ApplyAdd(const st_NoteModelItem &stItem);

    /** 应用一条更新变更，返回所在行号；未找到返回 -1 */
    INT32 ApplyUpdate(const st_NoteModelItem &stItem);

    /** 应用一条移除变更，返回被移除的行号；未找到返回 -1 */
    INT32 ApplyRemove(INT64 s64NoteId);

private:
    void PushContainer(const st_NoteModelItem &Item);
    void SortNoteData();

private:
    QHash<INT32, QMap<INT64, st_NoteModelItem>> m_hashNoteData;
    QVector<st_NoteModelItem>                   m_vecNote;
    E_NOTE_TYPE                                 m_eCurNoteType = E_NOTE_TYPE_ALL;
    INT64                                       m_s64NextID = 0;
    INT32                                       m_s32LastTopIndex = 0;
};
