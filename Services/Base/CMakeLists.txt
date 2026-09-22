# =============================================================================
# Base Services Module
# =============================================================================
# 提供基础服务（全局枚举、回调定义等）

add_library(BaseServices SHARED
        GlobalEnums.h
        DBackFun.h
        DTranslation.h
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
        ThreadBase
)