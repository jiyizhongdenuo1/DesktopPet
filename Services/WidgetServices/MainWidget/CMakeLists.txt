# =============================================================================
# Main Widget Module
# =============================================================================
# 主界面组件和视图模型

# 创建静态库
add_library(CMainWidgetServices STATIC CMainNoteListViewModel.cpp
        CMainNoteListViewModel.h)

# 设置库的属性
#set_target_properties(CMainWidgetServices PROPERTIES
#    VERSION 1.0.0
#    SOVERSION 1
#
#    AUTOMOC ON
#    AUTOUIC ON
#    AUTORCC ON
#)

# 添加包含目录
target_include_directories(CMainWidgetServices PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/CMainWidgetServices>
)

# 链接依赖库
target_link_libraries(CMainWidgetServices PUBLIC
    BaseServices
    DataManager
    Qt6::Core
)