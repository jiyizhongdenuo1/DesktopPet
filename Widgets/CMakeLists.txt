
# 把子目录 content 也加进来
add_subdirectory(content)
# 仅当 imports 目录有实际内容时添加
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/imports/CMakeLists.txt" AND IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/imports")
    add_subdirectory(imports)
endif()

qt6_add_qml_module(Widgets
        URI Widgets
        VERSION 1.0
        NO_PLUGIN
        STATIC
        QML_FILES
            MainWidget.qml
            ListOfNotes.qml
)

# 关键：Widgets 依赖 content → QML 里能 import content
target_link_libraries(Widgets
        PUBLIC
        content
        Qt6::Core
        Qt6::Qml
        Qt6::Quick
)
