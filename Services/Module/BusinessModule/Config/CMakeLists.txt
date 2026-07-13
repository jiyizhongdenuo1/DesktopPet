add_library(Config STATIC
        CCommonConfig.cpp
        CCommonConfig.h
        DConfig.h
)

target_link_libraries(Config
        PRIVATE
        BaseServices
)

target_include_directories(Config PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

#target_include_directories(Config PUBLIC
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#        $<INSTALL_INTERFACE:include/Config>
#)