

add_library(DataProcess SHARED
        CDataSave.cpp
        CDataSave.h
        CDataSaveRWMgr.cpp
        CDataSaveRWMgr.h
        CNoteDataRW.cpp
        CNoteDataRW.h
)
target_link_libraries(DataProcess
        PRIVATE
        BaseServices)

target_include_directories(DataProcess PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
install(FILES
        CDataSave.h
        DESTINATION include/DataProcess
)
install(TARGETS DataProcess
        DESTINATION lib
        EXPORT DataProcessTargets)
