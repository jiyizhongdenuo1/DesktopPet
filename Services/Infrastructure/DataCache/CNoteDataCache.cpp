/*
 * @file: CNoteDataCache.h
 * @brief: 
 * @author: nuo
 * @date: 2026/7/25
 * @Detail:
 */

#include <shared_mutex>
#include <array>
#include <cstring>
#include <atomic>
#include <mutex>

#include "CNoteDataCache.h"
#include "DDataCache.h"
using namespace std;
class CNoteDataCachePrivate
{
    friend class CNoteDataCache;
public:
    explicit CNoteDataCachePrivate() = default;
    ~CNoteDataCachePrivate() = default;

private:
    array<ST_NOTE_DATA, DDataCache::MAX_CACHE_SIZE> m_arrNoteData;
    INT32 m_s32WritePos = 0;
    INT32 m_s32Count = 0;
    std::shared_mutex m_shareMutex;

    std::atomic<NOTE_CACHE_SNAPSHOT> m_pNoteCacheSnapshot;
};

void CNoteDataCache::ClearCache()
{
    unique_lock<shared_mutex> lock(d_ptr->m_shareMutex);
    d_ptr->m_s32WritePos = 0;
    d_ptr->m_s32Count = 0;
    memset(d_ptr->m_arrNoteData.data(), 0, d_ptr->m_arrNoteData.size() * sizeof(ST_NOTE_DATA));
    PublishSnapshot();
}

void CNoteDataCache::SaveNoteDataCache(const ST_NOTE_DATA &stNoteData)
{
    unique_lock<shared_mutex> lock(d_ptr->m_shareMutex);

    if (d_ptr->m_s32WritePos >= 0 && d_ptr->m_s32WritePos < static_cast<int>(DDataCache::MAX_CACHE_SIZE))
    {
        d_ptr->m_arrNoteData[d_ptr->m_s32WritePos] = move(stNoteData);
        d_ptr->m_s32WritePos = (d_ptr->m_s32WritePos + 1) % DDataCache::MAX_CACHE_SIZE;
        if (d_ptr->m_s32Count < DDataCache::MAX_CACHE_SIZE)
        {
            ++d_ptr->m_s32Count;
        }
    }

    PublishSnapshot();
}

void CNoteDataCache::UpdateNoteDataCache(const ST_NOTE_DATA &stNoteData)
{
    unique_lock<shared_mutex> lock(d_ptr->m_shareMutex);
    for (INT32 i = 0; i < d_ptr->m_s32Count; ++i)
    {
        if (stNoteData.m_s32id == d_ptr->m_arrNoteData.at(i).m_s32id)
        {
            d_ptr->m_arrNoteData.at(i) = stNoteData;
            PublishSnapshot();
            return ;
        }
    }
}

void CNoteDataCache::AppendNoteData(const ST_NOTE_DATA *pArrNote, INT32 s32Count)
{
    if (pArrNote == nullptr || s32Count <= 0)
    {
        return;
    }
    unique_lock<shared_mutex> lock(d_ptr->m_shareMutex);
    for (INT32 i = 0; i < s32Count; ++i)
    {
        d_ptr->m_arrNoteData[d_ptr->m_s32WritePos] = pArrNote[i];
        d_ptr->m_s32WritePos = (d_ptr->m_s32WritePos + 1) % DDataCache::MAX_CACHE_SIZE;
        if (d_ptr->m_s32Count < DDataCache::MAX_CACHE_SIZE)
        {
            ++d_ptr->m_s32Count;
        }
    }

    PublishSnapshot();      // 整批只发布一次
}

int CNoteDataCache::PutBuffer2CacheData(char *pBuffer, INT32 s32BufferSize)
{
    if (!pBuffer || s32BufferSize <= 0 )
    {
        return 0;
    }
    unique_lock<shared_mutex> lock(d_ptr->m_shareMutex);
    INT32 s32_PutCount = min(s32BufferSize / static_cast<INT32>(sizeof(ST_NOTE_DATA)), DDataCache::MAX_CACHE_SIZE);
    memcpy(d_ptr->m_arrNoteData.data(), pBuffer, s32_PutCount * sizeof(ST_NOTE_DATA));
    d_ptr->m_s32WritePos = s32_PutCount % DDataCache::MAX_CACHE_SIZE;
    d_ptr->m_s32Count = s32_PutCount;

    PublishSnapshot();
    return s32_PutCount;
}

CNoteDataCache::CNoteDataCache()
    : d_ptr(make_unique<CNoteDataCachePrivate>())
{
    PublishSnapshot();
}

void CNoteDataCache::InvalidateCache()
{
    ClearCache();
}

NOTE_CACHE_SNAPSHOT CNoteDataCache::GetSnapshot() const
{
    return d_ptr->m_pNoteCacheSnapshot.load();
}

INT32 CNoteDataCache::GetCacheSize() const
{
    auto p_Snapshot = GetSnapshot();
    return p_Snapshot ? static_cast<INT32>(p_Snapshot->size() * sizeof(ST_NOTE_DATA)) : 0;
}

void CNoteDataCache::PublishSnapshot()
{
    auto p_Temp = make_shared<const std::vector<ST_NOTE_DATA>> (d_ptr->m_arrNoteData.begin(),
                                                               d_ptr->m_arrNoteData.begin() + d_ptr->m_s32Count);

    d_ptr->m_pNoteCacheSnapshot.store(move(p_Temp));
}

CNoteDataCache::~CNoteDataCache() = default;