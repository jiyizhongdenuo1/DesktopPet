
add_library(CNoteApp SHARED
        CNoteApp.h
        CNoteApp.cpp
)

target_include_directories(CNoteApp PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}

)

target_link_libraries(CNoteApp
        PRIVATE
        CNoteBusiness
)

