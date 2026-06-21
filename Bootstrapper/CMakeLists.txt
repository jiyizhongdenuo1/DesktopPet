add_library(Bootstrapper STATIC
            CAppBootstrapper.cpp
            CAppBootstrapper.h
)

target_link_libraries(Bootstrapper
        PRIVATE
        CAppSystem
        CMainWidgetServices
        Factory
)

target_include_directories(Bootstrapper PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/Bootstrapper>
)

install(TARGETS Bootstrapper
        EXPORT BootstrapperTargets
        ARCHIVE DESTINATION lib
)

install(FILES CAppBootstrapper.h
        DESTINATION include/Bootstrapper)
