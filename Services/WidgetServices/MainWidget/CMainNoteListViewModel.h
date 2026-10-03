/*
* @file: CMainNoteListViewModel.h
 * @brief: 笔记列表视图模型
 * @author: nuo
 * @date: 2026/5/10
 * @Detail: 负责管理 st_NoteData 数据并与 QML 进行交互
 */

#pragma once

#include <memory>
#include <QAbstractListModel>

#include "datatype.h"
#include "DDataMgrBase.h"
#include "DDataCache.h"

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

    Q_INVOKABLE void SetNoteRemind(int index, const QDateTime &time);
    Q_INVOKABLE void SetNoteLevel(int index, int level);
    Q_INVOKABLE void SetNoteType(int index, int type);
    Q_INVOKABLE void DeleteNote(int index);
    Q_INVOKABLE void PinTop(int index);

    void PutArrNoteData(NOTE_CACHE_SNAPSHOT pSnapshot);

    void UpdateNoteIndex(INT32U u32Index);
    void SetNextID(INT64 s64NextID);

    /**
     * @brief 注入数据服务并注册数据加载回调
     * @param pService 笔记数据服务实例
     */
    void Init(std::shared_ptr<CNoteDataService> pService);
private:
    void PushContainer(const st_NoteModelItem& Item);

    /** 把领域数据结构转换为界面模型结构 */
    st_NoteModelItem ConvertToModel(const ST_NOTE_DATA &st_Src) const;

    /** 在已显示列表中查找指定 id 的行号，未找到返回 -1 */
    INT32 FindRow(INT64 s64NoteId) const;

    /** 应用一条增量变更到列表模型 */
    void ApplyChange(const SNoteCacheChange &stChange);
private:
    QHash<INT32, QMap<INT64, st_NoteModelItem>> m_hashNoteData;

    std::shared_ptr<CNoteDataService>       m_pNoteService;
    QVector<st_NoteModelItem>               m_vecNote;
    E_NOTE_TYPE                             m_eCurNoteType;
    INT64                                   m_s64NextID;
    INT32                                   m_LastTopIndex;
};
