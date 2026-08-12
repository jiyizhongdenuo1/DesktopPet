
add_library(CNoteBusiness STATIC
        CNoteBusiness.cpp
        CNoteBusiness.h
)

target_include_directories(CNoteBusiness PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(CNoteBusiness
        PRIVATE
        BaseServices
)
