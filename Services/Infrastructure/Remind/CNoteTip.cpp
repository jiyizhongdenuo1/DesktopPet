/*
 * @file: CNoteTip.cpp
 * @brief: 便签提醒提示实现
 * @author: nuo
 * @date: 2026/10/13
 * @Detail: 使用 miniaudio 播放提示音，支持 MP3 / WAV
 */

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

#include <cstdio>
#include <filesystem>
#include <string>

#include "CNoteTip.h"
#include "miniaudio.h"

namespace
{
    // 提示音文件名，对应 exe 同级 resource/sounds 目录
    constexpr const char *TIP_SOUND_URGENT  = "urgent.mp3";    ///< 重要且紧急
    constexpr const char *TIP_SOUND_WARNING = "warning.wav";   ///< 紧急
    constexpr const char *TIP_SOUND_NOTICE  = "notice.wav";    ///< 重要

    /** 取当前可执行文件所在目录；失败返回空 path */
    std::filesystem::path GetExeDir()
    {
#ifdef _WIN32
        WCHAR szExePath[MAX_PATH] = {0};
        const DWORD u32Len = GetModuleFileNameW(nullptr, szExePath, MAX_PATH);

        // 返回 0 表示失败；返回 MAX_PATH 表示被截断，此时缓冲区没有结尾 '\0'
        if (u32Len == 0 || u32Len >= MAX_PATH)
        {
            return std::filesystem::path();
        }

        return std::filesystem::path(szExePath).parent_path();
#else
        // Linux：/proc/self/exe 是指向自身可执行文件的符号链接
        std::error_code ec;
        const std::filesystem::path pathExe =
            std::filesystem::read_symlink("/proc/self/exe", ec);

        if (ec)
        {
            return std::filesystem::path();
        }

        return pathExe.parent_path();
#endif
    }
}

CNoteTip::CNoteTip()
    : m_pEngine(new ma_engine)
{
    if (ma_engine_init(nullptr, m_pEngine) != MA_SUCCESS)
    {
        delete m_pEngine;
        m_pEngine = nullptr;
    }
}

CNoteTip::~CNoteTip()
{
    if (m_pEngine)
    {
        ma_engine_uninit(m_pEngine);
        delete m_pEngine;
        m_pEngine = nullptr;
    }
}

std::string CNoteTip::GetSoundPath(const std::string &strFileName)
{
    const std::filesystem::path pathExeDir = GetExeDir();
    if (pathExeDir.empty())
    {
        return std::string();
    }

    const std::filesystem::path pathSoundFile =
        pathExeDir / "resource" / "sounds" / std::filesystem::path(strFileName);

    // miniaudio 统一按 UTF-8 解析路径
    const std::u8string u8strPath = pathSoundFile.u8string();

    return std::string(u8strPath.begin(), u8strPath.end());
}

BOOL CNoteTip::PlaySound(const std::string &strFileName)
{
    if (m_pEngine)
    {
        const std::string strSoundPath = GetSoundPath(strFileName);
        if (!strSoundPath.empty() &&
            ma_engine_play_sound(m_pEngine, strSoundPath.c_str(), nullptr) == MA_SUCCESS)
        {
            return TRUE;
        }
    }

    // 引擎未就绪或音频文件缺失：退回系统提示音，保证"至少响一声"
#ifdef _WIN32
    ::MessageBeep(MB_ICONEXCLAMATION);
#else
    // Linux 没有等价的系统提示音 API，输出终端响铃字符
    std::fputc('\a', stderr);
    std::fflush(stderr);
#endif

    return FALSE;
}

BOOL CNoteTip::StartTip(E_NOTE_EVENT_WAKEUP_LEVEL eLevel)
{
    switch (eLevel)
    {
        case E_NOTE_EVENT_WAKEUP_LEVEL_IMPORTANT_URGENT:
        {
            PlaySound(TIP_SOUND_URGENT);
            ShowPopup();
            return TRUE;
        }
        case E_NOTE_EVENT_WAKEUP_LEVEL_URGENT:
        {
            PlaySound(TIP_SOUND_WARNING);
            ShowPopup();
            return TRUE;
        }
        case E_NOTE_EVENT_WAKEUP_LEVEL_IMPORTANT:
        {
            PlaySound(TIP_SOUND_NOTICE);
            return TRUE;
        }
        case E_NOTE_EVENT_WAKEUP_LEVEL_NORMAL:
        {
            return TRUE;
        }
        default:
        {
            return FALSE;
        }
    }
}

BOOL CNoteTip::ShowPopup()
{
    return FALSE;
}
