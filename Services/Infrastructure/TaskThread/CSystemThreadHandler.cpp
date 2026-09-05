/*
 * @file: CSystemThreadHandler.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/3
 * @Detail:
 */

#include "CSystemThreadHandler.h"

CSystemThreadHandler::CSystemThreadHandler()
{

}

CSystemThreadHandler::~CSystemThreadHandler()
{
}

void CSystemThreadHandler::HandleTask()
{
    if (m_func)
    {
        m_func();
    }
}

void CSystemThreadHandler::SetSystemThreadFunc(SYSTEM_THREAD_FUNC func)
{
    m_func = func;
}

