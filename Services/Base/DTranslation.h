/*
 * @file: DTranslation.h
 * @brief: 
 * @author: nuo
 * @date: 2026/9/22
 * @Detail:
 */

#pragma once

static bool ToLocalTm(time_t t, std::tm& tmOut)
{
#if defined(_WIN32)
    return localtime_s(&tmOut, &t) == 0;
#else
    return localtime_r(&t, &tmOut) != nullptr;
#endif
}

static INT64 LocalDayNumber(time_t t)
{
    std::tm tmBuf{};
    if (!ToLocalTm(t, tmBuf))
    {
        return 0;
    }

    tmBuf.tm_hour = 12;      // 取正午，避开夏令时当天 0 点 / 23 点的边界
    tmBuf.tm_min  = 0;
    tmBuf.tm_sec  = 0;

    return static_cast<INT64>(mktime(&tmBuf)) / 86400;
}
