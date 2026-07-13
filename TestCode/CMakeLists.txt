cmake_minimum_required(VERSION 3.28.3)

# 查找 Qt 测试模块
find_package(Qt6 REQUIRED COMPONENTS Test)

# 添加测试子目录
add_subdirectory(TestDataMgr)