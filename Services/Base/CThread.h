/*
 * @file: CThread.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/1
 * @Detail:
 */

#pragma once

#include <QThread>

#include "datatype.h"
#include "Business_G.h"

class CThreadHandler;
class CThreadPrivate;

class BUSINESS_EXPORT CThread: public QThread
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CThread)
public:
    explicit CThread(const QString &strName, std::shared_ptr<CThreadHandler> pHandle, INT32 s32Interval = 1000);
    ~CThread();

    /**
 * @brief
 * @param  n 唤醒间隔
 * @return
 * @note
 */
    void WakeUp(INT32 n = 1);

    /**
 * @brief
 * @param  s32Interval 间隔时间
 * @return
 * @note
 */
    void SetInterval(INT32 s32Interval);

protected:
    virtual void run();
private:
    void ExitThread();
private:
    QScopedPointer<CThreadPrivate> d_ptr;
};