
add_library(DataManager SHARED
        CNoteDataService.cpp
        CNoteDataService.h
        CServiceLocator.cpp
        CServiceLocator.h
)

target_link_libraries(DataManager PRIVATE
                     DataReadWriterModule
                     DataCache)
target_link_libraries(DataManager PUBLIC
                     BaseServices)

target_include_directories(DataManager PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/DataManager>
)