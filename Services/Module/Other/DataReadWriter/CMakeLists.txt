

add_library(DataReadWriterModule SHARED
        CDataSave.cpp
        CDataSave.h
        CDataRWMgr.cpp
        CDataRWMgr.h
        DSaveDefine.h
)
target_link_libraries(DataReadWriterModule
        PRIVATE
        BaseServices
        Config)

target_include_directories(DataReadWriterModule
        PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/DataReadWriter>
)