/*
 * @file: IFactory.h
 * @brief: 
 * @author: nuo
 * @date: 2026/6/8
 * @Detail:
 */

#pragma once

#include <string>

template <typename T>
class IFactory
{
public:
    explicit IFactory();

    ~IFactory();

    virtual T* Create() = 0;
};
