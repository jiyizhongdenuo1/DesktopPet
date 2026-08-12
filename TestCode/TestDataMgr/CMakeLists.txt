# 添加测试可执行文件
qt_add_executable(TestDataMgr
        CTestDataMgr.cpp
        CTestDataMgr.h
        CTestDataRW.cpp
        CTestDataRW.h
)

# 链接依赖库
target_link_libraries(TestDataMgr PRIVATE
        Qt6::Test
        DataManager          # 数据管理模块（自动传递 BaseServices 和 DataReadWriterModule）
)

# 添加测试
add_test(NAME TestDataMgr
        COMMAND TestDataMgr
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# 设置输出目录
set_target_properties(TestDataMgr PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests
)