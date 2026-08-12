/*
 * @file: CThreadHandler.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/2
 * @Detail:
 */

#pragma once

#include <memory>
#include <Business_G.h>
#include  "datatype.h"

class CThread;
class Q_DECL_EXPORT CThreadHandler
{
public:
    friend class CThread;
    CThreadHandler()
    {

    }

    ~CThreadHandler()
    {

    }

    virtual void Init()
    {

    }
    virtual void HandleTask() = 0;
private:
    void AttachThread(std::shared_ptr<CThread> pThread){ m_pThread = pThread;}
protected:
    std::weak_ptr<CThread> m_pThread;
};