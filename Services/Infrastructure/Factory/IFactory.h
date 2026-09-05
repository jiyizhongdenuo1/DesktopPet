/*
 * @file: IFactory.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/8
 * @Detail:
 */

#pragma once

#include <vector>

template <typename T>
class IFactory
{
public:
    explicit IFactory() = default;

    ~IFactory() = default;
    //virtual T* Create();
};
