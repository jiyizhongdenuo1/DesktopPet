
add_library(TaskThread SHARED
        CLogThreadHandler.cpp
        CLogThreadHandler.h
        CSystemThreadHandler.cpp
        CSystemThreadHandler.h
        CDynsDataSaveThreadHandler.cpp
        CDynsDataSaveThreadHandler.h
)

target_include_directories(TaskThread PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(TaskThread
        PUBLIC
        BaseServices
        DataProcess
)

install(TARGETS TaskThread
        DESTINATION lib
        EXPORT TaskThreadTargets
)

install(FILES
        CLogThreadHandler.h
        CSystemThreadHandler.h
        CDynsDataSaveThreadHandler.h
        DESTINATION include/TaskThread
)
