# Widgets/content/CMakeLists.txt
# 定义 content QML 模块（静态库，不生成插件）
qt6_add_qml_module(content
        URI content
        VERSION 1.0
        # RESOURCE_PREFIX 不用写了，QTP0001 NEW 默认为 /qt/qml
        NO_PLUGIN
        STATIC

        QML_FILES
            App.qml
            MainWindow.qml
            PopInputNoteDia.qml
            Screen01.ui.qml

        RESOURCES
            fonts/fonts.txt
)

# 链接 Qt 必要模块（QML/Quick 常用）
target_link_libraries(content
        PRIVATE
        Qt6::Core
        Qt6::Qml
        Qt6::Quick
)
