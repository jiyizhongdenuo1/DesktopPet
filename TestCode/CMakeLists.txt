# ============================================================================
# DesktopPet 单元测试 - 独立 CMake 项目
# ============================================================================
#
# 📌 核心原则：
#   本项目与 DesktopPet 主程序 100% 隔离，是独立的 CMake 构建项目。
#   - ❌ 不修改任何生产代码或配置文件
#   - ✅ 可独立编译、运行、部署
#   - ✅ 符合编码规范规则 #5（测试代码隔离）
#
# 🚀 使用方法：
#   mkdir build-test && cd build-test
#   cmake ../TestCode [-DDESKTOPPET_SOURCE_DIR=../..]
#   cmake --build . -j$(nproc)
#   ctest --output-on-failure -V
#
# 📖 完整文档：请参阅 TestCode/README.md
# ============================================================================

cmake_minimum_required(VERSION 3.28.3)

project(DesktopPetTests
    VERSION 1.0.0
    DESCRIPTION "DesktopPet Unit Tests (Independent Project)"
    LANGUAGES CXX
)

# ============================================================================
# C++ 标准设置
# ============================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 输出目录配置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================================
# 主项目源码路径检测
# ============================================================================

if(NOT DEFINED DESKTOPPET_SOURCE_DIR)
    # 默认假设 TestCode/ 在主项目根目录下
    set(DESKTOPPET_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
endif()

# 验证路径有效性
if(NOT EXISTS "${DESKTOPPET_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "[ERROR] 无法找到主项目 CMakeLists.txt\n"
        "        指定路径: ${DESKTOPPET_SOURCE_DIR}\n"
        "        请使用 -DDESKTOPPET_SOURCE_DIR=<path> 指定正确路径\n"
        "        示例: cmake .. -DDESKTOPPET_SOURCE_DIR=/home/user/DesktopPet"
    )
endif()

message(STATUS "====================================")
message(STATUS "🧪 DesktopPet 测试项目")
message(STATUS "====================================")
message(STATUS "主项目源码: ${DESKTOPPET_SOURCE_DIR}")
message(STATUS "测试输出目录: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "")

# ============================================================================
# 依赖项查找
# ============================================================================

# Qt6 基础组件（被测模块依赖）
find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 Qml)

# 线程库（GTest 和多线程测试需要）
find_package(Threads REQUIRED)

# ============================================================================
# 全局构建选项（与主项目保持一致）
# ============================================================================

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ============================================================================
# 构建被测模块（从主项目源码按需引入）
# ============================================================================
# 注意：这里只构建测试所需的模块，不构建主程序 UI 或其他无关部分

message(STATUS "📦 构建被测模块...")

# 基础服务层（公共类型定义、线程基类等）
add_subdirectory(
    "${DESKTOPPET_SOURCE_DIR}/Services/Base"
    "${CMAKE_BINARY_DIR}/BaseServices"
    EXCLUDE_FROM_ALL
)
message(STATUS "   ✅ BaseServices")

# 数据读写模块（文件 I/O、数据序列化等）
add_subdirectory(
    "${DESKTOPPET_SOURCE_DIR}/Services/Module/Other/DataReadWriter"
    "${CMAKE_BINARY_DIR}/DataReadWriterModule"
    EXCLUDE_FROM_ALL
)
message(STATUS "   ✅ DataReadWriterModule")

# 数据管理适配器层（业务逻辑编排、缓存机制等）+ 其内部测试子目录
add_subdirectory(
    "${DESKTOPPET_SOURCE_DIR}/Services/Module/Adapter/DataManager"
    "${CMAKE_BINARY_DIR}/DataManager"
    EXCLUDE_FROM_ALL
)
message(STATUS "   ✅ DataManager (含单元测试)")

# 可选：业务逻辑模块（如未来测试需要）
# add_subdirectory(
#     "${DESKTOPPET_SOURCE_DIR}/Services/Module/BusinessModule"
#     "${CMAKE_BINARY_DIR}/BusinessModule"
#     EXCLUDE_FROM_ALL
# )

message(STATUS "")

# ============================================================================
# Google Test 框架集成
# ============================================================================

message(STATUS "🔬 配置 Google Test...")

# 强制静态链接 GTest（避免动态库路径问题）
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# 使用本地 Google Test v1.18.0 源码（已包含在 TestCode/ 目录中）
add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/googletest-main"
    "${CMAKE_BINARY_DIR}/googletest"
    EXCLUDE_FROM_ALL
)

# 整理 IDE 视图：将 GTest 目标归入 ThirdParty 文件夹
set_target_properties(gtest PROPERTIES FOLDER "ThirdParty/GTest")
set_target_properties(gtest_main PROPERTIES FOLDER "ThirdParty/GTest")
set_target_properties(gmock PROPERTIES FOLDER "ThirdParty/GTest")
set_target_properties(gmock_main PROPERTIES FOLDER "ThirdParty/GTest")

message(STATUS "   ✅ Google Test v1.18.0")
message(STATUS "")

# ============================================================================
# 测试发现与执行框架
# ============================================================================

enable_testing()
include(GoogleTest)

# ============================================================================
# 传统 Qt Test 支持（可选，用于向后兼容旧测试套件）
# ============================================================================

option(BUILD_LEGACY_TESTS "Build legacy Qt Test suites (TestDataMgr)" OFF)

if(BUILD_LEGACY_TESTS)
    message(STATUS "📜 启用传统 Qt Test...")
    
    find_package(Qt6 REQUIRED COMPONENTS Test)
    
    add_subdirectory(TestDataMgr)
    
    message(STATUS "   ✅ TestDataMgr 已启用")
    message(STATUS "")
endif()

# ============================================================================
# 代码覆盖率分析（可选，用于 CI/CD 质量门禁）
# ============================================================================

option(ENABLE_COVERAGE "Enable code coverage reporting (requires GCC/Clang)" OFF)

if(ENABLE_COVERAGE)
    if(NOT (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
        message(WARNING 
            "[WARNING] 代码覆盖率分析仅支持 GCC 或 Clang 编译器\n"
            "         当前编译器: ${CMAKE_CXX_COMPILER_ID}\n"
            "         已自动禁用覆盖率分析..."
        )
        set(ENABLE_COVERAGE OFF)
    else()
        message(STATUS "📊 启用代码覆盖率分析...")
        
        # 编译选项：添加覆盖率 instrumentation
        target_compile_options(DataManager PUBLIC
            --coverage
            -fprofile-arcs
            -ftest-coverage
            -O0          # 优化级别 0（确保覆盖数据准确）
            -g           # 生成调试信息
        )
        
        # 链接选项：链接覆盖率运行时库
        target_link_options(DataManager PUBLIC
            --coverage
            -fprofile-arcs
            -ftest-coverage
        )
        
        # 定义排除模式（GTest 自身代码不应计入覆盖率）
        set(COVERAGE_EXCLUDE_PATTERNS
            "*googletest*"
            "*googlemock*"
            "*test_*"
            "*_test.cpp"
            "*_tests.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/*"
            CACHE STRING "Coverage exclusion patterns"
        )
        
        message(STATUS "   ✅ 覆盖率分析已启用（排除第三方代码）")
        message(STATUS "")
    endif()
endif()

# ============================================================================
# 自定义便捷命令目标
# ============================================================================

# 快速运行全部测试
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -V
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS DataManagerTests  # 确保先构建
    COMMENT "🧪 执行所有单元测试..."
    VERBATIM
)

# 过滤运行特定测试（通过环境变量 TEST_FILTER）
add_custom_target(run_tests_filter
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -V -R "$ENV{TEST_FILTER}"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS DataManagerTests
    COMMENT "🔍 运行匹配的测试 (过滤器: $ENV{TEST_FILTER})..."
    VERBATIM
)

# 仅列出所有已注册测试（不实际执行）
add_custom_target(list_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -N
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "📋 列出所有已注册测试..."
    VERBATIM
)

# 清理测试产物（保留构建缓存）
add_custom_target(clean_tests
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests"
    COMMENT "🧹 清理测试输出文件..."
)

# ============================================================================
# 安装规则（可选，用于 CI/CD 或分发）
# ============================================================================

option(INSTALL_TESTS "Install test binaries and results" OFF)

if(INSTALL_TESTS)
    include(GNUInstallDirs)
    
    # 安装测试可执行文件
    install(TARGETS DataManagerTests
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
    )
    
    # 安装测试结果模板（如存在）
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ctest_templates")
        install(DIRECTORY ctest_templates/
            DESTINATION ${CMAKE_INSTALL_DATADIR}/DesktopPet/tests/templates
        )
    endif()
    
    message(STATUS "📦 测试安装已启用")
endif()

# ============================================================================
# 配置总结与帮助信息
# ============================================================================

message(STATUS "====================================")
message(STATUS "✨ 配置完成")
message(STATUS "====================================")
message(STATUS "")
message(STATUS "📌 可用的构建目标:")
message(STATUS "   all              - 构建所有测试")
message(STATUS "   run_tests        - 运行全部测试")
message(STATUS "   run_tests_filter - 过滤运行 (环境变量 TEST_FILTER)")
message(STATUS("   list_tests       - 列出测试列表"))
message(STATUS("   clean_tests      - 清理测试输出"))
message(STATUS "")
message(STATUS "🔧 可选的 CMake 选项:")
message(STATUS("   -DDESKTOPPET_SOURCE_DIR=<path>   主项目源码路径"))
message(STATUS("   -DBUILD_LEGACY_TESTS=ON           启用传统 Qt Test"))
message(STATUS("   -DENABLE_COVERAGE=ON             启用代码覆盖率"))
message(STATUS("   -DINSTALL_TESTS=ON               安装测试二进制文件"))
message(STATUS "")
message(STATUS "💡 使用示例:")
message(STATUS "   # 标准构建与运行")
message(STATUS("   cmake --build . && ctest --output-on-failure -V"))
message(STATUS ""
message(STATUS "   # 只运行 NoteDataCache 相关测试")
message(STATUS("   TEST_FILTER='NoteDataCache*' make run_tests_filter"))
message(STATUS ""
message(STATUS "   # 启用覆盖率分析")
message(STATUS("   cmake .. -DENABLE_COVERAGE=ON && ctest"))
message(STATUS "====================================")
message(STATUS "")