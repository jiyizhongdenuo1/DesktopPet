/*
* @file: CLogThreadHandler.cpp
 * @brief: 日志线程处理器实现
 * @author: nuo
 * @date: 2026/6/4
 * @Detail: 负责日志的收集、存储和管理
 */

#include "CLogThreadHandler.h"
#include <memory>
#include <mutex>
#include <queue>
#include <QFile>
#include <QDateTime>
#include <QDir>

#include "CThread.h"
#define DATA_SAVE_PATh "./SaveFile"
#define SAVE_FILE_NAME "LogFile.json"

static CLogThreadHandler *g_pLogHandler = nullptr;

static void LogMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (!g_pLogHandler)
    {
        return;
    }

    QString str_Level;
    switch (type)
    {
    case QtDebugMsg:    str_Level = "DEBUG"; break;
    case QtInfoMsg:     str_Level = "INFO";  break;
    case QtWarningMsg:  str_Level = "WARN";  break;
    case QtCriticalMsg: str_Level = "ERROR"; break;
    case QtFatalMsg:    str_Level = "FATAL"; break;
    }

    QString str_Formatted = QString("[%1] [%2] %3:%4 - %5\n")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
        .arg(str_Level)
        .arg(context.file ? context.file : "unknown")
        .arg(context.line)
        .arg(msg);

    g_pLogHandler->AddTask(MSG_WRITE_LOG, {{"addlog", str_Formatted.toStdString()}});
}


class CLogThreadHandlerPrivate
{
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
    CLogThreadHandler                *q_ptr;
    bool                              m_bIsWriteEnable;
    bool                              m_bStop;
    INT32                             m_s32MaxSize;
    INT32                             m_s32MaxRotate;
    QFile                             m_file;
    QString                           m_strFileName;
    std::queue<ST_LOG_EVENT>          m_queLogData;
    std::mutex                        m_Mutex;
    QStringList                       m_strlist;
};

CLogThreadHandler::CLogThreadHandler()
    : d_ptr(new CLogThreadHandlerPrivate(this))
{
    d_ptr->m_bIsWriteEnable = true;
    d_ptr->m_s32MaxSize = 10000;
    d_ptr->m_s32MaxRotate = 10;
    LogInit();
}

CLogThreadHandler::~CLogThreadHandler()
{
    qInstallMessageHandler(0);

    if (d_ptr->m_file.isOpen())
    {
        d_ptr->m_file.close();
    }
}

void CLogThreadHandler::HandleTask()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(d_ptr->m_Mutex);
        if (d_ptr->m_queLogData.empty())
        {
            return;
        }
        ST_LOG_EVENT event = d_ptr->m_queLogData.front();
        d_ptr->m_queLogData.pop();
        lock.unlock();

        if (event.strMsgKey == MSG_WRITE_LOG)
        {
            auto it = event.params.find("addlog");
            if (it != event.params.end())
            {
                QString str = QString::fromStdString(it->second);
                if (d_ptr->m_strlist.size() < 2000)
                {
                    d_ptr->m_strlist.append(str);
                }
            }

            if (d_ptr->m_bStop)
            {
                break;
            }

            while (d_ptr->m_strlist.size() > 0)
            {
                QString str_Text = d_ptr->m_strlist.takeFirst();
                QByteArray data = str_Text.toUtf8();
                LogWrite2File(data.constData(), data.size());
            }
        }
        else if (event.strMsgKey == MSG_STOP_REC_LOG)
        {
            if (d_ptr->m_file.isOpen())
            {
                d_ptr->m_file.close();
            }
            d_ptr->m_bStop = true;
        }
    }
}

void CLogThreadHandler::AddTask(const std::string &strKey, const std::unordered_map<std::string, std::string> &mapParams)
{
    static constexpr INT32 s32_MaxLimit = 2000;
    ST_LOG_EVENT st_Event;
    st_Event.strMsgKey = strKey;
    st_Event.params = mapParams;
    std::lock_guard<std::mutex> lock(d_ptr->m_Mutex);
    if (d_ptr->m_queLogData.size() < s32_MaxLimit)
    {
        d_ptr->m_queLogData.push(st_Event);
    }
    auto pThread = m_pThread.lock();
    if (pThread)
    {
        pThread->WakeUp(1);
    }
}

void CLogThreadHandler::LogInit()
{
    g_pLogHandler = this;
    qInstallMessageHandler(LogMessageHandler);
}

void CLogThreadHandler::LogWrite2File(const char *log, qint64 size)
{
    if (!log || size <= 0)
    {
        return;
    }

    if (!d_ptr->m_file.isOpen())
    {
        d_ptr->m_strFileName = QString(DATA_SAVE_PATh) + "/" + SAVE_FILE_NAME;
        QFileInfo fileInfo(d_ptr->m_strFileName);
        QDir dir = fileInfo.dir();
        if (!dir.exists())
        {
            dir.mkpath(".");
        }
        d_ptr->m_file.setFileName(d_ptr->m_strFileName);
        if (!d_ptr->m_file.open(QIODevice::Append | QIODevice::WriteOnly))
        {
            return;
        }
    }

    d_ptr->m_file.write(log, size);
    d_ptr->m_file.flush();

    if (d_ptr->m_file.size() > d_ptr->m_s32MaxSize)
    {
        LogFileRotate();
    }
}

bool CLogThreadHandler::LogFileRotate()
{
    d_ptr->m_file.close();

    QString str_Oldest = d_ptr->m_strFileName + "." + QString::number(d_ptr->m_s32MaxRotate);
    if (QFile::exists(str_Oldest))
    {
        QFile::remove(str_Oldest);
    }

    for (INT32 s32_I = d_ptr->m_s32MaxRotate - 1; s32_I >= 1; --s32_I)
    {
        QString str_Src = d_ptr->m_strFileName + "." + QString::number(s32_I);
        QString str_Dst = d_ptr->m_strFileName + "." + QString::number(s32_I + 1);
        if (QFile::exists(str_Src))
        {
            QFile::rename(str_Src, str_Dst);
        }
    }

    QFile::rename(d_ptr->m_strFileName, d_ptr->m_strFileName + ".1");

    d_ptr->m_file.setFileName(d_ptr->m_strFileName);
    d_ptr->m_file.open(QIODevice::Append | QIODevice::WriteOnly);
    return true;
}