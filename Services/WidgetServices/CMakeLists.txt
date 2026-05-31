# =============================================================================
# Widget Services Module
# =============================================================================
# 提供界面相关的服务和组件

# 添加子目录
add_subdirectory(MainWidget)

# 创建 WidgetServices 库，聚合所有 Widget 相关服务
add_library(WidgetServices INTERFACE)

# 设置库的属性
set_target_properties(WidgetServices PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
)

# 添加当前目录到包含路径
target_include_directories(WidgetServices INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# 链接依赖的库
target_link_libraries(WidgetServices INTERFACE
    BaseServices
    CMainWidgetServices
)

# 导出配置
install(TARGETS WidgetServices
    EXPORT WidgetServicesTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

