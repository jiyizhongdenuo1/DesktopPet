# =============================================================================
# Note Domain Library
# =============================================================================
# 便签领域模型定义

add_library(NoteDomain STATIC
        DServiceBase.h
        DDataMgrBase.h
)

target_include_directories(NoteDomain PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(NoteDomain
        PRIVATE
        CoreTypes
)