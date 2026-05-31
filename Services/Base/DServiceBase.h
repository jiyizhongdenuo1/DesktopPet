#pragma once
#include <string>
#include <cstring>
#include <ctime>
#include "datatype.h"

namespace NoteSpace
{
    constexpr int s32NoteDataEventCount = 1024; ///< 便签关联事件/附件指针数组的最大容量
    constexpr int s32MaxContentLength = 4096;   ///< 便签内容的最大长度限制（字节）
}

namespace FixedValueSpace
{
    constexpr int s32ReservedCount = 20; ///< 结构体末尾预留指针数组的容量，用于未来功能扩展
}


enum E_NOTE_EVENT_TYPE
{
    E_NOTE_EVENT_NONE = 0,      ///< 未分类/不记名
    E_NOTE_EVENT_LIVE,          ///< 生活事件（如：生日、纪念日）
    E_NOTE_EVENT_THING,         ///< 琐事/任务（如：买菜、取快递）
    E_NOTE_EVENT_WORK,          ///< [扩充] 工作事项
    E_NOTE_EVENT_STUDY,         ///< [扩充] 学习记录

    E_NOTE_EVENT_MAX            ///< 边界值，用于合法性校验
};

enum E_NOTE_EVENT_WAKEUP_LEVEL
{
    E_NOTE_EVENT_WAKEUP_LEVEL_IMPORTANT_URGENT = 0, ///< 第一象限：重要且紧急（立即执行）
    E_NOTE_EVENT_WAKEUP_LEVEL_URGENT,               ///< 第二象限：紧急但不重要（授权或快速处理）
    E_NOTE_EVENT_WAKEUP_LEVEL_IMPORTANT,            ///< 第三象限：重要但不紧急（制定计划）
    E_NOTE_EVENT_WAKEUP_LEVEL_NORMAL,               ///< 第四象限：不重要不紧急（闲暇处理）

    E_NOTE_EVENT_WAKEUP_LEVEL_MAX                   ///< 边界值
};

#pragma pack(push, 1)
typedef struct st_NoteData
{
    // --- 基础信息 ---
    INT32                           m_s32id;                    ///< 便签唯一标识 ID (主键)
    char                            m_szContent[NoteSpace::s32MaxContentLength]; ///< 便签具体内容（使用定长数组以确保结构体偏移量固定）

    // --- 分类与等级 ---
    E_NOTE_EVENT_WAKEUP_LEVEL       m_eNoteLevel;               ///< 重要程度等级
    E_NOTE_EVENT_TYPE               m_eEventType;               ///< 事件类型

    // --- 时间信息 ---
    time_t                          m_s64RemindTime;            ///< 提醒触发时间 (Unix Timestamp, 0 表示不提醒)
    time_t                          m_s64NoteTime;              ///< 便签创建时间 (Unix Timestamp)
    time_t                          m_s64UpdateTime;            ///< 最后修改时间

    // --- 扩展数据 ---
    char *                          m_cEvent[NoteSpace::s32NoteDataEventCount];  ///< 关联事件或附件的指针数组
    INT32                           m_s32RemindLevel;           ///< 提醒重复级别（如：每天、每周、每月）

    // --- 状态标记 ---
    BOOL                            m_bCompleted;               ///< 完成状态 (TRUE: 已完成 / FALSE: 进行中)
    BOOL                            m_bDeleted;                 ///< 软删除标记 (TRUE: 已回收 / FALSE: 正常)
    BOOL                            m_bSynced;                  ///<  同步状态 (TRUE: 已同步至云端 / FALSE: 本地待同步)

    // --- 预留空间 ---
    char *                          m_cReserved[FixedValueSpace::s32ReservedCount];  ///< 预留指针数组，用于未来扩展而不破坏二进制兼容性

    st_NoteData()
        : m_s32id(0)
        , m_eNoteLevel(E_NOTE_EVENT_WAKEUP_LEVEL_NORMAL)
        , m_eEventType(E_NOTE_EVENT_NONE)
        , m_s64RemindTime(0)
        , m_s64NoteTime(0)
        , m_s64UpdateTime(0)
        , m_s32RemindLevel(0)
        , m_bCompleted(FALSE)
        , m_bDeleted(FALSE)
        , m_bSynced(FALSE)
    {
        std::memset(m_szContent, 0, sizeof(m_szContent));
        std::memset(m_cEvent, 0, sizeof(m_cEvent));
        std::memset(m_cReserved, 0, sizeof(m_cReserved));
    }

    void Reset()
    {
        std::memset(this, 0, sizeof(st_NoteData));
    }

} STNOTEDATA;
#pragma pack(pop)