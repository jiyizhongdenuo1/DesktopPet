add_library(ConfigManager STATIC
    CConfigManager.h
    CConfigManager.cpp
)

target_include_directories(ConfigManager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../Domain
)

target_link_libraries(ConfigManager PUBLIC
    CoreTypes
    BaseServices
    PRIVATE
    Interfaces
    Config
)

add_subdirectory(System)
add_subdirectory(DataManager)