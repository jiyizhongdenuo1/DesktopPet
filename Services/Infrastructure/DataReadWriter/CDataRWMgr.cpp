/*
 * @file: CDataRWMgr.cpp
 * @brief:
 * @author: nuo
 * @date: 2026/6/12
 * @Detail:
 */

// ========== 系统头文件 ==========
#include <filesystem>
#include <QDebug>
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
    explicit CDataRWMgrPrivate(shared_ptr<CDataSave> dataSaver, shared_ptr<CDataSave> recycle, const string& strDataPath);
    ~CDataRWMgrPrivate() = default;
private:
    shared_ptr<CDataSave> m_pDataSave;
    shared_ptr<CDataSave> m_pRecycleOperater;

    string m_strNoteFileName;
    string m_strRecycleFileName;
};

CDataRWMgrPrivate::CDataRWMgrPrivate(shared_ptr<CDataSave> pDataSaver, shared_ptr<CDataSave> recycle, const string& strDataPath)
    : m_pDataSave(pDataSaver)
    , m_pRecycleOperater(recycle)
{
    if (!m_pDataSave || !m_pRecycleOperater)
    {
        throw runtime_error("CDataRWMgr: 注入的 DataSave 实例无效");
    }
    m_strNoteFileName = strDataPath;
    if (!m_strNoteFileName.empty() && m_strNoteFileName.back() != '/')
    {
        m_strNoteFileName += '/';
    }
    m_strRecycleFileName = m_strNoteFileName + DSaveDefine::RECYCLE_FILE_NAME;
    m_strNoteFileName += DSaveDefine::NOTE_FILE_NAME;

}

CDataRWMgr::CDataRWMgr(shared_ptr<CDataSave> pDataSaver, shared_ptr<CDataSave> recycle, const string& strDataPath)
    : d_ptr(make_unique<CDataRWMgrPrivate>(pDataSaver, recycle, strDataPath))
{
    if (d_ptr->m_pDataSave && !filesystem::exists(d_ptr->m_strNoteFileName))
    {
        d_ptr->m_pDataSave->CreateNewFile(d_ptr->m_strNoteFileName);
    }
    if (d_ptr->m_pRecycleOperater && !filesystem::exists(d_ptr->m_strRecycleFileName))
    {
        d_ptr->m_pRecycleOperater->CreateNewFile(d_ptr->m_strRecycleFileName);
    }
}

CDataRWMgr::~CDataRWMgr() = default;

void CDataRWMgr::WriteToFile(const char *pData, INT32 s32Size) const
{
    if (!pData || s32Size <= 0 || !d_ptr->m_pDataSave)
    {
        return;
    }

    if (!d_ptr->m_pDataSave->Write2FileData(d_ptr->m_strNoteFileName, pData, s32Size))
    {
        qDebug() << "WriteToFile failed";
    }
}

int CDataRWMgr::AddOneNoteData(const char *pData, INT32 s32Size, BOOL bIsDeleteData)
{
    if (!pData || s32Size <= 0 || !d_ptr->m_pDataSave || !d_ptr->m_pRecycleOperater)
    {
        return FALSE;
    }
    shared_ptr<CDataSave> p_Saver = bIsDeleteData ? d_ptr->m_pRecycleOperater : d_ptr->m_pDataSave;
    INT64 s64_Offset = p_Saver->GetFileLastCount() * p_Saver->GetSingleSTSize();

    string str_Filename = bIsDeleteData ? d_ptr->m_strRecycleFileName : d_ptr->m_strNoteFileName;
    if (!p_Saver->Write2FileData(str_Filename, pData, s32Size, s64_Offset))
    {
        qDebug() << (bIsDeleteData ? "AddRecycleData failed" : "AddOneNoteData failed");
    }
    return TRUE;
}

void CDataRWMgr::ReadFromFile(char *pBuffer, INT32 s32BufferSize, INT32 &s32DataSize, BOOL bIsDeleteData) const
{
    s32DataSize = 0;

    if (!pBuffer || s32BufferSize <= 0 || !d_ptr->m_pDataSave || !d_ptr->m_pRecycleOperater)
    {
        return;
    }

    memset(pBuffer, 0, s32BufferSize);

    shared_ptr<CDataSave> p_Saver = bIsDeleteData ? d_ptr->m_pRecycleOperater : d_ptr->m_pDataSave;

    INT64 s64_HeaderSize = p_Saver ? p_Saver->GetHeaderSize() : 0;
    if (s64_HeaderSize < 0)
    {
        s64_HeaderSize = 0;
    }

    const string &str_Filename = bIsDeleteData ? d_ptr->m_strRecycleFileName : d_ptr->m_strNoteFileName;

    INT32 s32_ReadStartPos = static_cast<INT32>(s64_HeaderSize);
    s32DataSize = s32BufferSize;
    p_Saver->ReadFileData(str_Filename, pBuffer, s32DataSize, s32_ReadStartPos);
}

bool CDataRWMgr::UpdateNextId(INT64 s64NextId)
{
    if (!d_ptr->m_pDataSave)
    {
        return FALSE;
    }
    d_ptr->m_pDataSave->SetNextId(s64NextId);
    return TRUE;
}

void CDataRWMgr::CompactNoteFile()
{
    if (!d_ptr->m_pDataSave)
    {
        return ;
    }
    d_ptr->m_pDataSave->CompactNoteFile(d_ptr->m_strNoteFileName);
}
