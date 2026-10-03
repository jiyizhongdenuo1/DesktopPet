qt6_add_qml_module(imports
        URI components
        VERSION 1.0
        NO_PLUGIN
        STATIC
        OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/components

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