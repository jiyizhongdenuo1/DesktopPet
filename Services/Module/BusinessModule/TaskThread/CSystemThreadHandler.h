/*
 * @file: CSystemThreadHandler.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/3
 * @Detail:
 */

#pragma once

#include "CThreadHandler.h"
class CSystemThreadHandler: public CThreadHandler
{
public:
    explicit CSystemThreadHandler();
    ~CSystemThreadHandler();
    virtual void HandleTask();

};
