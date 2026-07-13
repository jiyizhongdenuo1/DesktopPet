/*
 * @file: CDataCollectFactory.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/28
 * @Detail:
 */

#pragma once
#include <memory>
#include <map>

enum E_COLLECTION_TYPE
{
    E_COLLECTION_TYPE_NOTE = 0,

    E_COLLECTION_TYPE_MAX,
};
class CDataCollectBase;
class CDataCollectFactory
{
public:
    explicit CDataCollectFactory();

    ~CDataCollectFactory() = default;

    static CDataCollectFactory *GetInstance();

    std::shared_ptr<CDataCollectBase> GetCDataCollect(E_COLLECTION_TYPE type);
private:
    std::shared_ptr<CDataCollectBase> CreateCDataCollect(E_COLLECTION_TYPE type);

private:
    std::map<E_COLLECTION_TYPE, std::shared_ptr<CDataCollectBase>> m_pCDataCollect;
    static CDataCollectFactory *m_pInstance;
};

#define g_CDataCollectFactory CDataCollectFactory::GetInstance()
