add_library(Config STATIC
        CCommonConfig.cpp
        CCommonConfig.h
        CUserConfig.cpp
        CUserConfig.h
)

target_link_libraries(Config
        PRIVATE
        CoreTypes
        Interfaces          # ✅ 4A: 添加Interfaces依赖，继承其头文件路径
        Qt6::Core
)

target_include_directories(Config PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/Config>
)