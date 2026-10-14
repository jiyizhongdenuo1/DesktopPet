/*
 * @file: INoteDataLoader.h
 * @brief: 笔记数据加载接口（入站端口）
 * @author: nuo
 * @date: 2026/10/9
 * @Detail: 数据线程把从文件读出的字节流交给应用层解析入库
 */

#pragma once

#include "datatype.h"

class INoteDataLoader
{
public:
    virtual ~INoteDataLoader() = default;

    /** ***********************************************************
     * @brief       将原始字节缓冲区解析为笔记数据并存入缓存
     * @param[in]   pBuffer        原始字节数据缓冲区
     * @param[in]   s32BufferSize  缓冲区大小（字节）
     * @return      成功存入缓存的数据条数，失败返回0
     ************************************************************/
    virtual INT32 LoadFromBuffer(char *pBuffer, INT32 s32BufferSize) = 0;

    /** ***********************************************************
     * @brief       将原始字节缓冲区解析为回收站数据并存入回收站缓存
     * @param[in]   pBuffer        原始字节数据缓冲区
     * @param[in]   s32BufferSize  缓冲区大小（字节）
     * @return      成功存入回收站缓存的条数，失败返回0
     ************************************************************/
    virtual INT32 LoadRecycleFromBuffer(char *pBuffer, INT32 s32BufferSize) = 0;
};
