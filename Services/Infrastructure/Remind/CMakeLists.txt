
add_library(NoteTip SHARED
            CNoteTip.cpp
            CNoteTip.h
            MiniaudioImpl.cpp
)

# miniaudio.h 只在 .cpp 内部使用，头文件已用前向声明隔离，故用 PRIVATE
target_include_directories(NoteTip
        PRIVATE
        ${CMAKE_SOURCE_DIR}/ExternalCode/miniaudio
)

target_include_directories(NoteTip PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/Remind>
)

target_link_libraries(NoteTip
        PRIVATE
            Interfaces
)

# miniaudio 各平台后端所需的系统库
if(WIN32)
    # WASAPI 后端
    target_link_libraries(NoteTip PRIVATE ole32)
elseif(APPLE)
    # CoreAudio 后端
    target_link_libraries(NoteTip PRIVATE
            "-framework CoreFoundation"
            "-framework CoreAudio")
else()
    # ALSA / PulseAudio / JACK 后端通过 dlopen 加载，需要 dl 与 pthread
    target_link_libraries(NoteTip PRIVATE pthread dl m)
endif()