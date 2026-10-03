#pragma once
#include <cstring>
#include <vector>
#include <memory>
#include <functional>
#include "datatype.h"
#include "DNoteCommon.h"


//#pragma pack(push, 1)
typedef struct st_NoteData
{
    // --- 基础信息 ---
    INT32                           m_s32id;                    ///< 便签唯一标识 ID (主键)
    char                            m_szContent[NoteSpace::CONTENT_LENGTH_MAX]; ///< 便签具体内容（使用定长数组以确保结构体偏移量固定）

    // --- 分类与等级 ---
    E_NOTE_EVENT_WAKEUP_LEVEL       m_eNoteLevel;               ///< 重要程度等级（四象限法则）
    E_NOTE_TIME_SPAN_TYPE           m_eTimeSpanType;            ///< 时间跨度类型（单次任务/长期任务）
    E_NOTE_TYPE                     m_eNoteType;               ///< 便签类型（生活、工作、学习、琐事、其他）

    // --- 时间信息 ---
    time_t                          m_s64RemindTime;            ///< 提醒触发时间 (Unix Timestamp, 0 表示不提醒)
    time_t                          m_s64CreateTime;              ///< 便签创建时间 (Unix Timestamp)
    time_t                          m_s64UpdateTime;            ///< 最后修改时间
    time_t                          m_S64LastRemindTime;        ///< 上一次提醒时间 (Unix Timestamp)
    time_t                          m_S64CompletionTime;        ///< 完成时间 (Unix Timestamp, 0 表示未完成)
    time_t                          m_S64DelayTriggerTime;      ///< 延迟触发时间 (Unix Timestamp, 0 表示无延迟)
    time_t                          m_S64DeleteTime;
    INT16U                          m_s16CustomInterval;        ///< 自定义间隔（单位：天）
    // --- 扩展数据 ---
    char                            m_cEvent[NoteSpace::NOTE_DATA_EVENT_COUNT];  ///< 关联事件或附件的指针数组
    E_NOTE_REMIND_FREQUENCY         m_eRemindFrequency;         ///< 提醒频率（不提醒/单次/每天/每周/每月/自定义）

    // --- 状态标记 ---
    BOOL                            m_bCompleted;               ///< 完成状态 (TRUE: 已完成 / FALSE: 进行中)
    BOOL                            m_bDeleted;                 ///< 软删除标记 (TRUE: 已回收 / FALSE: 正常)
    BOOL                            m_bSynced;                  ///<  同步状态 (TRUE: 已同步至云端 / FALSE: 本地待同步)
    BOOL                            m_bIsTop;                   ///< 置顶标记 (TRUE: 已置顶 / FALSE: 普通)
    INT32                           m_s32TopOrder;
    // --- 预留空间 ---
    char                            m_cReserved[FixedValueSpace::RESERVED_COUNT];  ///< 预留指针数组，用于未来扩展而不破坏二进制兼容性

    st_NoteData()
        : m_s32id(0)
        , m_eNoteLevel(E_NOTE_EVENT_WAKEUP_LEVEL_NORMAL)
        , m_eTimeSpanType(E_NOTE_TIME_SPAN_ONCE)
        , m_eNoteType(E_NOTE_TYPE_ALL)
        , m_s64RemindTime(0)
        , m_s64CreateTime(0)
        , m_s64UpdateTime(0)
        , m_S64LastRemindTime(-1)
        , m_S64CompletionTime(0)
        , m_S64DelayTriggerTime(0)
        , m_S64DeleteTime(-1)
        , m_s16CustomInterval(0)
        , m_eRemindFrequency(E_NOTE_REMIND_NONE)
        , m_bCompleted(FALSE)
        , m_bDeleted(FALSE)
        , m_bSynced(FALSE)
        , m_bIsTop(FALSE)
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
//#pragma pack(pop)

using NOTE_ITEM_PTR       = std::shared_ptr<const ST_NOTE_DATA>;
using NOTE_CACHE_SNAPSHOT = std::shared_ptr<const std::vector<NOTE_ITEM_PTR>>;

/** 缓存变更类型 */
enum class E_CACHE_CHANGE_TYPE
{
    E_CACHE_CHANGE_ADD = 0,   ///< 新增
    E_CACHE_CHANGE_UPDATE,    ///< 更新
    E_CACHE_CHANGE_REMOVE     ///< 移除
};

/** 缓存变更事件 */
struct SNoteCacheChange
{
    E_CACHE_CHANGE_TYPE eType     = E_CACHE_CHANGE_ADD;   ///< 变更类型
    INT32               s32NoteId = 0;                    ///< 目标笔记 id（REMOVE 时有效）
    NOTE_ITEM_PTR       pData;                            ///< 变更后的数据（ADD/UPDATE 时有效）
};

using CACHE_CHANGE_CALLBACK = std::function<void(const SNoteCacheChange &)>;

typedef struct st_FileHeaderBase
{
    INT64 m_s64FileStoreLimit;
    INT64 m_s64FileStoreCount;
    INT64 m_s64HeaderSize;
    INT64 m_s64SingleSTSize;
    INT64 m_s64NextId;
    st_FileHeaderBase(INT64 s64HeaderSize, INT64 s64SingleSTSize)
        : m_s64HeaderSize(s64HeaderSize)
        , m_s64SingleSTSize(s64SingleSTSize)
        , m_s64FileStoreLimit(0)
        , m_s64FileStoreCount(0)
        , m_s64NextId(0)
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
    INT64 GetNextId() const
    {
        return m_s64NextId;
    }
    void SetNextId(INT64 s64NextId)
    {
        m_s64NextId = s64NextId;
    }

}FILE_HEADER_BASE;
