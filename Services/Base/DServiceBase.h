#pragma once
#include <cstring>
#include <ctime>
#include "datatype.h"

namespace NoteSpace
{
    constexpr int NOTE_DATA_EVENT_COUNT = 1024; ///< 便签关联事件/附件指针数组的最大容量
    constexpr int CONTENT_LENGTH_MAX = 4096;   ///< 便签内容的最大长度限制（字节）
}

namespace FixedValueSpace
{
    constexpr int RESERVED_COUNT = 20; ///< 结构体末尾预留指针数组的容量，用于未来功能扩展
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

enum E_NOTE_TIME_SPAN_TYPE
{
    E_NOTE_TIME_SPAN_ONCE = 0,                      ///< 单次任务（一次性完成，有明确截止日期）
    E_NOTE_TIME_SPAN_LONG_TERM,                     ///< 长期任务（持续性习惯、周期性目标）

    E_NOTE_TIME_SPAN_MAX                            ///< 边界值
};

enum E_NOTE_REMIND_FREQUENCY
{
    E_NOTE_REMIND_NONE = 0,                         ///< 不提醒
    E_NOTE_REMIND_ONCE,                             ///< 单次提醒（到达指定时间触发一次）
    E_NOTE_REMIND_DAILY,                            ///< 每天提醒
    E_NOTE_REMIND_WEEKLY,                           ///< 每周提醒
    E_NOTE_REMIND_MONTHLY,                          ///< 每月提醒
    E_NOTE_REMIND_CUSTOM,                           ///< 自定义间隔（需配合 m_s64CustomInterval 使用）

    E_NOTE_REMIND_MAX                               ///< 边界值
};

#pragma pack(push, 1)
typedef struct st_NoteData
{
    // --- 基础信息 ---
    INT32                           m_s32id;                    ///< 便签唯一标识 ID (主键)
    char                            m_szContent[NoteSpace::CONTENT_LENGTH_MAX]; ///< 便签具体内容（使用定长数组以确保结构体偏移量固定）

    // --- 分类与等级 ---
    E_NOTE_EVENT_WAKEUP_LEVEL       m_eNoteLevel;               ///< 重要程度等级（四象限法则）
    E_NOTE_EVENT_TYPE               m_eEventType;               ///< 事件类型（生活/琐事/工作/学习）
    E_NOTE_TIME_SPAN_TYPE           m_eTimeSpanType;            ///< 时间跨度类型（单次任务/长期任务）

    // --- 时间信息 ---
    time_t                          m_s64RemindTime;            ///< 提醒触发时间 (Unix Timestamp, 0 表示不提醒)
    time_t                          m_s64NoteTime;              ///< 便签创建时间 (Unix Timestamp)
    time_t                          m_s64UpdateTime;            ///< 最后修改时间

    // --- 扩展数据 ---
    char                            m_cEvent[NoteSpace::NOTE_DATA_EVENT_COUNT];  ///< 关联事件或附件的指针数组
    E_NOTE_REMIND_FREQUENCY         m_eRemindFrequency;         ///< 提醒频率（不提醒/单次/每天/每周/每月/自定义）

    // --- 状态标记 ---
    BOOL                            m_bCompleted;               ///< 完成状态 (TRUE: 已完成 / FALSE: 进行中)
    BOOL                            m_bDeleted;                 ///< 软删除标记 (TRUE: 已回收 / FALSE: 正常)
    BOOL                            m_bSynced;                  ///<  同步状态 (TRUE: 已同步至云端 / FALSE: 本地待同步)

    // --- 预留空间 ---
    char                            m_cReserved[FixedValueSpace::RESERVED_COUNT];  ///< 预留指针数组，用于未来扩展而不破坏二进制兼容性

    st_NoteData()
        : m_s32id(0)
        , m_eNoteLevel(E_NOTE_EVENT_WAKEUP_LEVEL_NORMAL)
        , m_eEventType(E_NOTE_EVENT_NONE)
        , m_eTimeSpanType(E_NOTE_TIME_SPAN_ONCE)
        , m_s64RemindTime(0)
        , m_s64NoteTime(0)
        , m_s64UpdateTime(0)
        , m_eRemindFrequency(E_NOTE_REMIND_NONE)
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

} ST_NOTE_DATA;
#pragma pack(pop)


typedef struct st_FileHeaderBase
{
    INT64 m_s64FileStoreLimit;
    INT64 m_s64FileStoreCount;
    INT64 m_s64HeaderSize;
    INT64 m_s64SingleSTSize;
    st_FileHeaderBase(INT64 s64HeaderSize, INT64 s64SingleSTSize)
        : m_s64HeaderSize(s64HeaderSize)
        , m_s64SingleSTSize(s64SingleSTSize)
        , m_s64FileStoreLimit(0)
        , m_s64FileStoreCount(0)
    {
    }

    INT64 GetStoreLimit() const
    {
        return m_s64FileStoreLimit;
    }
    INT64 GetStoreCount() const
    {
        return m_s64FileStoreCount;
    }
    INT64 GetHeaderSize() const
    {
        return m_s64HeaderSize;
    }
    INT64 GetSingleSTSize() const
    {
        return m_s64SingleSTSize;
    }
    void SetStoreCount(INT64 s64StoreCount)
    {
        m_s64FileStoreCount = s64StoreCount;
    }


}FILE_HEADER_BASE;
