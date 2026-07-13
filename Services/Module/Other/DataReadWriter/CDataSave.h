/*
 * @file: CDataSave.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/12
 * @Detail:
 */

#pragma once

#include <memory>
#include "Business_G.h"
class CDataSavePrivate;
class BUSINESS_EXPORT CDataSave
{
public:
    explicit CDataSave();

    ~CDataSave();

    void SaveNoteData() const;
private:
    std::unique_ptr<CDataSavePrivate> d_ptr;

};
