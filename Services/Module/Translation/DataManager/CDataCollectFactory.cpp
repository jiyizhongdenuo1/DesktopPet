/*
 * @file: CDataCollectFactory.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/28
 * @Detail:
 */

#include "CDataCollectFactory.h"

#include "CNoteDataCollect.h"
#include "DServiceBase.h"
using namespace std;

CDataCollectFactory * CDataCollectFactory::m_pInstance = nullptr;
CDataCollectFactory::CDataCollectFactory()
{

}

CDataCollectFactory * CDataCollectFactory::GetInstance()
{
    if (!m_pInstance)
    {
        m_pInstance = new CDataCollectFactory();
    }
    return m_pInstance;
}

std::shared_ptr<CDataCollectBase> CDataCollectFactory::GetCDataCollect(E_COLLECTION_TYPE type)
{
    if (m_pCDataCollect.find(type) == m_pCDataCollect.end())
    {
        m_pCDataCollect[type] = CreateCDataCollect(type);
    }
    return m_pCDataCollect[type];
}

std::shared_ptr<CDataCollectBase> CDataCollectFactory::CreateCDataCollect(E_COLLECTION_TYPE type)
{
    switch (type)
    {
        case E_COLLECTION_TYPE_NOTE:
            {
                return std::make_shared<CNoteDataCollect>(NDataManager::NOTE_BUFFER_ITEM_COUNT_MAX * sizeof(ST_NOTE_DATA));
            }
            break;
        default:
            return nullptr;
    }
}
