# DataManager 模块的 Google Test 配置

# 添加测试可执行文件（链接到全局的 main.cpp）
add_executable(DataManagerTests
        ${CMAKE_CURRENT_SOURCE_DIR}/test_note_data_cache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_note_data_service.cpp
)

# 链接依赖
target_link_libraries(DataManagerTests PRIVATE
        GTest::gtest_main          # 包含 main() 函数
        GTest::gtest               # GTest 库
        DataManager                # 被测模块（自身）
        pthread                    # Linux 线程库
)

# 启用 C++20
target_compile_features(DataManagerTests PRIVATE cxx_std_20)

# 添加到 CTest 自动发现
include(GoogleTest)
gtest_discover_tests(DataManagerTests)

# 设置输出目录
set_target_properties(DataManagerTests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests
)