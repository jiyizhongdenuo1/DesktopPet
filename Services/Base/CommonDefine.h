/*
 * @file: CommonDefind.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/14
 * @Detail:
 */

#pragma once
#include <string>

namespace CPath
{
    constexpr char STORE_PATH[] = "./config/";
    constexpr char COMMON_COFNIG_FILE_NAME[] = "CommonConfig.json";
    constexpr char NOTE_FILE_NAME[] = "NoteData";
}

namespace CLimit
{
    constexpr int MAX_FILE_STORE_LIMIT = 10000;
}