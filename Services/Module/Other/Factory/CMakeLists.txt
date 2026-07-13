
add_library(Factory SHARED
        CThreadFactory.cpp
        CThreadFactory.h
        IFactory.h)

target_link_libraries(Factory
        PRIVATE
        TaskThread
        BaseServices
        TaskThread)

target_include_directories(Factory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
