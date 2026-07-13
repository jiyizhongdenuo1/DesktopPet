/*
 * @file: CNoteDataRW.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/17
 * @Detail:
 */

#include "CNoteDataRW.h"
#include "CDataSaveRWMgr.h"

using namespace CPath;
using namespace std;

class CNoteDataRWPrivate
{
    friend class CNoteDataRW;
public:
    explicit CNoteDataRWPrivate();
    ~CNoteDataRWPrivate(){}

private:
    unique_ptr<CDataSaveRWMgr> m_pDataSaveRWMgr;
    INT32                      m_s32CurrentPos = 0;
    string                     m_strNoteFileName;
};

CNoteDataRWPrivate::CNoteDataRWPrivate()
    : m_pDataSaveRWMgr(make_unique<CDataSaveRWMgr>(make_unique<NOTE_FILE_HEADER>()))
{
    m_strNoteFileName = STORE_PATH;
    m_strNoteFileName +=  NOTE_FILE_NAME;
}

CNoteDataRW::CNoteDataRW()
    : d_ptr(make_unique<CNoteDataRWPrivate>())
{
    d_ptr->m_pDataSaveRWMgr->CreateNewFile(d_ptr->m_strNoteFileName);
}

CNoteDataRW::~CNoteDataRW()
{

}

void CNoteDataRW::ReadNoteData(const std::string &strNoteFileName, std::vector<ST_NOTE_DATA> &vectorNoteData)
{
    INT32 s32_BufferSize = NoteSpace::CONTENT_LENGTH_MAX * sizeof(ST_NOTE_DATA);
    char *pBuffer = new char[s32_BufferSize];
    if (d_ptr->m_pDataSaveRWMgr  && !strNoteFileName.empty())
    {
        d_ptr->m_pDataSaveRWMgr->ReadFileData(strNoteFileName, pBuffer, s32_BufferSize, 0);
        FileData2NoteData(pBuffer, s32_BufferSize, vectorNoteData);
        RELEASEIF(pBuffer);
    }
}

void CNoteDataRW::SaveNoteData(const std::string &strNoteFileName, const ST_NOTE_DATA &noteData, INT32 s32AppendPos)
{
    if (d_ptr->m_pDataSaveRWMgr && !strNoteFileName.empty())
    {
        d_ptr->m_pDataSaveRWMgr->CheckAndTruncateOldData(strNoteFileName);
        d_ptr->m_pDataSaveRWMgr->Write2FileData(strNoteFileName,
                                                reinterpret_cast<const char*>(&noteData),
                                                sizeof(noteData),
                                                s32AppendPos * sizeof(ST_NOTE_DATA) + sizeof(NOTE_FILE_HEADER));
    }
}

void CNoteDataRW::FileData2NoteData(const char *pBuffer, INT32 s32FileSize, std::vector<ST_NOTE_DATA> &vectorNoteData)
{
    vectorNoteData.clear();
    if (pBuffer != nullptr && s32FileSize > 0)
    {
        vectorNoteData.resize(s32FileSize / sizeof(ST_NOTE_DATA));
        memcpy(vectorNoteData.data(), pBuffer, s32FileSize);
    }
}


