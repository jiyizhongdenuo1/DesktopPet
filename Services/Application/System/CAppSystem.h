/*
 * @file: CAppSystem.h
 * @brief: 应用系统管理类（线程管理、任务调度）
 * @author: nuo
 * @date: 2026/6/4
 * @update: 2026/9/4 - 移除 QObject 依赖，优化为纯 C++ 类
 */

#pragma once

#include <memory>

#include "datatype.h"
#include "GlobalEnums.h"

class CThread;
class CThreadHandler;
class CAppSystemPrivate;

class CAppSystem
{
public:
    explicit CAppSystem();
    ~CAppSystem();

    static CAppSystem *GetInstance();
    void IniAppFrame();

    VOID SetThreadHandler(E_THREAD_ID eThreadId, CThreadHandler *pThreadHandler);

    INT32 DoSecEvent();

private:
    void AddSaveDataTask();
    void SaveDataSeconed();
    void CreateThread();
    shared_ptr<CThreadHandler> GetThreadHandler(E_THREAD_ID eThreadId);
    void StartThread();
    void InitSystem();
public:
    static CAppSystem                   *m_pInstance;
    std::unique_ptr<CAppSystemPrivate>  d_ptr;
};

#define g_CAppSystem CAppSystem::GetInstance()