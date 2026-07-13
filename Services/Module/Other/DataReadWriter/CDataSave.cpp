/*
 * @file: CDataSave.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/12
 * @Detail:
 */

#include "CDataSave.h"
#include "CDataCollectFactory.h"
#include "CNoteDataCollect.h"
#include "DSaveDefine.h"
#include "CDataSaveRWMgr.h"
#include "CCommonConfig.h"

using namespace std;

class CDataSavePrivate
{
    friend class CDataSave;
public:
    CDataSavePrivate();
    ~CDataSavePrivate() = default;
private:
    unique_ptr<CDataSaveRWMgr> m_DataSaveRWMgr;
    string m_strNoteFileName;
};

CDataSavePrivate::CDataSavePrivate()
    : m_DataSaveRWMgr(make_unique<CDataSaveRWMgr>(make_unique<NOTE_FILE_HEADER>()))
{
    g_CCommonConfig->GetDataConfig(string(CConfigKey::COMMON_CONFIG_DATA_PATH), m_strNoteFileName);
    m_strNoteFileName += string("/")+ string(DSaveDefine::NOTE_FILE_NAME);
}

CDataSave::CDataSave()
    : d_ptr(make_unique<CDataSavePrivate>())
{

}

CDataSave::~CDataSave() = default;

void CDataSave::SaveNoteData() const
{
    auto p_DataCollect = g_CDataCollectFactory->GetCDataCollect(E_COLLECTION_TYPE_NOTE);
    auto p_NoteData = std::dynamic_pointer_cast<CNoteDataCollect>(p_DataCollect);
    if (!p_NoteData)
    {
        return;
    }
    INT32 s32_SaveNoteDataLimit = DSaveDefine::SINGLE_SAVE_NOTE_DATA_COUNT * sizeof(st_NoteData);
    char *pNoteData = new char[s32_SaveNoteDataLimit];
    p_NoteData->GetBuffer(s32_SaveNoteDataLimit, pNoteData);
    d_ptr->m_DataSaveRWMgr->Write2FileData(d_ptr->m_strNoteFileName, pNoteData, s32_SaveNoteDataLimit);
}
