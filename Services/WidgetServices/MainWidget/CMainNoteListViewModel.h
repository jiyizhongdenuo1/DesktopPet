/*
* @file: CMainNoteListViewModel.h
* @brief: 笔记列表视图模型
* @author: nuo
* @date: 2026/5/10
* @Detail: 负责管理 st_NoteData 数据并与 QML 进行交互
*/

#pragma once

#include <functional>
#include <memory>

#include <QAbstractListModel>
#include <QMap>
#include <QVariant>

#include "datatype.h"
#include "DDataMgrBase.h"
#include "DDataCache.h"
#include "NoteListStore.h"

class CNoteDataService;

class CMainNoteListViewModel : public QAbstractListModel
{
    Q_OBJECT
public:
    /**
     * @brief 右键菜单操作类型
     * 取值与 QML 端 NoteContextMenu.E_NOTE_ACTION 的顺序保持一致
     */
    enum class E_NOTE_ACTION : INT32
    {
        E_NOTE_ACTION_TOP_WINDOW = 0,
        E_NOTE_ACTION_CATEGORY   = 1,
        E_NOTE_ACTION_REMIND     = 2,
        E_NOTE_ACTION_SET_LEVEL  = 3,
        E_NOTE_ACTION_PIN_NOTE   = 4,
        E_NOTE_ACTION_DELETE     = 5
    };

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
     * @brief 执行右键菜单操作（QML 可调用）
     * @param s32Action 操作类型，取值见 E_NOTE_ACTION
     * @param index 目标笔记行号
     * @param value 操作参数，按操作类型自行解释
     */
    Q_INVOKABLE void ExecNoteAction(INT32 s32Action, INT32 index, const QVariant &value);

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

    Q_INVOKABLE void SetNoteRemind(int index, const QDateTime &time, int repeatIndex, int interval);
    Q_INVOKABLE void SetNoteLevel(int index, int level);
    Q_INVOKABLE void ChangeNoteCategory(int index, int type);
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
    using NoteActionHandler = std::function<void(INT32 index, const QVariant &value)>;

    /** 注册右键菜单各操作的处理器 */
    void RegisterNoteActions();

    /** 应用一条增量变更到列表模型 */
    void ApplyChange(const SNoteCacheChange &stChange);

    /** 把变更写回数据服务 */
    void UpdateNoteFile(const st_NoteModelItem &Item);

private:
    NoteListStore                     m_store;
    std::shared_ptr<CNoteDataService> m_pNoteService;
    QMap<INT32, NoteActionHandler>    m_mapActions;
};
