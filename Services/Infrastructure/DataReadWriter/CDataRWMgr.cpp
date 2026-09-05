/*
 * @file: CDataRWMgr.cpp
 * @brief:
 * @author: nuo
 * @date: 2026/6/12
 * @Detail:
 */

// ========== 系统头文件 ==========
#include <filesystem>

// ========== 项目头文件 ==========
#include "CDataRWMgr.h"
#include "INoteDataBuffer.h"
#include "DSaveDefine.h"
#include "CDataSave.h"

using namespace std;

class CDataRWMgrPrivate
{
    friend class CDataRWMgr;
public:
    explicit CDataRWMgrPrivate(std::shared_ptr<CDataSave> dataSaver, const string& strDataPath);
    ~CDataRWMgrPrivate() = default;
private:
    std::shared_ptr<CDataSave> m_pDataSave;
    std::string m_strNoteFileName;
};

CDataRWMgrPrivate::CDataRWMgrPrivate(std::shared_ptr<CDataSave> pDataSaver, const string& strDataPath)
    : m_pDataSave(pDataSaver)
{
    if (!m_pDataSave)
    {
        throw std::runtime_error("CDataRWMgr: 注入的 DataSave 实例无效");
    }
    m_strNoteFileName = strDataPath;
    if (!m_strNoteFileName.empty() && m_strNoteFileName.back() != '/')
    {
        m_strNoteFileName += '/';
    }
    m_strNoteFileName += DSaveDefine::NOTE_FILE_NAME;
}

CDataRWMgr::CDataRWMgr(std::shared_ptr<CDataSave> pDataSaver, const std::string& strDataPath)
    : d_ptr(std::make_unique<CDataRWMgrPrivate>(pDataSaver, strDataPath))
{
    if (d_ptr->m_pDataSave && !std::filesystem::exists(d_ptr->m_strNoteFileName))
    {
        d_ptr->m_pDataSave->CreateNewFile(d_ptr->m_strNoteFileName);
    }
}

CDataRWMgr::~CDataRWMgr() = default;

void CDataRWMgr::WriteToFile(const char *pData, INT32 s32Size) const
{
    if (!pData || s32Size <= 0)
    {
        return;
    }

    if (!d_ptr->m_pDataSave->Write2FileData(d_ptr->m_strNoteFileName, pData, s32Size))
    {
        qDebug() << "WriteToFile failed";
    }
}

int CDataRWMgr::AddOneNoteData(const char *pData, INT32 s32Size)
{
    if (!pData || s32Size <= 0)
    {
        return FALSE;
    }
    INT64 s64_Offset = d_ptr->m_pDataSave->GetFileLastCount() * d_ptr->m_pDataSave->GetSingleSTSize();

    if (!d_ptr->m_pDataSave->Write2FileData(d_ptr->m_strNoteFileName, pData, s32Size, s64_Offset))
    {
        qDebug() << "AddOneNoteData failed";
    }
    return TRUE;
}

void CDataRWMgr::ReadFromFile(char *pBuffer, INT32 s32BufferSize, INT32 &s32DataSize) const
{
    s32DataSize = 0;

    if (!pBuffer || s32BufferSize <= 0)
    {
        return;
    }

    memset(pBuffer, 0, s32BufferSize);

    INT64 s64_HeaderSize = d_ptr->m_pDataSave ? d_ptr->m_pDataSave->GetHeaderSize() : 0;
    if (s64_HeaderSize < 0)
    {
        s64_HeaderSize = 0;
    }

    INT32 s32_ReadStartPos = static_cast<INT32>(s64_HeaderSize);
    s32DataSize = s32BufferSize;
    d_ptr->m_pDataSave->ReadFileData(d_ptr->m_strNoteFileName, pBuffer, s32DataSize, s32_ReadStartPos);
}