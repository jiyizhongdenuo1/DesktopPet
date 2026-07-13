/*
 * @file: CDataCollectBase.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/22
 * @Detail:
 */

#pragma once
#include "datatype.h"
#include <memory>
#include <cstring>

class CDataCollectBase
{
public:
    explicit CDataCollectBase(INT32 s32BufferSize)
        : m_cBuffer(std::make_shared<char[]>(s32BufferSize))
        , m_s32BufferSize(s32BufferSize)
    {
    }

    virtual ~CDataCollectBase() = default;

    INT32 GetBuffer(INT32 s32GetSize, char *pcBuffer)
    {
        if (pcBuffer == nullptr || m_cBuffer == nullptr || s32GetSize <= 0)
        {
            return -1;
        }
        INT32 s32_ReadSize = std::min(s32GetSize, m_s32UnSaveSize);
        if (s32_ReadSize + m_s32ReadPos <= m_s32BufferSize)
        {
            memcpy(pcBuffer, m_cBuffer.get() + m_s32ReadPos, s32_ReadSize);
            m_s32ReadPos += s32_ReadSize;
        }
        else
        {
            memcpy(pcBuffer, m_cBuffer.get(), m_s32BufferSize - m_s32ReadPos);
            memcpy(pcBuffer + m_s32BufferSize - m_s32ReadPos, m_cBuffer.get(), s32_ReadSize - m_s32BufferSize + m_s32ReadPos);
            m_s32ReadPos = s32_ReadSize + m_s32ReadPos % m_s32BufferSize;
        }
        m_s32UnSaveSize -= s32_ReadSize;
        return s32_ReadSize;
    }

    INT32 SetBuffer(INT32 s32Size, const char *pcBuffer)
    {
        if (s32Size > 0 && s32Size <= m_s32BufferSize && pcBuffer != nullptr && m_cBuffer != nullptr)
        {
            if (s32Size + m_s32CurrentPos <= m_s32BufferSize)
            {
                memcpy(m_cBuffer.get() + m_s32CurrentPos, pcBuffer, s32Size);
                m_s32UnSaveSize += s32Size;
                m_s32CurrentPos += s32Size;
            }
            else
            {
                INT32 s32_UnSaveSize = s32Size + m_s32CurrentPos - m_s32BufferSize;
                memcpy(m_cBuffer.get() + m_s32CurrentPos, pcBuffer, m_s32BufferSize - m_s32CurrentPos);
                memcpy(m_cBuffer.get(), pcBuffer + s32_UnSaveSize, s32Size - s32_UnSaveSize);
                m_s32UnSaveSize += s32Size;
                m_s32CurrentPos = s32Size + m_s32CurrentPos % m_s32BufferSize;
            }
        }
        else
        {
            return -1;
        }
        return s32Size;
    }

    INT32 GetBufferSize() const
    {
        return m_s32BufferSize;
    }


private:
    std::shared_ptr<char[]>     m_cBuffer;           ///< 环形缓冲区指针
    const INT32                 m_s32BufferSize;     ///< 缓冲区总大小
    const INT32                 m_s32SingleSize = 0; ///< 单条数据大小（预留）

    INT32                       m_s32UnSaveSize = 0; ///< 未保存数据的大小
    INT32                       m_s32CurrentPos = 0; ///< 当前写入位置
    INT32                       m_s32ReadPos = 0;    ///< 当前读取位置
};