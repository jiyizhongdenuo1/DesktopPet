add_library(CAppSystem STATIC
            CAppSystem.cpp
            CAppSystem.h
)

target_link_libraries(CAppSystem
        PUBLIC
        BaseServices
)

target_include_directories(CAppSystem PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/System>
)

install(TARGETS CAppSystem
        EXPORT SystemTargets
        ARCHIVE DESTINATION lib
)

install(FILES CAppSystem.h
        DESTINATION include/System)
