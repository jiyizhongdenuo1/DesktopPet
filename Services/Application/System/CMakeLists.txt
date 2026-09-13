add_library(CAppSystem STATIC
            CAppSystem.cpp
            CAppSystem.h
)

target_link_libraries(CAppSystem
        PRIVATE
        BaseServices
        CoreTypes
        NoteDomain
        Interfaces
        TaskThread
        Factory
        DataCache
        ThreadBase)

target_include_directories(CAppSystem PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/System>
)