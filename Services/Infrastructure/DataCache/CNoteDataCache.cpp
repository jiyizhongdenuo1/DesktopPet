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
    array<NOTE_ITEM_PTR, DDataCache::MAX_CACHE_SIZE> m_arrNoteData;
    INT32 m_s32WritePos = 0;
    INT32 m_s32Count = 0;

    array<NOTE_ITEM_PTR, DDataCache::MAX_CACHE_SIZE> m_arrRecycleData;
    INT32 m_s32RecycleWritePos = 0;
    INT32 m_s32RecycleCount = 0;

    std::shared_mutex m_shareMutex;

    std::atomic<NOTE_CACHE_SNAPSHOT> m_pNoteCacheSnapshot;
    std::atomic<NOTE_CACHE_SNAPSHOT> m_pRecycleSnapshot;

    CACHE_CHANGE_CALLBACK m_ChangeCallback;
};

void CNoteDataCache::ClearCache()
{
    unique_lock<shared_mutex> lock(d_ptr->m_shareMutex);
    d_ptr->m_s32WritePos = 0;
    d_ptr->m_s32Count = 0;
    d_ptr->m_arrNoteData.fill(nullptr);

    d_ptr->m_s32RecycleWritePos = 0;
    d_ptr->m_s32RecycleCount = 0;
    d_ptr->m_arrRecycleData.fill(nullptr);

    PublishSnapshot();
    PublishRecycleSnapshot();
}

void CNoteDataCache::SaveNoteDataCache(const ST_NOTE_DATA &stNoteData)
{
    CACHE_CHANGE_CALLBACK cb;
    SNoteCacheChange stChange;
    {
        unique_lock<shared_mutex> lock(d_ptr->m_shareMutex);

        if (d_ptr->m_s32WritePos >= 0 && d_ptr->m_s32WritePos < static_cast<int>(DDataCache::MAX_CACHE_SIZE))
        {
            auto p_Item = make_shared<const ST_NOTE_DATA>(stNoteData);
            d_ptr->m_arrNoteData[d_ptr->m_s32WritePos] = p_Item;
            d_ptr->m_s32WritePos = (d_ptr->m_s32WritePos + 1) % DDataCache::MAX_CACHE_SIZE;
            if (d_ptr->m_s32Count < DDataCache::MAX_CACHE_SIZE)
            {
                ++d_ptr->m_s32Count;
            }
            PublishSnapshot();

            stChange.eType     = E_CACHE_CHANGE_ADD;
            stChange.s32NoteId = p_Item->m_s32id;
            stChange.pData     = p_Item;
            cb = d_ptr->m_ChangeCallback;
        }
    }

    if (cb)
    {
        cb(stChange);
    }
}

void CNoteDataCache::UpdateNoteDataCache(const ST_NOTE_DATA &stNoteData)
{
    CACHE_CHANGE_CALLBACK cb;
    SNoteCacheChange stChange;
    {
        unique_lock<shared_mutex> lock(d_ptr->m_shareMutex);
        for (INT32 i = 0; i < d_ptr->m_s32Count; ++i)
        {
            if (d_ptr->m_arrNoteData.at(i) && stNoteData.m_s32id == d_ptr->m_arrNoteData.at(i)->m_s32id)
            {
                auto p_Item = make_shared<const ST_NOTE_DATA>(stNoteData);
                d_ptr->m_arrNoteData.at(i) = p_Item;
                PublishSnapshot();

                stChange.eType     = E_CACHE_CHANGE_UPDATE;
                stChange.s32NoteId = p_Item->m_s32id;
                stChange.pData     = p_Item;
                cb = d_ptr->m_ChangeCallback;
                break;
            }
        }
    }

    if (cb)
    {
        cb(stChange);
    }
}

void CNoteDataCache::AppendNoteData(const ST_NOTE_DATA *pArrNote, INT32 s32Count)
{
    if (pArrNote == nullptr || s32Count <= 0)
    {
        return;
    }

    CACHE_CHANGE_CALLBACK cb;
    vector<SNoteCacheChange> vecChanges;
    {
        unique_lock<shared_mutex> lock(d_ptr->m_shareMutex);
        for (INT32 i = 0; i < s32Count; ++i)
        {
            auto p_Item = make_shared<const ST_NOTE_DATA>(pArrNote[i]);
            d_ptr->m_arrNoteData[d_ptr->m_s32WritePos] = p_Item;
            d_ptr->m_s32WritePos = (d_ptr->m_s32WritePos + 1) % DDataCache::MAX_CACHE_SIZE;
            if (d_ptr->m_s32Count < DDataCache::MAX_CACHE_SIZE)
            {
                ++d_ptr->m_s32Count;
            }
            vecChanges.push_back({E_CACHE_CHANGE_ADD, p_Item->m_s32id, p_Item});
        }

        PublishSnapshot();      // 整批只发布一次
        cb = d_ptr->m_ChangeCallback;
    }

    if (cb)
    {
        for (const auto &stChange : vecChanges)
        {
            cb(stChange);
        }
    }
}

int CNoteDataCache::PutBuffer2CacheData(char *pBuffer, INT32 s32BufferSize)
{
    if (!pBuffer || s32BufferSize <= 0 )
    {
        return 0;
    }
    unique_lock<shared_mutex> lock(d_ptr->m_shareMutex);
    INT32 s32_PutCount = min(s32BufferSize / static_cast<INT32>(sizeof(ST_NOTE_DATA)), DDataCache::MAX_CACHE_SIZE);
    const auto *pArrNote = reinterpret_cast<const ST_NOTE_DATA *>(pBuffer);
    for (INT32 i = 0; i < s32_PutCount; ++i)
    {
        d_ptr->m_arrNoteData[i] = make_shared<const ST_NOTE_DATA>(pArrNote[i]);
    }
    d_ptr->m_s32WritePos = s32_PutCount % DDataCache::MAX_CACHE_SIZE;
    d_ptr->m_s32Count = s32_PutCount;

    PublishSnapshot();
    return s32_PutCount;
}

INT32 CNoteDataCache::PutBuffer2RecycleData(char *pBuffer, INT32 s32BufferSize)
{
    if (!pBuffer || s32BufferSize <= 0)
    {
        return 0;
    }
    unique_lock<shared_mutex> lock(d_ptr->m_shareMutex);
    INT32 s32_PutCount = min(s32BufferSize / static_cast<INT32>(sizeof(ST_NOTE_DATA)), DDataCache::MAX_CACHE_SIZE);
    const auto *pArrRecycle = reinterpret_cast<const ST_NOTE_DATA *>(pBuffer);
    for (INT32 i = 0; i < s32_PutCount; ++i)
    {
        d_ptr->m_arrRecycleData[i] = make_shared<const ST_NOTE_DATA>(pArrRecycle[i]);
    }
    d_ptr->m_s32RecycleWritePos = s32_PutCount % DDataCache::MAX_CACHE_SIZE;
    d_ptr->m_s32RecycleCount = s32_PutCount;

    PublishRecycleSnapshot();
    return s32_PutCount;
}

void CNoteDataCache::AppendRecycleNoteData(const ST_NOTE_DATA *pArrNote, INT32 s32Count)
{
    if (pArrNote == nullptr || s32Count <= 0)
    {
        return;
    }
    unique_lock<shared_mutex> lock(d_ptr->m_shareMutex);
    for (INT32 i = 0; i < s32Count; ++i)
    {
        d_ptr->m_arrRecycleData[d_ptr->m_s32RecycleWritePos] = make_shared<const ST_NOTE_DATA>(pArrNote[i]);
        d_ptr->m_s32RecycleWritePos = (d_ptr->m_s32RecycleWritePos + 1) % DDataCache::MAX_CACHE_SIZE;
        if (d_ptr->m_s32RecycleCount < DDataCache::MAX_CACHE_SIZE)
        {
            ++d_ptr->m_s32RecycleCount;
        }
    }

    PublishRecycleSnapshot();
}

CNoteDataCache::CNoteDataCache()
    : d_ptr(make_unique<CNoteDataCachePrivate>())
{
    PublishSnapshot();
    PublishRecycleSnapshot();
}

void CNoteDataCache::InvalidateCache()
{
    ClearCache();
}

NOTE_CACHE_SNAPSHOT CNoteDataCache::GetSnapshot() const
{
    return d_ptr->m_pNoteCacheSnapshot.load();
}

NOTE_CACHE_SNAPSHOT CNoteDataCache::GetRecycleSnapshot() const
{
    return d_ptr->m_pRecycleSnapshot.load();
}

void CNoteDataCache::SetChangeCallback(CACHE_CHANGE_CALLBACK cb)
{
    unique_lock<shared_mutex> lock(d_ptr->m_shareMutex);
    d_ptr->m_ChangeCallback = std::move(cb);
}

INT32 CNoteDataCache::GetCacheSize() const
{
    auto p_Snapshot = GetSnapshot();
    return p_Snapshot ? static_cast<INT32>(p_Snapshot->size() * sizeof(ST_NOTE_DATA)) : 0;
}

void CNoteDataCache::PublishSnapshot()
{
    auto p_Temp = make_shared<const std::vector<NOTE_ITEM_PTR>> (d_ptr->m_arrNoteData.begin(),
                                                                d_ptr->m_arrNoteData.begin() + d_ptr->m_s32Count);

    d_ptr->m_pNoteCacheSnapshot.store(move(p_Temp));
}

void CNoteDataCache::PublishRecycleSnapshot()
{
    auto p_Temp = make_shared<const std::vector<NOTE_ITEM_PTR>> (d_ptr->m_arrRecycleData.begin(),
                                                                d_ptr->m_arrRecycleData.begin() + d_ptr->m_s32RecycleCount);

    d_ptr->m_pRecycleSnapshot.store(move(p_Temp));
}

CNoteDataCache::~CNoteDataCache() = default;
