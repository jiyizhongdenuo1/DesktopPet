/*
* @file: CLogThreadHandler.h
 * @brief: 日志线程处理器
 * @author: nuo
 * @date: 2026/6/4
 * @Detail: 负责日志的收集、存储和管理
 */

#pragma once

#include <QVariantHash>

#include "datatype.h"
#include "CThreadHandler.h"

constexpr std::string MSG_WRITE_LOG = "write_log";
constexpr std::string MSG_STOP_REC_LOG = "stop_rec_log";

typedef struct stLogEvent
{
public:
    std::string     strMsgKey;
    QVariantHash    params;
}ST_LOG_EVENT;

class CLogThreadHandlerPrivate;

class CLogThreadHandler : public CThreadHandler
{
    Q_DECLARE_PRIVATE(CLogThreadHandler)
public:
    CLogThreadHandler();
    ~CLogThreadHandler();

    virtual void HandleTask() override;
    void AddTask(const std::string &strKey, const QVariantHash &params);
    void SetCanWriteLog();
    void StopRecLogForce();

private:
    void LogInit();
    void LogWrite2File(const char* log, qint64 size);
    bool LogFileRotate();

private:
    std::unique_ptr<CLogThreadHandlerPrivate> d_ptr;
};

