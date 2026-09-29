/*
 * @file: CServiceLocator.cpp
 * @brief: 全局服务定位器实现
 * @author: nuo
 * @date: 2026/8/1
 * @Detail:
 */

#include <iostream>

#include "CServiceLocator.h"
#include "CNoteDataCache.h"
#include "CNoteDataService.h"
#include "INoteDataBuffer.h"
#include "CDataRWMgr.h"


using namespace std;

CServiceLocator& CServiceLocator::GetInstance()
{
    static CServiceLocator instance;
    return instance;
}

void CServiceLocator::RegisterNoteCache(shared_ptr<CNoteDataCache> pCache)
{
    if (!pCache)
    {
        cerr << "[Warning] CServiceLocator::RegisterNoteCache() 接收到空指针，注册失败" << endl;
        return;
    }

    lock_guard<mutex> lock(m_mutex);
    m_pNoteCache = move(pCache);
}

void CServiceLocator::RegisterNoteService(shared_ptr<CNoteDataService> pService)
{
    if (!pService)
    {
        cerr << "[Warning] CServiceLocator::RegisterNoteService() 接收到空指针，注册失败" << endl;
        return;
    }

    lock_guard<mutex> lock(m_mutex);
    m_pNoteService = move(pService);
}

void CServiceLocator::RegisterDataSaver(shared_ptr<CDataRWMgr> pSaver)
{
    if (!pSaver)
    {
        cerr << "[Warning] CServiceLocator::RegisterDataSaver() 接收到空指针，注册失败" << endl;
    }

    lock_guard<mutex> lock(m_mutex);
    m_pDataSaveRWMgr = move(pSaver);
}

void CServiceLocator::RegisterNoteCollect(shared_ptr<INoteDataBuffer> pBuffer)
{
    if (!pBuffer)
    {
        cerr << "[Warning] CServiceLocator::RegisterNoteCollect() 接收到空指针，注册失败" << endl;
        return;
    }

    lock_guard<mutex> lock(m_mutex);
    m_pNoteCollect = move(pBuffer);
}

shared_ptr<CNoteDataCache> CServiceLocator::GetNoteCache() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_pNoteCache;
}

shared_ptr<CNoteDataService> CServiceLocator::GetNoteService() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_pNoteService;
}

shared_ptr<CDataRWMgr> CServiceLocator::GetDataRWMgr() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_pDataSaveRWMgr;
}

shared_ptr<INoteDataBuffer> CServiceLocator::GetNoteCollect() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_pNoteCollect;
}