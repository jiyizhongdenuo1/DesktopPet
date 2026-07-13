/*
 * @file: CDataSaveRWMgr.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/12
 * @Detail:
 */

#pragma once


#include <string>
#include <mutex>
#include <memory>
#include "DServiceBase.h"

typedef struct st_NoteFileHeader : public st_FileHeaderBase
{
    INT64 m_s64InitID;
    INT64 m_s64FileID;
    INT64 m_s64FileType;
    INT64 m_s64FileEventType;

    st_NoteFileHeader()
        : st_FileHeaderBase(sizeof(st_NoteFileHeader),  sizeof(st_NoteData))
        , m_s64InitID(0)
        , m_s64FileID(0)
        , m_s64FileType(0)
        , m_s64FileEventType(0)
    {
    }
}NOTE_FILE_HEADER;

class CDataSaveRWMgr
{
public:
    explicit CDataSaveRWMgr(std::unique_ptr<st_FileHeaderBase> pFileHeader);
    ~CDataSaveRWMgr();

    /** ***********************************************************
     * @brief       从文件读取数据
     * @param[in]   strFileName    文件名（完整路径）
     * @param[out]  pBuffer        输出缓冲区，用于存储读取的数据
     * @param[in]   s32FileSize    缓冲区大小（字节），实际读取量不超过此值
     * @param[in]   s32ReadStartPos 读取起始位置（字节偏移量，从文件开头算起）
     * @return      void
     * @note        线程安全，内部会自动获取互斥锁
     *************************************************************/
    void ReadFileData(const std::string &strFileName, char *pBuffer, INT32 &s32FileSize, INT32 s32ReadStartPos);

    /** ***********************************************************
     * @brief       写入数据到文件
     * @param[in]   strFileName    文件名（完整路径）
     * @param[in]   pBuffer        待写入的数据缓冲区
     * @param[in]   s32FileSize    待写入的数据大小（字节）
     * @param[in]   s32CurrentPos  写入位置：0表示追加到文件末尾，>0表示指定字节偏移位置
     * @return      void
     * @note        线程安全，内部会自动获取互斥锁；文件不存在时会自动创建
     *************************************************************/
    void Write2FileData(const std::string &strFileName, const char *pBuffer, INT32 s32FileSize, INT32 s32CurrentPos = 0);

    /** ***********************************************************
     * @brief       创建新文件并写入文件头
     * @param[in]   strFileName    文件名（完整路径）
     * @return      true-创建成功，false-创建失败
     * @note        文件已存在时不会覆盖，直接返回false
     *************************************************************/
    bool CreateNewFile(const std::string &strFileName);

    bool CheckAndTruncateOldData(const std::string &strFileName, bool bIsTrunateLast = false);

private:
    bool IsOverFileStoreLimit(const std::string &strFileName);
    bool UpdateNoteFileHeader(const std::string &strFileName, st_FileHeaderBase *fileHeader);
    bool ReadFileHeader(const std::string &strFileName);

private:
    std::mutex m_mutexNote;
    std::unique_ptr<st_FileHeaderBase> m_pFileHeader;
};