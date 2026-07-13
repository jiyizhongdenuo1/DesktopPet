/*
* @file: CDynsDataSaveThreadHandler.cpp
 * @brief: 动态数据保存线程处理器实现
 * @author: nuo
 * @date: 2026/6/10
 * @Detail: 负责数据保存任务的处理和调度
 */
#include "CDynsDataSaveThreadHandler.h"
#include <memory>
#include <queue>
#include <map>
#include <QMutex>
#include "CDataSave.h"
#include  "CThread.h"

using namespace std;

static void func_SaveNoteData(shared_ptr<CDataSave> pDataSave, DataSaveEvent &event)
{
    pDataSave->SaveNoteData();
    // pDataSave->ClearBuffer();
}
class CDynsDataSaveThreadHandlerPrivate
{
    friend class CDynsDataSaveThreadHandler;

public:
    explicit CDynsDataSaveThreadHandlerPrivate(CDynsDataSaveThreadHandler *q)
        : q_ptr(q)
        , m_pDataSave(make_shared<CDataSave>())
        , m_bIsExit(false)
    {

    }

private:
    typedef void (*DATASAVE_FUNC)(shared_ptr<CDataSave> pDataSave, DataSaveEvent &event);

    CDynsDataSaveThreadHandler              *q_ptr;
    shared_ptr<CDataSave>                   m_pDataSave;
    queue<DataSaveEvent>                    m_queSaveEvent;
    QMutex                                  m_mutex;
    map<string, DATASAVE_FUNC>              m_mapFunc;
    BOOL                                    m_bIsExit;
};

CDynsDataSaveThreadHandler::CDynsDataSaveThreadHandler()
    : d_ptr(new CDynsDataSaveThreadHandlerPrivate(this))
{
    d_ptr->m_mapFunc[DataSaveFucName::MSG_DATASAVE_NOTE] = func_SaveNoteData;
}

CDynsDataSaveThreadHandler::~CDynsDataSaveThreadHandler()
{
    // 停止线程并等待结束
    d_ptr->m_bIsExit = true;

    // 清空任务队列
    d_ptr->m_mutex.lock();
    while (!d_ptr->m_queSaveEvent.empty())
    {
        d_ptr->m_queSaveEvent.pop();
    }
    d_ptr->m_mutex.unlock();

    // 如果有正在运行的线程，唤醒它以退出
    if (m_pThread)
    {
        m_pThread->WakeUp(1);
        m_pThread->wait();
    }

    // 释放数据保存对象
    d_ptr->m_pDataSave.reset();
}

void CDynsDataSaveThreadHandler::SaveAllData()
{
    if (!d_ptr->m_pDataSave)
    {
        return;
    }

    // 处理所有待保存的数据
    d_ptr->m_mutex.lock();
    while (!d_ptr->m_queSaveEvent.empty())
    {
        DataSaveEvent event = d_ptr->m_queSaveEvent.front();
        d_ptr->m_queSaveEvent.pop();

        auto it = d_ptr->m_mapFunc.find(event.strMsgKey);
        if (it != d_ptr->m_mapFunc.end())
        {
            it->second(d_ptr->m_pDataSave, event);
        }
    }
    d_ptr->m_mutex.unlock();
}

void CDynsDataSaveThreadHandler::HandleTask()
{
    if (!d_ptr->m_pDataSave)
    {
        return;
    }

    while (true)
    {
        d_ptr->m_mutex.lock();
        if (d_ptr->m_queSaveEvent.empty())
        {
            d_ptr->m_mutex.unlock();
            return;
        }
        DataSaveEvent event = d_ptr->m_queSaveEvent.front();
        d_ptr->m_queSaveEvent.pop();
        d_ptr->m_mutex.unlock();

        auto it = d_ptr->m_mapFunc.find(event.strMsgKey);
        if (it != d_ptr->m_mapFunc.end())
        {
            it->second(d_ptr->m_pDataSave, event);
            sleep(1);
        }
    }
}

void CDynsDataSaveThreadHandler::AddTask(const string &strKey, const QVariantHash &params)
{
    static constexpr INT32 s32_MaxLimit = 1000;
    DataSaveEvent tmpEvent;
    if (!strKey.empty())
    {
        tmpEvent.strMsgKey = strKey;
        if (params.size() > 0)
        {
            tmpEvent.params = params;
        }
        d_ptr->m_mutex.lock();
        if (d_ptr->m_queSaveEvent.size() < s32_MaxLimit)
        {
            d_ptr->m_queSaveEvent.push(tmpEvent);
        }
        d_ptr->m_mutex.unlock();
    }

    if (m_pThread)
    {
        m_pThread->WakeUp(s32_MaxLimit);
    }
}

