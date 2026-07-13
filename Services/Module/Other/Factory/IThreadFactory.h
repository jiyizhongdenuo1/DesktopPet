/*
 * @file: IThreadFactory.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/5
 * @Detail:
 */

#pragma once
#include "datatype.h"

class IThreadFactory
{
public:
    explicit IThreadFactory();

    ~IThreadFactory();
    virtual VOID Create() = 0;
};
