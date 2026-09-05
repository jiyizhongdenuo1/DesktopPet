/*
 * @file: CAppSystem.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/4
 * @Detail:
 */

#pragma once


#include <QObject>

#include "datatype.h"
#include "GlobalEnums.h"

class CThread;
class CThreadHandler;
class CAppSystemPrivate;
class CAppSystem: public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CAppSystem)
public:
    explicit CAppSystem(QObject *parent = nullptr);
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
