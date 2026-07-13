/*
 * @file: CSystemThreadHandler.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/3
 * @Detail:
 */

#pragma once
#include "datatype.h"
#include <functional>
typedef std::function<INT32()> SYSTEM_THREAD_FUNC;

#include "CThreadHandler.h"
class CSystemThreadHandler: public CThreadHandler
{
public:
    explicit CSystemThreadHandler();
    ~CSystemThreadHandler();
    virtual void HandleTask();
    void SetSystemThreadFunc(SYSTEM_THREAD_FUNC func);

private:
    SYSTEM_THREAD_FUNC m_func;
};
