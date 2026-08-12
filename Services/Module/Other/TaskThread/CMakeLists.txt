
add_library(TaskThread SHARED
        CLogThreadHandler.cpp
        CLogThreadHandler.h
        CSystemThreadHandler.cpp
        CSystemThreadHandler.h
        CDynsDataSaveThreadHandler.cpp
        CDynsDataSaveThreadHandler.h
        DThread.h
)

target_include_directories(TaskThread PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/TaskThread>
)
target_link_libraries(TaskThread
        PRIVATE
        BaseServices
        DataReadWriterModule
        DataCache
        DataManager
)