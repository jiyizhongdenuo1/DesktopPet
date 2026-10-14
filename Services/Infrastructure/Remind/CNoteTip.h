/*
 * @file: CNoteTip.h
 * @brief: 便签提醒提示（按四象限等级播放不同提示音）
 * @author: nuo
 * @date: 2026/10/13
 * @Detail:
 */

#pragma once

#include <string>

#include "INoteTipBusiness.h"

// 前向声明，避免把 miniaudio.h 暴露给外部依赖者
struct ma_engine;

class CNoteTip : public INoteTipBusiness
{
public:

    CNoteTip();
    ~CNoteTip() override;

    BOOL StartTip(E_NOTE_EVENT_WAKEUP_LEVEL eLevel) override;

private:
    /** 播放 sounds 目录下的提示音；引擎未就绪或文件缺失时退回系统提示音 */
    BOOL PlaySound(const std::string &strFileName);

    /** 弹出提示窗口（待实现） */
    BOOL ShowPopup();

    /** 拼出 exe 同级 sounds 目录下的绝对路径，失败返回空串 */
    static std::string GetSoundPath(const std::string &strFileName);

private:
    ma_engine *m_pEngine = nullptr;
};
