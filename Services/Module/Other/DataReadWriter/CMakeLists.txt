

add_library(DataReadWriterModule SHARED
        CDataSave.cpp
        CDataSave.h
        CDataSaveRWMgr.cpp
        CDataSaveRWMgr.h
        CNoteDataRW.cpp
        CNoteDataRW.h
        DSaveDefine.h
)
target_link_libraries(DataReadWriterModule
        PRIVATE
        BaseServices
        DataManager
        Config)

target_include_directories(DataReadWriterModule
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)