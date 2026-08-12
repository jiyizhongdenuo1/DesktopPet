add_library(DataCache SHARED
        CNoteDataCollect.cpp
        CNoteDataCollect.h
        CNoteDataCache.cpp
        CNoteDataCache.h
        DDataCache.h
)

target_link_libraries(DataCache PRIVATE
                     BaseServices)

target_include_directories(DataCache PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/DataCache>
)