/*
 * @file: CDataCollectBase.h
 * @brief: 环形缓冲区基类（SPSC 无锁）
 * @author: nuo
 * @date: 2026/6/22
 * @Detail: 单生产者单消费者模型。写计数只由生产者更新、读计数只由消费者更新，
 *          两个方向各用 release/acquire 配对保证数据可见性，全程不需要加锁。
 * @note    线程约束（必须遵守，否则会破坏 SPSC 假设）：
 *          - SetBuffer      只能由生产者线程调用
 *          - GetBuffer      只能由消费者线程调用
 *          - DiscardAll     只能由消费者线程调用
 *          - HasUnsavedData / GetBufferSize  任意线程可调（只读）
 */

#pragma once
#include "datatype.h"
#include <atomic>
#include <memory>
#include <algorithm>
#include <cstring>

class CDataCollectBase
{
public:
    explicit CDataCollectBase(INT32 s32BufferSize)
        : m_cBuffer(std::make_unique<char[]>(s32BufferSize))
        , m_s32BufferSize(s32BufferSize)
    {
    }

    virtual ~CDataCollectBase() = default;

    /** ***********************************************************
     * @brief       写入数据（生产者线程独占调用）
     * @param[in]   s32Size   数据字节数
     * @param[in]   pcBuffer  数据指针
     * @return      写入的字节数；空间不足返回 -1（该条数据被丢弃）
     ************************************************************/
    INT32 SetBuffer(INT32 s32Size, const char *pcBuffer)
    {
        if (pcBuffer == nullptr || s32Size <= 0 || s32Size > m_s32BufferSize)
        {
            return -1;
        }

        const INT64 s64Write = m_atomWriteCount.load(std::memory_order_relaxed);
        const INT64 s64Read  = m_atomReadCount.load(std::memory_order_acquire);

        // 剩余空间 = 容量 − 已用
        if (s64Write - s64Read + s32Size > m_s32BufferSize)
        {
            return -1;
        }

        const INT32 s32Pos   = static_cast<INT32>(s64Write % m_s32BufferSize);
        const INT32 s32First = std::min(s32Size, m_s32BufferSize - s32Pos);

        memcpy(m_cBuffer.get() + s32Pos, pcBuffer, s32First);
        if (s32First < s32Size)
        {
            memcpy(m_cBuffer.get(), pcBuffer + s32First, s32Size - s32First);
        }

        // 先写完数据，再发布写计数：release 保证数据对消费者可见
        m_atomWriteCount.store(s64Write + s32Size, std::memory_order_release);
        return s32Size;
    }

    /** ***********************************************************
     * @brief       读取数据（消费者线程独占调用）
     * @param[in]   s32GetSize  期望读取的字节数
     * @param[out]  pcBuffer    输出缓冲区
     * @return      实际读取的字节数；无数据返回 0
     ************************************************************/
    INT32 GetBuffer(INT32 s32GetSize, char *pcBuffer)
    {
        if (pcBuffer == nullptr || s32GetSize <= 0)
        {
            return 0;
        }
        memset(pcBuffer, 0, s32GetSize);

        const INT64 s64Read  = m_atomReadCount.load(std::memory_order_relaxed);
        const INT64 s64Write = m_atomWriteCount.load(std::memory_order_acquire);

        const INT64 s64Avail = s64Write - s64Read;
        if (s64Avail <= 0)
        {
            return 0;
        }

        const INT32 s32ReadSize = static_cast<INT32>(std::min<INT64>(s32GetSize, s64Avail));
        const INT32 s32Pos      = static_cast<INT32>(s64Read % m_s32BufferSize);
        const INT32 s32First    = std::min(s32ReadSize, m_s32BufferSize - s32Pos);


        memcpy(pcBuffer, m_cBuffer.get() + s32Pos, s32First);
        if (s32First < s32ReadSize)
        {
            memcpy(pcBuffer + s32First, m_cBuffer.get(), s32ReadSize - s32First);
        }

        // 先读完数据，再发布读计数
        m_atomReadCount.store(s64Read + s32ReadSize, std::memory_order_release);
        return s32ReadSize;
    }

    /** ***********************************************************
     * @brief       丢弃所有未读数据（消费者线程独占调用）
     ************************************************************/
    void DiscardAll()
    {
        const INT64 s64Write = m_atomWriteCount.load(std::memory_order_acquire);
        m_atomReadCount.store(s64Write, std::memory_order_release);
    }

    /** 是否有未读数据（只读，任意线程可调） */
    bool HasUnsavedData() const
    {
        return m_atomWriteCount.load(std::memory_order_acquire)
             > m_atomReadCount.load(std::memory_order_acquire);
    }

    /** 缓冲区容量（字节） */
    INT32 GetBufferSize() const
    {
        return m_s32BufferSize;
    }

private:
    std::unique_ptr<char[]>  m_cBuffer;              ///< 环形缓冲区
    const INT32              m_s32BufferSize;        ///< 缓冲区总大小（字节）

    std::atomic<INT64>       m_atomWriteCount{0};    ///< 累计写入字节数，只被生产者更新
    std::atomic<INT64>       m_atomReadCount{0};     ///< 累计读出字节数，只被消费者更新
};
