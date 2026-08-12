/*
 * @file: CDataRWMgr.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/12
 * @Detail:
 */

#pragma once

#include <memory>
#include "Business_G.h"
#include "datatype.h"

class CDataSave;
class CDataRWMgrPrivate;

/** ***********************************************************
 * @brief       数据保存服务（业务逻辑层）
 * @Detail:     封装笔记数据的保存逻辑，通过依赖注入接收共享的
 *              DataSave 实例，确保文件操作的一致性
 ************************************************************/
class BUSINESS_EXPORT CDataRWMgr
{
public:
    /** ***********************************************************
     * @brief       构造函数（支持依赖注入）
     * @param[in]   pDataSaver 共享的DataSave实例（由组合根注入）
     * @note        必须传入有效的 DataSave 实例，否则运行时会抛出异常。
     *              内部使用拷贝语义（shared_ptr），调用者仍持有有效引用
     ************************************************************/
    explicit CDataRWMgr(std::shared_ptr<CDataSave> pDataSaver);

    ~CDataRWMgr();

    /** ***********************************************************
     * @brief       保存笔记数据到文件
     * @param[in]   pData 待保存的数据缓冲区
     * @param[in]   s32Size 数据大小（字节）
     * @return      void
     * @note        数据由外部提供，内部只负责写入文件
     ************************************************************/
    void WriteToFile(const char *pData, INT32 s32Size) const;

    /** ***********************************************************
     * @brief       从文件读取原始字节数据（纯IO操作）
     * @param[out]  pBuffer        输出缓冲区（由调用者分配）
     * @param[in]   s32BufferSize 缓冲区大小（字节）
     * @param[out]  s32DataSize   实际读取的数据大小（字节）
     * @return      void
     * @note        只负责字节级读写，不进行结构体转换；线程安全
     ************************************************************/
    void ReadFromFile(char *pBuffer, INT32 s32BufferSize, INT32 &s32DataSize) const;
private:
    std::unique_ptr<CDataRWMgrPrivate> d_ptr;
};