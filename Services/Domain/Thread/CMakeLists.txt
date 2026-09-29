# =============================================================================
# Thread Domain Module
# =============================================================================
# 提供线程基础类（CThread）

add_library(ThreadBase SHARED
    CThread.cpp
    CThread.h
)

target_compile_definitions(ThreadBase
    PRIVATE
        THREAD_BASE_LIBRARY
)

target_include_directories(ThreadBase PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/ThreadBase>
)

target_link_libraries(ThreadBase
    PRIVATE
        CoreTypes
        Interfaces
)