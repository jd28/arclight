find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(DoorView STATIC
    doorgeneralview.h
    doorgeneralview.cpp
    doorgeneralview.ui
    doorproperties.h
    doorproperties.cpp
    doorview.h
    doorview.cpp
    doorview.ui

    ../../resources.qrc
)

target_compile_definitions(DoorView PUBLIC -DGLM_ENABLE_EXPERIMENTAL)

target_include_directories(DoorView PRIVATE
    ./
)

target_include_directories(DoorView SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/xxhash-0.8.3
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(DoorView PRIVATE
    arclight-widgets
    nw
    renderer
    VariableTableView

    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)
