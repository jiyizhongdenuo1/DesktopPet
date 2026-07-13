
add_library(DataManager SHARED
        CNoteDataCollect.cpp
        CNoteDataCollect.h
        DDataMgrBase.h
        CDataCollectFactory.cpp
        CDataCollectFactory.h
)

target_link_libraries(DataManager PUBLIC
                     BaseServices)

target_include_directories(DataManager PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
)
