/*
* @file: CDynsDataSaveThreadHandler.h
 * @brief: 动态数据保存线程处理器
 * @author: nuo
 * @date: 2026/6/10
 * @Detail: 负责数据保存任务的处理和调度
 */

#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "CThreadHandler.h"
#include "datatype.h"

typedef struct st_DataSaveEvent
{
    std::string strMsgKey;
    std::unordered_map<std::string, std::string> mapParams;
} ST_DATA_SAVE_EVENT;

class CDataRWMgr;
class CDynsDataSaveThreadHandlerPrivate;

class CDynsDataSaveThreadHandler : public CThreadHandler
{
public:

    explicit CDynsDataSaveThreadHandler();
    ~CDynsDataSaveThreadHandler();

    /** ***********************************************************
     * @brief       处理任务队列中的任务（线程循环调用）
     * @detail      从队列中逐个取出任务，根据 strMsgKey 查找对应的
     *              处理函数并执行，处理间隔 1 秒
     * @note        队列为空时自动返回
     ************************************************************/
    void HandleTask() override;

    /** ***********************************************************
     * @brief       添加任务到处理队列
     * @param[in]   strKey     任务键名（用于匹配处理函数）
     * @param[in]   mapParams  任务参数（键值对）
     * @note        队列上限 1000，超出时丢弃新任务；添加后唤醒工作线程
     ************************************************************/
    void AddTask(const std::string &strKey, const std::unordered_map<std::string, std::string> &mapParams);

    /** ***********************************************************
     * @brief       保存所有待处理数据（阻塞式）
     * @detail      一次性处理队列中的所有待保存任务，通常用于退出前
     *              或定期全量保存
     * @note        调用前需确保 m_pDataSaveRWMgr 有效
     ************************************************************/
    void SaveAllData();

private:
    std::unique_ptr<CDynsDataSaveThreadHandlerPrivate> d_ptr;
};

typedef void (*DATASAVE_FUNC)(std::shared_ptr<CDataRWMgr> pDataRWMgr, ST_DATA_SAVE_EVENT &event);