# =============================================================================
# Base Services Module
# =============================================================================
# 提供基础线程服务（依赖 CoreTypes 基础类型）

add_library(BaseServices SHARED
        CThread.cpp
        CThread.h
        GlobalEnums.h
)

target_compile_definitions(BaseServices
        PRIVATE
        MODULEBASE_LIBRARY
)

target_include_directories(BaseServices PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/BaseServices>
)

target_link_libraries(BaseServices
        PRIVATE
        CoreTypes
        Interfaces
)