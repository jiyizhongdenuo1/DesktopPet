cmake_minimum_required(VERSION 3.28.3)
project(DesktopPet VERSION 1.0.0 LANGUAGES CXX)
#自定义资源目录的前缀
#if(POLICY QTP0001)
#    cmake_policy(SET QTP0001 NEW)
#endif()

# 项目配置
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 依赖查找
find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 Qml)

#if(POLICY QTP0001)
    qt_policy(SET QTP0001 NEW)
#endif()

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_AUTOUIC ON)

# 子目录
add_subdirectory(Services)
add_subdirectory(Widgets)

# 主可执行文件
qt_add_executable(DesktopPet
        main.cpp
#        ${PROJECT_WIDGETS}
)

# 设置可执行文件属性
set_target_properties(DesktopPet PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
        OUTPUT_NAME ${PROJECT_NAME}
)

# 链接库
target_link_libraries(DesktopPet PRIVATE 
    Qt6::Core 
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Qml

    BaseServices
    WidgetServices
    content
    Widgets
)

# 安装配置
# 强制设置项目内部安装路径
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/CodeExportDIr" CACHE PATH "Project install path" FORCE)
endif()

message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

install(TARGETS DesktopPet
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

# 添加清理目标
add_custom_target(clean-all
        COMMAND rm -rf ${CMAKE_BINARY_DIR}/*
        COMMENT "Cleaning all build files"
)