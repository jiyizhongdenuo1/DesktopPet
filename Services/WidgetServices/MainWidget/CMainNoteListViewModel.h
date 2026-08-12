/*
* @file: CMainNoteListViewModel.h
 * @brief: 笔记列表视图模型
 * @author: nuo
 * @date: 2026/5/10
 * @Detail: 负责管理 st_NoteData 数据并与 QML 进行交互
 */

#pragma once

#include <QAbstractListModel>
#include <QQueue>

#include "datatype.h"
#include "DDataMgrBase.h"
#include "DDataCache.h"

class QDataTime;
class CNoteDataService;

class CMainNoteListViewModel : public QAbstractListModel
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit CMainNoteListViewModel(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~CMainNoteListViewModel() = default;

    /**
     * @brief 笔记列表的角色枚举
     * 用于 QML 端访问 model 数据
     */
    enum ENUM_ROLE_NOTELIST
    {
        RoleNoteId = Qt::UserRole + 1,     ///< 笔记ID
        RoleNoteContent,                   ///< 笔记内容
        RoleNoteLevel,                     ///< 重要程度等级（四象限）
        RoleNoteTimeSpanType,              ///< 时间跨度类型（单次/长期）
        RoleNoteWriteTime,                 ///< 写入时间
        RoleNoteModifyTime,                ///< 修改时间
        RoleNoteRemindTime,                ///< 提醒时间
        RoleNoteRemindFrequency,           ///< 提醒频率（不提醒/单次/每天/每周/每月）
        RoleNoteCompleted,                 ///< 是否已完成
        RoleNoteDeleted,                   ///< 是否已删除
        RoleNoteType                       ///< 事件类型（生活/琐事/工作/学习）
    };

    /**
     * @brief 获取列表行数
     * @param parent 父索引
     * @return 列表行数
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief 获取指定索引和角色的数据
     * @param index 数据索引
     * @param role 数据角色
     * @return 数据值
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief 获取角色名称映射（用于 QML 访问）
     * @return 角色名称哈希表
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief 添加新笔记（QML 可调用）
     * @param strContent 笔记内容
     * @param time 写入时间
     */
    Q_INVOKABLE void AddNote(const QString &strContent, const QDateTime &time);

    /**
     * @brief 更新笔记内容（QML 可调用）
     * @param index 笔记索引
     * @param newContent 新内容
     */
    Q_INVOKABLE void UpdateNoteContent(int index, const QString &newContent);
    void PutArrNoteData(std::shared_ptr<std::array<ST_NOTE_DATA, DDataCache::MAX_CACHE_SIZE>> pArrData, INT32 s32Count);
private:
    void Init();
    void InitService();
private:
    QHash<INT32, QVector<st_NoteModelItem>> m_hashNoteData;
    std::shared_ptr<CNoteDataService> m_pNoteService;
    QVector<st_NoteModelItem> m_vecNote;
};