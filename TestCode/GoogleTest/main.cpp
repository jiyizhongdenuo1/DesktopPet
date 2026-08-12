/*
 * @file: main.cpp
 * @brief: Google Test 测试入口
 * @author: nuo
 * @date: 2026/7/25
 */

#include <gtest/gtest.h>

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}