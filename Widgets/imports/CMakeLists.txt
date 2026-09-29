qt6_add_qml_module(imports
        URI components
        VERSION 1.0
        NO_PLUGIN
        STATIC

        QML_FILES
            StyledButton.qml
            LeftWdt.qml
)

target_link_libraries(imports
        PRIVATE
        Qt6::Core
        Qt6::Qml
        Qt6::Quick
)