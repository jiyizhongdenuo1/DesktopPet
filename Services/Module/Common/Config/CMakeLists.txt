
add_library(Config SHARED
        CCommonConfig.cpp
        CCommonConfig.h
        DConfig.h)

target_link_libraries(Config
        PRIVATE
        BaseServices
)

target_include_directories(Config PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/Config>
)

install(TARGETS Config
        EXPORT ConfigTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)

install(FILES
        CCommonConfig.h
        DESTINATION include/Config
)
