/*
 * @file: CThreadFactory.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/8
 * @Detail:
 */

#pragma once

#include "IFactory.h"
#include "CThread.h"
#include "GlobalEnums.h"

class CThreadHandler;

/**
 * @brief 线程工厂类
 * @detail 负责创建 CThread 对象，封装线程创建逻辑
 */
class CThreadFactory : public IFactory<CThread>
{
public:
    /**
     * @brief 构造函数
     */
    explicit CThreadFactory();

    /**
     * @brief 析构函数
     */
    ~CThreadFactory();

    /**
     * @brief 创建线程对象（需要手动指定参数）
     * @param strName 线程名称
     * @param pHandle 线程处理器
     * @param s32Interval 唤醒间隔（毫秒）
     * @return 返回创建的线程对象指针
     */
    unique_ptr<CThread> Create(const QString &strName, std::shared_ptr<CThreadHandler> pHandle, INT32 s32Interval = 1000);

    /**
     * @brief 创建线程对象（无参版本，返回空指针）
     * @note 此方法为接口实现，实际应使用带参数的 Create 方法
     * @return 返回 nullptr
     */
    CThread* Create() override;

private:
    // 禁用拷贝
    CThreadFactory(const CThreadFactory&) = delete;
    CThreadFactory& operator=(const CThreadFactory&) = delete;

    void InitThread();
    void InitThreadHanders();
    void ThreadModule();
private:
    std::vector<unique_ptr<CThread>>                   m_vecpThread;
    std::vector<shared_ptr<CThreadHandler>>            m_vecpThreadHanders;

};