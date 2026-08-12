/*
 * @file: CNoteDataCollect.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/23
 * @Detail:
 */

#pragma once

#include "DDataMgrBase.h"
#include "datatype.h"
#include "CDataCollectBase.h"
#include "INoteDataBuffer.h"

class CNoteDataColloctPrivate;

/** ***********************************************************
 * @brief       笔记数据收集器（实现 INoteDataBuffer 接口）
 * @Detail:     继承 CDataCollectBase（缓冲区管理）+ 实现 INoteDataBuffer（标准接口）
 ************************************************************/
class CNoteDataCollect: public CDataCollectBase, public INoteDataBuffer
{
public:
    explicit CNoteDataCollect(INT32 s32BufferSize);

    ~CNoteDataCollect() override;

    // ========== INoteDataBuffer 接口实现 ==========

    /** ***********************************************************
     * @brief       追加UI层数据到缓冲区（写入端）
     * @param[in]   stModelItem UI层数据结构
     ************************************************************/
    void AppendData(const NOTE_MODEL_ITEM &stModelItem) override;

    /** ***********************************************************
     * @brief       从缓冲区读取原始字节数据（读取端）
     * @param[in]   s32GetSize 请求读取的大小
     * @param[out]  pcBuffer 输出缓冲区
     * @return      实际读取的字节数，-1 表示错误
     ************************************************************/
    INT32 ReadBuffer(INT32 s32GetSize, char *pcBuffer) override;

    /** ***********************************************************
     * @brief       检查缓冲区是否有未读数据
     * @return      true 有数据 / false 无数据
     ************************************************************/
    bool HasData() const override;

    /** ***********************************************************
     * @brief       清空缓冲区
     ************************************************************/
    void Clear() override;

    /** ***********************************************************
     * @brief       获取缓冲区总大小
     * @return      缓冲区容量（字节）
     ************************************************************/
    INT32 GetBufferSize() const override;
};