# =============================================================================
# Base Services Module
# =============================================================================
# 提供基础数据类型和服务接口

# 创建接口库，只包含头文件，不需要编译
add_library(BaseServices INTERFACE)

# 设置库的属性
set_target_properties(BaseServices PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
)

# 添加当前目录到包含路径
target_include_directories(BaseServices INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/BaseServices>
)

# 导出头文件
target_sources(BaseServices INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/datatype.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DServiceBase.h
)

# 设置导出目标
install(TARGETS BaseServices
    EXPORT BaseServicesTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES 
    datatype.h
    DServiceBase.h
    DESTINATION include/BaseServices
)

