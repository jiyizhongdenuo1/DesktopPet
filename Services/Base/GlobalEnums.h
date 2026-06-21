/*
 * @file: GlobalEnums.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/8
 * @Detail:
 */
#pragma once
using std::shared_ptr;
using std::unique_ptr;
enum E_THREAD_ID
{
    E_THREAD_SAVE_DATA = 0,
    E_THREAD_LOG,
    E_THREAD_SYSTEM,

    E_THREAD_MAX
};
