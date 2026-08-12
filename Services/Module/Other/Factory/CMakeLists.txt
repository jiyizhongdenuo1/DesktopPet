
add_library(Factory SHARED
        CThreadFactory.cpp
        CThreadFactory.h
        IFactory.h)

target_link_libraries(Factory
        PRIVATE
        TaskThread
        BaseServices)

target_include_directories(Factory PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/Factory>
)