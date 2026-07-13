/*
 * @file: CDataSaveRWMgr.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/12
 * @Detail:
 */
#include "CDataSaveRWMgr.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <QDebug>
#include <mutex>
#include "CommonDefine.h"

using namespace std;

CDataSaveRWMgr::CDataSaveRWMgr(std::unique_ptr<st_FileHeaderBase> pFileHeader)
    : m_pFileHeader(move(pFileHeader))
{

}

CDataSaveRWMgr::~CDataSaveRWMgr()
{

}

void CDataSaveRWMgr::ReadFileData(const string &strFileName, char *pBuffer, INT32 &s32BufferSize, INT32 s32ReadStartPos)
{
    lock_guard<mutex> lock(m_mutexNote);
    if (!strFileName.empty() && pBuffer != nullptr && m_pFileHeader != nullptr)
    {
        ifstream file(strFileName, std::ios::binary);
        if (!file.is_open())
        {
            qDebug()<<"file not exist!";
            return ;
        }
        INT32 s32_ReadDataByte = 0;
        file.seekg(0, std::ios::end);
        s32_ReadDataByte = file.tellg();
        s32BufferSize = min(s32_ReadDataByte, s32BufferSize);

        file.seekg(s32ReadStartPos, std::ios::beg);
        file.read(pBuffer, s32BufferSize);
        file.close();
    }
}

void CDataSaveRWMgr::Write2FileData(const string &strFileName, const char *pBuffer, INT32 s32FileSize, INT32 s32CurrentPos)
{
    if (strFileName.empty() || pBuffer == nullptr || s32FileSize <= 0)
    {
        qDebug() << "Invalid parameters for write operation";
        return;
    }

    if (!filesystem::exists(strFileName))
    {
        if (!CreateNewFile(strFileName))
        {
            qDebug()<<"create new file failed!";
            return ;
        }
    }

    if (m_pFileHeader != nullptr)
    {
        lock_guard<mutex> lock(m_mutexNote);
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
            ofstream file(strFileName, std::ios::binary | std::ios::app);
            if (!file.is_open())
            {
                qDebug() << "Failed to open file for write at position: " << QString::fromStdString(strFileName);
                return ;
            }
            file.write(pBuffer, s32FileSize);
            file.close();
        }

    }
}

bool CDataSaveRWMgr::IsOverFileStoreLimit(const std::string &strFileName)
{
    if (!strFileName.empty() && m_pFileHeader != nullptr)
    {
        ifstream file(strFileName, std::ios::binary);
        if (!file.is_open())
        {
            qDebug() << "File not exist when checking limit: " << QString::fromStdString(strFileName);
            return false;
        }
        file.read(reinterpret_cast<char*>(m_pFileHeader.get()), m_pFileHeader->GetHeaderSize());
        return (m_pFileHeader->GetStoreCount() >= m_pFileHeader->GetStoreLimit());

    }
    return false;
}

bool CDataSaveRWMgr::CreateNewFile(const std::string &strFileName)
{
    if (!strFileName.empty() && m_pFileHeader != nullptr)
    {
        filesystem::path path(strFileName);
        filesystem::path path_Dir = path.parent_path();
        if (!filesystem::exists(path_Dir))
        {
            try
            {
                if (!filesystem::create_directories(path_Dir))
                {
                    qDebug()<<"create dir failed!";
                    return false;
                }
            }
            catch (...)
            {
                qDebug()<<"create dir failed!";
                return false;
            }
        }

        ofstream file(strFileName, std::ios::binary | std::ios::out);
        if (!file.is_open())
        {
            qDebug() << "Failed to create file: " << QString::fromStdString(strFileName);
            return false;
        }
        m_pFileHeader->m_s64FileStoreLimit = (CLimit::MAX_FILE_STORE_LIMIT);
        file.write(reinterpret_cast<const char*>(m_pFileHeader.get()),  m_pFileHeader->GetHeaderSize());
        file.close();
        return true;
    }
    return false;
}


bool CDataSaveRWMgr::UpdateNoteFileHeader(const std::string &strFileName,  st_FileHeaderBase *fileHeader)
{
    if (!strFileName.empty() && fileHeader != nullptr)
    {
        fstream file(strFileName, std::ios::binary | std::ios::out | std::ios::in);
        if (!file.is_open())
        {
            qDebug()<<"file not exist!";
            return false;
        }
        file.seekp(0, std::ios::beg);
        file.write((char *)fileHeader, fileHeader->GetHeaderSize());
        file.close();
        return true;
    }
    return false;
}

bool CDataSaveRWMgr::ReadFileHeader(const std::string &strFileName)
{
    memset(m_pFileHeader.get(), 0, m_pFileHeader->GetHeaderSize());
    if (!strFileName.empty() && m_pFileHeader != nullptr)
    {
        fstream file(strFileName, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            qDebug()<<"file not exist!";
            return false;
        }
        file.read((char *)m_pFileHeader.get(), m_pFileHeader->GetHeaderSize());
        file.close();
        return true;
    }
    return false;
}

bool CDataSaveRWMgr::CheckAndTruncateOldData(const std::string &strFileName, bool bIsTrunateLast)
{
    if (!strFileName.empty() && m_pFileHeader != nullptr)
    {
        bool b_ret= ReadFileHeader(strFileName);
        if (b_ret && m_pFileHeader->GetStoreCount() >= m_pFileHeader->GetStoreLimit() && m_pFileHeader->GetStoreCount() > 0)
        {
            if (bIsTrunateLast)
            {
                filesystem::resize_file(strFileName, (m_pFileHeader->GetStoreCount() * m_pFileHeader->GetSingleSTSize() * 0.8 + m_pFileHeader->GetHeaderSize()));
            }
            else
            {
                INT32 s32_ReadDataByte = m_pFileHeader->GetHeaderSize() + m_pFileHeader->GetStoreCount() * m_pFileHeader->GetSingleSTSize();
                char * pBuffer = new char[s32_ReadDataByte];
                ReadFileData(strFileName, pBuffer, s32_ReadDataByte, 0);
                INT32 s32_ReStoreDataByte = s32_ReadDataByte - m_pFileHeader->GetHeaderSize() - (INT32)(0.2 * m_pFileHeader->GetSingleSTSize() * m_pFileHeader->GetStoreCount());
                char * ptempBuffer = new char[s32_ReStoreDataByte];

                memcpy(ptempBuffer, (pBuffer + m_pFileHeader->GetHeaderSize() + (INT32)(0.2 * m_pFileHeader->GetSingleSTSize() * m_pFileHeader->GetStoreCount())), s32_ReStoreDataByte);
                Write2FileData(strFileName, ptempBuffer, s32_ReStoreDataByte, 0);
                RELEASEIF(pBuffer);
                RELEASEIF(ptempBuffer);
            }
            sync();
            m_pFileHeader->m_s64FileStoreCount = m_pFileHeader->GetStoreLimit() * 0.8;
            UpdateNoteFileHeader(strFileName, m_pFileHeader.get());
            return true;
        }
    }
    return false;
}
