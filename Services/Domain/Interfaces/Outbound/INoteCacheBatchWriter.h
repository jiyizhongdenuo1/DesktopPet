/*
 * @file: INoteCacheBatchWriter.h
 * @brief: 笔记缓存批量写入接口（数据线程使用）
 * @author: nuo
 * @date: 2026/10/9
 * @Detail: 数据线程把落盘数据回灌缓存时使用的出站端口
 */

#pragma once

#include "DServiceBase.h"

class INoteCacheBatchWriter
{
public:
    virtual ~INoteCacheBatchWriter() = default;

    /** 批量追加笔记数据到缓存 */
    virtual void AppendNoteData(const ST_NOTE_DATA *pArrNote, INT32 s32Count) = 0;

    /** 批量追加回收站数据到缓存 */
    virtual void AppendRecycleNoteData(const ST_NOTE_DATA *pArrNote, INT32 s32Count) = 0;
};
