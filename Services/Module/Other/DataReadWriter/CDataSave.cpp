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
                qDebug() << "file exists but cannot open: " << QString::fromStdString(strFileName);
            }
            else
            {
                qDebug() << "file not exist: " << QString::fromStdString(strFileName);
            }
            return ;
        }
        INT32 s32_ReadDataByte = 0;
        file.seekg(0, std::ios::end);
        s32_ReadDataByte = file.tellg();
        if (s32_ReadDataByte <= 0)
        {
            qDebug()<<"file is empty!";
            return ;
        }
        s32BufferSize = min(s32_ReadDataByte, s32BufferSize);

        file.seekg(s32ReadStartPos, std::ios::beg);
        file.read(pBuffer, s32BufferSize);
        file.close();
    }
}

void CDataSave::Write2FileData(const string &strFileName, const char *pBuffer, INT32 s32FileSize, INT32 s32CurrentPos)
{
    if (strFileName.empty() || pBuffer == nullptr || s32FileSize <= 0)
    {
        qDebug() << "Invalid parameters for write operation";
        return;
    }

    unique_lock<shared_mutex> lock(m_mutexNote);
    if (!filesystem::exists(strFileName))
    {
        if (!CreateNewFile(strFileName))
        {
            qDebug() << "create new file failed! errno:" << errno << QString::fromStdString(strFileName);
            return ;
        }
        ReadFileHeader(strFileName);
    }
    CheckAndTruncateOldData(strFileName);
    if (m_pFileHeader != nullptr)
    {
        if (s32CurrentPos > 0)
        {
            fstream file(strFileName, std::ios::binary | std::ios::out | std::ios::in);
            if (!file.is_open())
            {
                qDebug() << "Failed to open file for write at position: " << QString::fromStdString(strFileName);
                return ;
            }
            file.seekp(m_pFileHeader->GetHeaderSize() + s32CurrentPos, std::ios::beg);
            file.write(pBuffer, s32FileSize);
            file.close();
        }
        else if (s32CurrentPos == 0)
        {
            fstream file(strFileName, std::ios::binary | std::ios::out | std::ios::in);
            if (!file.is_open())
            {
                qDebug() << "Failed to open file for write at position: " << QString::fromStdString(strFileName);
                return ;
            }
            file.seekp(m_pFileHeader->GetHeaderSize(), std::ios::beg);
            file.write(pBuffer, s32FileSize);
            file.close();
        }
        st_FileHeaderBase *p_FileHeader = m_pFileHeader.get();
        if (p_FileHeader != nullptr && p_FileHeader->GetSingleSTSize() > 0)
        {
            p_FileHeader->SetStoreCount(p_FileHeader->GetStoreCount() + s32FileSize / p_FileHeader->GetSingleSTSize());
        }
        UpdateNoteFileHeader(strFileName, p_FileHeader);
    }
}

bool CDataSave::IsOverFileStoreLimit(const std::string &strFileName)
{
    if (!strFileName.empty() && m_pFileHeader != nullptr)
    {
        ifstream file(strFileName, std::ios::binary);
        if (!file.is_open())
        {
            if (filesystem::exists(strFileName))
            {
                qDebug() << "file exists but cannot open when checking limit: " << QString::fromStdString(strFileName);
            }
            else
            {
                qDebug() << "File not exist when checking limit: " << QString::fromStdString(strFileName);
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
    if (!strFileName.empty() && m_pFileHeader != nullptr)
    {
        filesystem::path fs_Path(strFileName);
        filesystem::path path_Dir = fs_Path.parent_path();
        if (!filesystem::exists(path_Dir))
        {
            try
            {
                if (!filesystem::create_directories(path_Dir))
                {
                    qDebug() << "create dir failed! errno:" << errno << QString::fromStdString(path_Dir.c_str());
                    return false;
                }
            }
            catch (const std::exception &e)
            {
                qDebug() << "create dir exception:" << e.what() << QString::fromStdString(path_Dir.c_str());
                return false;
            }
        }

        ofstream file(strFileName, std::ios::binary | std::ios::out);
        if (!file.is_open())
        {
            qDebug() << "Failed to create file, errno:" << errno << QString::fromStdString(strFileName);
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
                qDebug() << "file exists but cannot open for update: " << QString::fromStdString(strFileName);
            }
            else
            {
                qDebug() << "file not exist when updating header: " << QString::fromStdString(strFileName);
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
        memset(m_pFileHeader.get(), 0, s64_HeaderSize);
        fstream file(strFileName, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            if (filesystem::exists(strFileName))
            {
                qDebug() << "file exists but cannot open for read: " << QString::fromStdString(strFileName);
            }
            else
            {
                qDebug() << "file not exist when reading header: " << QString::fromStdString(strFileName);
            }
            return false;
        }
        file.read((char *)m_pFileHeader.get(), s64_HeaderSize);
        file.close();
        return true;
    }
    return false;
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
                INT32 s32_ReadDataByte = static_cast<INT32>(s64_HeaderSize + s64_StoreCount * s64_SingleSTSize);
                char * p_Buffer = new char[s32_ReadDataByte];

                {
                    ifstream file_Data(strFileName, std::ios::binary);
                    if (!file_Data.is_open())
                    {
                        qDebug() << "Failed to open file for truncate read";
                        RELEASEIF(p_Buffer);
                        return false;
                    }
                    file_Data.read(p_Buffer, s32_ReadDataByte);
                }

                INT32 s32_ActualDataLen = s32_ReadDataByte - static_cast<INT32>(s64_HeaderSize);
                INT32 s32_SkipBytes   = static_cast<INT32>(s64_StoreCount * s64_SingleSTSize * (100 - s64_KeepPercent) / 100);

                if (s32_SkipBytes >= s32_ActualDataLen || s32_ActualDataLen <= 0)
                {
                    RELEASEIF(p_Buffer);
                    return false;
                }

                INT32 s32_KeepDataLen = s32_ActualDataLen - s32_SkipBytes;

                char * p_tempBuffer = new char[s32_KeepDataLen];
                memcpy(p_tempBuffer, p_Buffer + s64_HeaderSize + s32_SkipBytes, s32_KeepDataLen);

                std::string str_tempFileName = strFileName + ".tmp";
                bool b_WriteOk = false;

                {
                    ofstream file_Tmp(str_tempFileName, std::ios::binary | std::ios::out | std::ios::trunc);
                    if (!file_Tmp.is_open())
                    {
                        qDebug() << "Failed to open temp file for truncate: " << QString::fromStdString(str_tempFileName);
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
                    qDebug() << "Rename temp file failed: " << QString::fromStdString(ec.message());
                    filesystem::remove(str_tempFileName);
                    return false;
                }
            }
            sync();
            m_pFileHeader->m_s64FileStoreCount = m_pFileHeader->GetStoreLimit() * s64_KeepPercent / 100;
            return true;
        }
    }
    return false;
}