/*
* @file: CDynsDataSaveThreadHandler.h
 * @brief: 动态数据保存线程处理器
 * @author: nuo
 * @date: 2026/6/10
 * @Detail: 负责数据保存任务的处理和调度
 */

#pragma once

#include <QVariantHash>
#include "CThreadHandler.h"
#include "datatype.h"

struct DataSaveEvent
{
    std::string strMsgKey;
    QVariantHash params;
};

class CDataSave;
class CDynsDataSaveThreadHandlerPrivate;

class CDynsDataSaveThreadHandler : public CThreadHandler
{
public:
    explicit CDynsDataSaveThreadHandler();
    ~CDynsDataSaveThreadHandler();

    virtual void HandleTask() override;
    void AddTask(const std::string &strKey, const QVariantHash &params);
    void SaveAllData();

private:
    std::unique_ptr<CDynsDataSaveThreadHandlerPrivate> d_ptr;
};

typedef void (*DATASAVE_FUNC)(std::shared_ptr<CDataSave> pDataSave, DataSaveEvent &event);

namespace DataSaveFucName
{
    constexpr auto MSG_DATASAVE_NOTE = "SaveNoteData";
}