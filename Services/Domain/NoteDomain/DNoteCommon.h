#pragma once

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

namespace DDataCache
{
    constexpr static INT32 MAX_CACHE_SIZE = 500;
}
namespace CSpaceTime
{
    constexpr static INT32 COMPACT_TIME_SPACE = 12 * 60 * 60;
}
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

enum E_NOTE_TYPE
{
    E_NOTE_TYPE_ALL      = 0xFFFF,
    E_NOTE_TYPE_TWO      = 0x0002,
    E_NOTE_TYPE_THREE    = 0x0004,
    E_NOTE_TYPE_FOUR     = 0x0008,
    E_NOTE_TYPE_FIVE     = 0x0010,
    E_NOTE_TYPE_SIX      = 0x0020,
    E_NOTE_TYPE_SEVEN    = 0x0040,
    E_NOTE_TYPE_EIGHT    = 0x0080,
    E_NOTE_TYPE_NINE     = 0x0100,
    E_NOTE_TYPE_TEN      = 0x0200,
    E_NOTE_TYPE_ELEVEN   = 0x0400,
    E_NOTE_TYPE_TWELVE   = 0x0800,
    E_NOTE_TYPE_THIRTEEN = 0x1000,
    E_NOTE_TYPE_FOURTEEN = 0x2000,
    E_NOTE_TYPE_FIFTEEN  = 0x4000,
    E_NOTE_TYPE_MAX      = 0x8000,
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
