/*
 * @file: DSaveDefine.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/30
 * @Detail:
 */

#pragma once

#include <string>
#include <QVariantHash>

namespace DSaveDefine
{
    constexpr INT32 SINGLE_SAVE_NOTE_DATA_COUNT = 10;
    constexpr std::string NOTE_FILE_NAME = "NoteData.bin";
    constexpr INT64  TRUNCATE_KEEP_RATIO_PERCENT = 80;
}