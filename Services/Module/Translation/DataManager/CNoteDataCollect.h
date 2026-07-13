/*
 * @file: CNoteDataCollect.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/23
 * @Detail:
 */

#pragma once

#include "DDataMgrBase.h"
#include "datatype.h"
#include "CDataCollectBase.h"

class CNoteDataColloctPrivate;
class CNoteDataCollect: public CDataCollectBase
{
public:
    explicit CNoteDataCollect(INT32 s32BufferSize);

    ~CNoteDataCollect();

    void AddNoteData(const NOTE_MODEL_ITEM &noteData);
    void ClearUnSaveNoteData();

    BOOL TickEvent();
private:
    std::unique_ptr<CNoteDataColloctPrivate> d_ptr;
};

