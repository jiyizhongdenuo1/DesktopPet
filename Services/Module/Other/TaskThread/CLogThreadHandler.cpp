/*
* @file: CLogThreadHandler.cpp
 * @brief: 日志线程处理器实现
 * @author: nuo
 * @date: 2026/6/4
 * @Detail: 负责日志的收集、存储和管理
 */

#include "CLogThreadHandler.h"
#include <QDebug>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <deque>
#include <fstream>
#include <filesystem>
#include <memory>
#include <mutex>
#include <queue>

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

    const char *szLevel = "DEBUG";
    switch (type)
    {
    case QtDebugMsg:    szLevel = "DEBUG"; break;
    case QtInfoMsg:     szLevel = "INFO";  break;
    case QtWarningMsg:  szLevel = "WARN";  break;
    case QtCriticalMsg: szLevel = "ERROR"; break;
    case QtFatalMsg:    szLevel = "FATAL"; break;
    }

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::tm tm_now{};
    localtime_r(&time_t_now, &tm_now);
    char szTime[64];
    std::strftime(szTime, sizeof(szTime), "%Y-%m-%d %H:%M:%S", &tm_now);

    std::string strFormatted = std::string("[")
        + szTime + "." + std::to_string(ms.count())
        + "] [" + szLevel + "] "
        + (context.file ? context.file : "unknown") + ":"
        + std::to_string(context.line) + " - "
        + msg.toStdString() + "\n";

    fprintf(stderr, "%s", strFormatted.c_str());
    fflush(stderr);

    g_pLogHandler->AddTask(MSG_WRITE_LOG, {{"addlog", strFormatted}});
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
    std::ofstream                     m_fileStream;
    std::string                       m_strFileName;
    std::queue<ST_LOG_EVENT>          m_queLogData;
    std::mutex                        m_Mutex;
    std::deque<std::string>            m_strlist;
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

    if (d_ptr->m_fileStream.is_open())
    {
        d_ptr->m_fileStream.close();
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
                if (d_ptr->m_strlist.size() < 2000)
                {
                    d_ptr->m_strlist.push_back(it->second);
                }
            }

            if (d_ptr->m_bStop)
            {
                break;
            }

            while (!d_ptr->m_strlist.empty())
            {
                std::string str_Text = d_ptr->m_strlist.front();
                d_ptr->m_strlist.pop_front();
                LogWrite2File(str_Text);
            }
        }
        else if (event.strMsgKey == MSG_STOP_REC_LOG)
        {
            if (d_ptr->m_fileStream.is_open())
            {
                d_ptr->m_fileStream.close();
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
    qDebug() << "LogMessageHandler installed and working";
}

void CLogThreadHandler::LogWrite2File(const std::string &strLog)
{
    if (strLog.empty())
    {
        return;
    }

    if (!d_ptr->m_fileStream.is_open())
    {
        d_ptr->m_strFileName = std::string(DATA_SAVE_PATh) + "/" + SAVE_FILE_NAME;
        std::filesystem::path filePath(d_ptr->m_strFileName);
        std::filesystem::create_directories(filePath.parent_path());
        d_ptr->m_fileStream.open(d_ptr->m_strFileName, std::ios::app);
        if (!d_ptr->m_fileStream.is_open())
        {
            return;
        }
    }

    d_ptr->m_fileStream.write(strLog.data(), static_cast<std::streamsize>(strLog.size()));
    d_ptr->m_fileStream.flush();

    // 检查文件大小，超过阈值则轮转
    d_ptr->m_fileStream.seekp(0, std::ios::end);
    if (d_ptr->m_fileStream.tellp() > d_ptr->m_s32MaxSize)
    {
        LogFileRotate();
    }
}

bool CLogThreadHandler::LogFileRotate()
{
    d_ptr->m_fileStream.close();

    std::string str_Oldest = d_ptr->m_strFileName + "." + std::to_string(d_ptr->m_s32MaxRotate);
    if (std::filesystem::exists(str_Oldest))
    {
        std::filesystem::remove(str_Oldest);
    }

    for (INT32 s32_I = d_ptr->m_s32MaxRotate - 1; s32_I >= 1; --s32_I)
    {
        std::string str_Src = d_ptr->m_strFileName + "." + std::to_string(s32_I);
        std::string str_Dst = d_ptr->m_strFileName + "." + std::to_string(s32_I + 1);
        if (std::filesystem::exists(str_Src))
        {
            std::filesystem::rename(str_Src, str_Dst);
        }
    }

    std::filesystem::rename(d_ptr->m_strFileName, d_ptr->m_strFileName + ".1");

    d_ptr->m_fileStream.open(d_ptr->m_strFileName, std::ios::app);
    return true;
}