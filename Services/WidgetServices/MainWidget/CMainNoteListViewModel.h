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

class QDataTime;
class CNoteDataCollect;

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
        RoleNoteLevel,                     ///< 笔记级别
        RoleNoteWriteTime,                 ///< 写入时间
        RoleNoteModifyTime,                ///< 修改时间
        RoleNoteRemindTime,                ///< 提醒时间
        RoleNoteRemindType,                ///< 提醒类型
        RoleNoteCompleted,                 ///< 是否已完成
        RoleNoteDeleted,                   ///< 是否已删除
        RoleNoteType                       ///< 笔记类型
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

private:
    void Init();
private:
    QList<st_NoteModelItem> m_listNote;
    std::shared_ptr<CNoteDataCollect> m_pNoteDataCollect;
};