/*
 * @file: CDataSave.cpp
 * @brief: 
 * @author: nuo
 * @date: 2026/6/12
 * @Detail:
 */
#include "CDataSave.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <cerrno>
#include <QDebug>
#include <mutex>
#include "CommonDefine.h"
#include "DSaveDefine.h"

using namespace std;

CDataSave::CDataSave(std::unique_ptr<st_FileHeaderBase> pFileHeader)
    : m_pFileHeader(move(pFileHeader))
{

}

CDataSave::~CDataSave()
{

}

INT64 CDataSave::GetHeaderSize() const
{
    return m_pFileHeader ? m_pFileHeader->GetHeaderSize() : 0;
}

INT64 CDataSave::GetFileLastCount() const
{
    if (m_pFileHeader == nullptr)
    {
        return 0;
    }
    return m_pFileHeader->m_s64FileStoreCount;
}

INT64 CDataSave::GetSingleSTSize() const
{
    if (m_pFileHeader == nullptr)
    {
        return 0;
    }
    return m_pFileHeader->GetSingleSTSize();
}

bool CDataSave::SetNextId(INT64 s64NextId)
{
    if (m_pFileHeader == nullptr)
    {
        return FALSE;
    }
    m_pFileHeader->SetNextId(s64NextId);
    return TRUE;
}

bool CDataSave::CompactNoteFile(const string &strFileName)
{
    unique_lock<shared_mutex> lock(m_mutexNote);
    if (strFileName.empty()|| m_pFileHeader == nullptr || !ReadFileHeader(strFileName))
    {
        return FALSE;
    }
    const INT64 s64_HeaderSize   = m_pFileHeader->GetHeaderSize();
    const INT64 s64_SingleSTSize = m_pFileHeader->GetSingleSTSize();
    const INT64 s64_StoreCount   = m_pFileHeader->GetStoreCount();

    if (s64_StoreCount <= 0 || s64_SingleSTSize <= 0)
    {
        return false;
    }

    ifstream file(strFileName, std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }
    INT64 s64_ReadSize = s64_SingleSTSize * s64_StoreCount;

    char *p_Buffer = new char[s64_ReadSize];
    s64_ReadSize = ReadData(file, p_Buffer, s64_ReadSize, s64_HeaderSize);
    if (s64_ReadSize <= 0)
    {
        RELEASEIF(p_Buffer);
        return false;
    }
    const auto *p_arr = reinterpret_cast<const ST_NOTE_DATA*> (p_Buffer);
    const INT32 s32_ReadDataCount = s64_ReadSize / (s64_SingleSTSize);
    std::map<INT64, ST_NOTE_DATA> map_Data;
    for (INT32 i = 0; i < s32_ReadDataCount; ++i)
    {
        const ST_NOTE_DATA& st_Data = p_arr[i];
        if (st_Data.m_s32id <= 0 || st_Data.m_bDeleted)
        {
            continue;
        }
        map_Data[st_Data.m_s32id] = st_Data;
    }

    m_pFileHeader->SetStoreCount(static_cast<INT64>(map_Data.size()));

    const std::string str_Tmp = strFileName + ".tmp";
    {
        // ofstream 打开模式与"文件已存在"时的行为：
        // ofstream 默认out ifstream 默认in
        //   out         → 截断（清空） 创建文件
        //   out | trunc → 截断（清空） 创建文件
        //   out | app   → 不截断，追加到末尾 创建文件
        //   out | in    → 不截断，可定位覆盖写 不创建文件
        std::ofstream file(str_Tmp, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!file.is_open())
        {
            RELEASEIF(p_Buffer);
            return false;
        }
        file.write(reinterpret_cast<const char *>(m_pFileHeader.get()), s64_HeaderSize);
        {
            for (const auto &pair_Data : map_Data)
            {
                file.write(reinterpret_cast<const char *>(&pair_Data.second), sizeof(ST_NOTE_DATA));
            }
        }
        file.close();
        if (!file.good())
        {
            std::filesystem::remove(str_Tmp);
            RELEASEIF(p_Buffer);
            return false;
        }
    }
    RELEASEIF(p_Buffer);
    std::error_code ec;
    std::filesystem::rename(str_Tmp, strFileName, ec);
    if (ec)
    {
        std::filesystem::remove(str_Tmp);
        return false;
    }
    return true;
}

void CDataSave::ReadFileData(const string &strFileName, char *pBuffer, INT32 &s32BufferSize, INT32 s32ReadStartPos)
{
    shared_lock<shared_mutex> lock(m_mutexNote);
    if (!strFileName.empty() && pBuffer != nullptr && m_pFileHeader != nullptr)
    {
        ifstream file(strFileName, std::ios::binary);
        if (!file.is_open())
        {
            if (filesystem::exists(strFileName))
            {
                qDebug() << "file exists but cannot open: " << strFileName;
            }
            else
            {
                qDebug() << "file not exist: " << strFileName;
            }
            return ;
        }
        const INT32 s32_Read = ReadData(file, pBuffer, s32BufferSize, s32ReadStartPos);
        s32BufferSize = (s32_Read > 0) ? s32_Read : 0;
        file.close();
    }
}

bool CDataSave::Write2FileData(const string &strFileName, const char *pBuffer, INT32 s32FileSize, INT64 s64CurrentPos)
{
    if (strFileName.empty() || pBuffer == nullptr || s32FileSize <= 0)
    {
        qDebug() << "Invalid parameters for write operation";
        return false;
    }

    unique_lock<shared_mutex> lock(m_mutexNote);
    if (!filesystem::exists(strFileName))
    {
        if (!CreateNewFile(strFileName))
        {
            qDebug() << "create new file failed! errno:" << errno << strFileName;
            return false;
        }
        ReadFileHeader(strFileName);
    }
    bool bIsTrunc = CheckAndTruncateOldData(strFileName);
    if (bIsTrunc)
    {
        s64CurrentPos = GetFileLastCount() * GetSingleSTSize();
    }
    if (m_pFileHeader != nullptr)
    {
        if (s64CurrentPos > 0)
        {
            fstream file(strFileName, std::ios::binary | std::ios::out | std::ios::in);
            if (!file.is_open())
            {
                qDebug() << "Failed to open file for write at position: " << strFileName;
                return false;
            }
            file.seekp(m_pFileHeader->GetHeaderSize() + s64CurrentPos, std::ios::beg);
            file.write(pBuffer, s32FileSize);
            file.close();
        }
        else if (s64CurrentPos == 0)
        {
            fstream file(strFileName, std::ios::binary | std::ios::out | std::ios::in);
            if (!file.is_open())
            {
                qDebug() << "Failed to open file for write at position: " << strFileName;
                return false;
            }
            file.seekp(m_pFileHeader->GetHeaderSize(), std::ios::beg);
            file.write(pBuffer, s32FileSize);
            file.close();
        }
        st_FileHeaderBase *p_FileHeader = m_pFileHeader.get();
        if (p_FileHeader != nullptr && p_FileHeader->GetSingleSTSize() > 0)
        {
            const INT64 s64WriteCount = s32FileSize / p_FileHeader->GetSingleSTSize();
            if (s64CurrentPos > 0)
            {
                p_FileHeader->SetStoreCount(p_FileHeader->GetStoreCount() + s64WriteCount);
            }
            else
            {
                p_FileHeader->SetStoreCount(s64WriteCount);
            }
        }
        UpdateNoteFileHeader(strFileName, p_FileHeader);
    }
    return true;
}

bool CDataSave::IsOverFileStoreLimit(const std::string &strFileName)
{
    if (!strFileName.empty() && m_pFileHeader != nullptr)
    {
        unique_lock<shared_mutex> lock(m_mutexNote);
        ifstream file(strFileName, std::ios::binary);
        if (!file.is_open())
        {
            if (filesystem::exists(strFileName))
            {
                qDebug() << "file exists but cannot open when checking limit: " << strFileName;
            }
            else
            {
                qDebug() << "File not exist when checking limit: " << strFileName;
            }
            return false;
        }
        file.read(reinterpret_cast<char*>(m_pFileHeader.get()), m_pFileHeader->GetHeaderSize());
        return (m_pFileHeader->GetStoreCount() >= m_pFileHeader->GetStoreLimit());

    }
    return false;
}

bool CDataSave::CreateNewFile(const std::string &strFileName)
{
    if (!strFileName.empty() && m_pFileHeader != nullptr && !filesystem::exists(strFileName))
    {
        filesystem::path fs_Path(strFileName);
        filesystem::path path_Dir = fs_Path.parent_path();
        if (!filesystem::exists(path_Dir))
        {
            try
            {
                if (!filesystem::create_directories(path_Dir))
                {
                    qDebug() << "create dir failed! errno:" << errno << path_Dir.string();
                    return false;
                }
            }
            catch (const std::exception &e)
            {
                qDebug() << "create dir exception:" << e.what() << path_Dir.string();
                return false;
            }
        }

        ofstream file(strFileName, std::ios::binary | std::ios::out);
        if (!file.is_open())
        {
            qDebug() << "Failed to create file, errno:" << errno << strFileName;
            return false;
        }
        m_pFileHeader->m_s64FileStoreLimit = (CLimit::MAX_FILE_STORE_LIMIT);
        file.write(reinterpret_cast<const char*>(m_pFileHeader.get()),  m_pFileHeader->GetHeaderSize());
        file.close();
        return true;
    }
    return false;
}


bool CDataSave::UpdateNoteFileHeader(const std::string &strFileName,  st_FileHeaderBase *pFileHeader)
{
    if (!strFileName.empty() && pFileHeader != nullptr)
    {
        fstream file(strFileName, std::ios::binary | std::ios::out | std::ios::in);
        if (!file.is_open())
        {
            if (filesystem::exists(strFileName))
            {
                qDebug() << "file exists but cannot open for update: " << strFileName;
            }
            else
            {
                qDebug() << "file not exist when updating header: " << strFileName;
            }
            return false;
        }
        file.seekp(0, std::ios::beg);
        file.write((char *)pFileHeader, pFileHeader->GetHeaderSize());
        file.close();
        return true;
    }
    return false;
}

bool CDataSave::ReadFileHeader(const std::string &strFileName)
{
    if (!strFileName.empty() && m_pFileHeader != nullptr)
    {
        const INT64 s64_HeaderSize = m_pFileHeader->GetHeaderSize();
        const INT64 s64_KeepNextId  = m_pFileHeader->GetNextId();
        fstream file(strFileName, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            if (filesystem::exists(strFileName))
            {
                qDebug() << "file exists but cannot open for read: " << strFileName;
            }
            else
            {
                qDebug() << "file not exist when reading header: " << strFileName;
            }
            return false;
        }
        memset(m_pFileHeader.get(), 0, s64_HeaderSize);
        file.read((char *)m_pFileHeader.get(), s64_HeaderSize);
        file.close();

        if (s64_KeepNextId > m_pFileHeader->GetNextId())
        {
            m_pFileHeader->SetNextId(s64_KeepNextId);
        }

        return true;
    }
    return false;
}

INT64 CDataSave::ReadData(std::ifstream &file, char *pBuffer, INT64 s64MaxSize, INT64 s64StartPos) const
{
    file.seekg(0, std::ios::end);
    const INT64 s64_FileSize = static_cast<INT64>(file.tellg());
    if (s64_FileSize <= 0)
    {
        return 0;
    }
    const INT64 s64_DataLen = s64_FileSize - s64StartPos;
    if (s64_DataLen <= 0)
    {
        return 0;
    }
    const INT64 s64_ReadSize = std::min<INT64>(s64_DataLen, s64MaxSize);
    file.seekg(s64StartPos, std::ios::beg);
    file.read(pBuffer, s64_ReadSize);
    return static_cast<INT64>(file.gcount());
}

bool CDataSave::CheckAndTruncateOldData(const std::string &strFileName, bool bIsTrunateLast)
{
    if (!strFileName.empty() && m_pFileHeader != nullptr)
    {
        bool b_Ret= ReadFileHeader(strFileName);
        if (b_Ret && m_pFileHeader->GetStoreCount() >= m_pFileHeader->GetStoreLimit() && m_pFileHeader->GetStoreCount() > 0)
        {
            const INT64 s64_HeaderSize   = m_pFileHeader->GetHeaderSize();
            const INT64 s64_SingleSTSize = m_pFileHeader->GetSingleSTSize();
            const INT64 s64_StoreCount   = m_pFileHeader->GetStoreCount();
            const INT64 s64_KeepPercent  = DSaveDefine::TRUNCATE_KEEP_RATIO_PERCENT;

            if (bIsTrunateLast)
            {
                INT64 s64_NewFileSize = s64_HeaderSize + s64_StoreCount * s64_SingleSTSize * s64_KeepPercent / 100;
                if (s64_NewFileSize > 0)
                {
                    filesystem::resize_file(strFileName, s64_NewFileSize);
                }
            }
            else
            {
                const INT32 s32_BufferSize = static_cast<INT32>(s64_HeaderSize + s64_StoreCount * s64_SingleSTSize);
                char * p_Buffer = new char[s32_BufferSize];
                INT32 s32_ActualDataLen = 0;

                {
                    ifstream file_Data(strFileName, std::ios::binary);
                    if (!file_Data.is_open())
                    {
                        qDebug() << "Failed to open file for truncate read";
                        RELEASEIF(p_Buffer);
                        return false;
                    }
                    s32_ActualDataLen = ReadData(file_Data, p_Buffer, s32_BufferSize, s64_HeaderSize);
                }

                INT32 s32_SkipBytes   = static_cast<INT32>(s64_StoreCount * s64_SingleSTSize * (100 - s64_KeepPercent) / 100);

                if (s32_SkipBytes >= s32_ActualDataLen || s32_ActualDataLen <= 0)
                {
                    RELEASEIF(p_Buffer);
                    return false;
                }

                INT32 s32_KeepDataLen = s32_ActualDataLen - s32_SkipBytes;

                char * p_tempBuffer = new char[s32_KeepDataLen];
                memcpy(p_tempBuffer, p_Buffer + s32_SkipBytes, s32_KeepDataLen);

                std::string str_tempFileName = strFileName + ".tmp";
                bool b_WriteOk = false;

                {
                    ofstream file_Tmp(str_tempFileName, std::ios::binary | std::ios::out | std::ios::trunc);
                    if (!file_Tmp.is_open())
                    {
                        qDebug() << "Failed to open temp file for truncate: " << str_tempFileName;
                        RELEASEIF(p_Buffer);
                        RELEASEIF(p_tempBuffer);
                        return false;
                    }
                    file_Tmp.write(reinterpret_cast<const char*>(m_pFileHeader.get()), s64_HeaderSize);
                    file_Tmp.write(p_tempBuffer, s32_KeepDataLen);
                    b_WriteOk = file_Tmp.good();
                }

                RELEASEIF(p_Buffer);
                RELEASEIF(p_tempBuffer);

                if (!b_WriteOk)
                {
                    qDebug() << "Temp file write failed, abort truncate";
                    filesystem::remove(str_tempFileName);
                    return false;
                }

                std::error_code ec;
                filesystem::rename(str_tempFileName, strFileName, ec);
                if (ec)
                {
                    qDebug() << "Rename temp file failed: " << ec.message();
                    filesystem::remove(str_tempFileName);
                    return false;
                }
            }
            {
                // 打开文件触发 OS flush，确保 rename 持久化，跨平台无需 sync()
                std::ofstream touch(strFileName, std::ios::app);
            }
            m_pFileHeader->m_s64FileStoreCount = m_pFileHeader->GetStoreLimit() * s64_KeepPercent / 100;
            return true;
        }
    }
    return false;
}
