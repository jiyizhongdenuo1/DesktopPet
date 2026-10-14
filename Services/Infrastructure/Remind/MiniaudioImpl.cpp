/*
 * @file: MiniaudioImpl.cpp
 * @brief: miniaudio 实现单元
 * @author: nuo
 * @date: 2026/10/13
 * @Detail: 全工程必须且只能有一个编译单元定义 MINIAUDIO_IMPLEMENTATION，
 *          否则会出现重复符号。其他文件只 #include "miniaudio.h" 使用声明即可。
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// windows.h 默认定义 min/max 宏，会和 std::min / std::max 冲突，必须屏蔽
#ifndef NOMINMAX
#define NOMINMAX
#endif

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
