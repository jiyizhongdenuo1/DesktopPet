/*
 * @file: CNoteDataRW.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/17
 * @Detail:
 */

#pragma once
#include <memory>
#include <QVariantHash>

#include "CommonDefine.h"
#include "DServiceBase.h"

class CDataSaveRWMgr;
class CNoteDataRWPrivate;

class CNoteDataRW
{
public:
    explicit CNoteDataRW();

    ~CNoteDataRW();

    /** ***********************************************************
     * @brief       从文件读取笔记数据
     * @param[in]   strNoteFileName 笔记文件名（完整路径）
     * @param[out]  vectorNoteData  存储读取结果的向量，函数会清空原有数据
     * @return      void
     * @note        线程安全，内部会自动处理文件读写锁
     *************************************************************/
    void ReadNoteData(const std::string &strNoteFileName, std::vector<ST_NOTE_DATA> &vectorNoteData);

    /** ***********************************************************
     * @brief       保存笔记数据到文件
     * @param[in]   strNoteFileName 笔记文件名（完整路径）
     * @param[in]   noteData        待保存的笔记数据
     * @param[in]   s32AppendPos    追加位置：0表示追加到末尾，>0表示插入到指定位置
     * @return      void
     * @note        线程安全，文件不存在时会自动创建；存储超限会自动截断旧数据
     *************************************************************/
    void SaveNoteData(const std::string &strNoteFileName, const ST_NOTE_DATA &noteData, INT32 s32AppendPos = 0);
private:
    void FileData2NoteData(const char *pBuffer, INT32 s32FileSize, std::vector<ST_NOTE_DATA> &listNote);
private:
    std::unique_ptr<CNoteDataRWPrivate> d_ptr;
};