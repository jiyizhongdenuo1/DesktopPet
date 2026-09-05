/*
 * @file: CNoteDataCache.h
 * @brief: 
 * @author: nuo
 * @date: 2026/7/25
 * @Detail:
 */

#include <shared_mutex>
#include "CNoteDataCache.h"
#include "DDataCache.h"
#include <array>
#include <cstring>

#include <mutex>

using namespace std;
class CNoteDataCachePrivate
{
    friend class CNoteDataCache;
public:
    explicit CNoteDataCachePrivate() = default;
    ~CNoteDataCachePrivate() = default;

    void ClearCache();

private:
    shared_ptr<array<ST_NOTE_DATA, DDataCache::MAX_CACHE_SIZE>> m_parrNoteData;
    INT32 m_s32WritePos = 0;
    INT32 m_s32Count = 0;
    std::shared_mutex m_shareMutex;
};

void CNoteDataCachePrivate::ClearCache()
{
    if (!m_parrNoteData)
    {
        return;
    }
    m_s32WritePos = 0;
    m_s32Count = 0;
    unique_lock<shared_mutex> lock(m_shareMutex);
    memset(m_parrNoteData->data(), 0, m_parrNoteData->size() * sizeof(ST_NOTE_DATA));
}

void CNoteDataCache::InvalidateCache()
{
    d_ptr->ClearCache();
}

std::shared_ptr<std::array<ST_NOTE_DATA, DDataCache::MAX_CACHE_SIZE>> CNoteDataCache::GetCache()
{
    shared_lock<shared_mutex> lock(d_ptr->m_shareMutex);
    return d_ptr->m_parrNoteData;
}

void CNoteDataCache::SaveNoteDataCache(const ST_NOTE_DATA &stNoteData)
{
    unique_lock<shared_mutex> lock(d_ptr->m_shareMutex);
    if (d_ptr->m_parrNoteData == nullptr)
    {
        d_ptr->m_parrNoteData = make_shared<array<ST_NOTE_DATA, DDataCache::MAX_CACHE_SIZE>>();
    }
    (*d_ptr->m_parrNoteData)[d_ptr->m_s32WritePos] = move(stNoteData);
    d_ptr->m_s32WritePos = (d_ptr->m_s32WritePos + 1) % DDataCache::MAX_CACHE_SIZE;
    if (d_ptr->m_s32Count < DDataCache::MAX_CACHE_SIZE)
    {
        ++d_ptr->m_s32Count;
    }
}

int CNoteDataCache::PutBuffer2CacheData(char *pBuffer, INT32 s32BufferSize)
{
    if (!pBuffer || s32BufferSize <= 0 )
    {
        return 0;
    }
    unique_lock<shared_mutex> lock(d_ptr->m_shareMutex);
    if (d_ptr->m_parrNoteData == nullptr)
    {
        d_ptr->m_parrNoteData = make_shared<array<ST_NOTE_DATA, DDataCache::MAX_CACHE_SIZE>>();
    }
    INT32 s32_PutCount = min(s32BufferSize / static_cast<INT32>(sizeof(ST_NOTE_DATA)), DDataCache::MAX_CACHE_SIZE);
    memcpy(d_ptr->m_parrNoteData->data(), pBuffer, s32_PutCount * sizeof(ST_NOTE_DATA));
    d_ptr->m_s32WritePos = s32_PutCount % DDataCache::MAX_CACHE_SIZE;
    d_ptr->m_s32Count = s32_PutCount;
    return s32_PutCount;
}

INT32 CNoteDataCache::GetCacheSize()
{
    return d_ptr->m_s32Count * sizeof(ST_NOTE_DATA);
}

CNoteDataCache::CNoteDataCache()
    : d_ptr(make_unique<CNoteDataCachePrivate>())
{

}

CNoteDataCache::~CNoteDataCache() = default;