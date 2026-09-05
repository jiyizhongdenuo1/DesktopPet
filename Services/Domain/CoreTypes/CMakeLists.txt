# =============================================================================
# Core Types Library
# =============================================================================
# 基础数据类型定义

add_library(CoreTypes STATIC
        datatype.h
        Export.h
        Business_G.h
        CommonDefine.h
)

target_include_directories(CoreTypes PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)