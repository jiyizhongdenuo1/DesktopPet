/*
* @file: IDataCache.h
 * @brief:
 * @author: nuo
 * @date: 2026/9/21
 * @Detail:
 */

#pragma once

#include <memory>

#include "DServiceBase.h"
class IDataCache
{
public:
    explicit IDataCache() = default;

    virtual ~IDataCache() = default;

    // virtual void GetCache(std::shared_ptr<std::array<ST_NOTE_DATA, DDataCache::MAX_CACHE_SIZE>> pCache, INT32 s32Count) = 0;
};
