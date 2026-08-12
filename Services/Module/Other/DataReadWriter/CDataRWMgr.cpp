/*
 * @file: CDataRWMgr.cpp
 * @brief: 
 * @author: nuo
 * @date: 2026/6/12
 * @Detail:
 */

#include "CDataRWMgr.h"
#include "INoteDataBuffer.h"
#include "DSaveDefine.h"
#include "CDataSave.h"
#include "CCommonConfig.h"
#include <filesystem>
using namespace std;

class CDataRWMgrPrivate
{
    friend class CDataRWMgr;
public:
    explicit CDataRWMgrPrivate(std::shared_ptr<CDataSave> dataSaver);
    ~CDataRWMgrPrivate() = default;
private:
    std::shared_ptr<CDataSave> m_pDataSave;
    std::string m_strNoteFileName;
};

CDataRWMgrPrivate::CDataRWMgrPrivate(std::shared_ptr<CDataSave> pDataSaver)
    : m_pDataSave(pDataSaver)
{
    if (!m_pDataSave)
    {
        throw std::runtime_error("CDataRWMgr: 注入的 DataSave 实例无效");
    }
    g_CCommonConfig->GetDataConfig(std::string(CConfigKey::COMMON_CONFIG_DATA_PATH), m_strNoteFileName);
    if (!m_strNoteFileName.empty() && m_strNoteFileName.back() != '/')
    {
        m_strNoteFileName += '/';
    }
    m_strNoteFileName += DSaveDefine::NOTE_FILE_NAME;
}

CDataRWMgr::CDataRWMgr(std::shared_ptr<CDataSave> pDataSaver)
    : d_ptr(std::make_unique<CDataRWMgrPrivate>(pDataSaver))
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

    d_ptr->m_pDataSave->Write2FileData(d_ptr->m_strNoteFileName, pData, s32Size);
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