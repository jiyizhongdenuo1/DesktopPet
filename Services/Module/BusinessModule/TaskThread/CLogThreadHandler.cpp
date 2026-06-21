/*
* @file: CLogThreadHandler.cpp
 * @brief: 日志线程处理器实现
 * @author: nuo
 * @date: 2026/6/4
 * @Detail: 负责日志的收集、存储和管理
 */

#include "CLogThreadHandler.h"
#include <memory>

#include <QDebug>
#include <QFileInfo>
#include <queue>
#include <QMutex>
#include <QQueue>
#include <QFile>

#include "CThread.h"
#define DATA_SAVE_PATh "./SaveFile"
#define SAVE_FILE_NAME "LogFile.json"


class CLogThreadHandlerPrivate
{
    Q_DECLARE_PUBLIC(CLogThreadHandler)
    friend class CLogThreadHandler;

public:
    explicit CLogThreadHandlerPrivate(CLogThreadHandler *q)
        : q_ptr(q)
        , m_bIsWriteEnable(false)
        , m_bStop(false)
        , m_s32MaxSize(1024 * 1024 * 10)  // 10MB
        , m_s32MaxRotate(5)
    {}

private:
    CLogThreadHandler *q_ptr;
    bool                    m_bIsWriteEnable;
    bool                    m_bStop;
    INT32                   m_s32MaxSize;
    INT32                   m_s32MaxRotate;
    QFile                   m_file;
    QString                 m_strFileName;
    QQueue<ST_LOG_EVENT>    m_queLogData;
    QMutex                  m_Mutex;
    QStringList             m_strlist;
};

CLogThreadHandler::CLogThreadHandler()
    : d_ptr(new CLogThreadHandlerPrivate(this))
{
    Q_D(CLogThreadHandler);
    d->m_bIsWriteEnable = true;
    d->m_s32MaxSize = 10000;
    d->m_s32MaxRotate = 10;
    LogInit();
}

CLogThreadHandler::~CLogThreadHandler()
{
    qInstallMessageHandler(0);

    Q_D(CLogThreadHandler);
    if (d->m_file.isOpen())
    {
        d->m_file.close();
    }
}

void CLogThreadHandler::HandleTask()
{
    Q_D(CLogThreadHandler);

    while (true)
    {
        d->m_Mutex.lock();
        if (d->m_queLogData.empty())
        {
            d->m_Mutex.unlock();
            break;
        }
        stLogEvent event = d->m_queLogData.dequeue();
        d->m_Mutex.unlock();

        if (event.strMsgKey == MSG_WRITE_LOG)
        {
            QString str = event.params.value("addlog").toString();
            if (d->m_strlist.size() < 2000)
            {
                d->m_strlist.append(str);
            }

            if (d->m_bStop)
            {
                break;
            }

            while (d->m_strlist.size() > 0)
            {
                QString str_Text = d->m_strlist.takeFirst();
                QByteArray data = str_Text.toUtf8();
                LogWrite2File(data.constData(), data.size());
            }
        }
        else if (event.strMsgKey == MSG_STOP_REC_LOG)
        {
            if (d->m_file.isOpen())
            {
                d->m_file.close();
            }
            d->m_bStop = true;
        }
    }
}
