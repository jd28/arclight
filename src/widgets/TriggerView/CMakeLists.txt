find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(TriggerView STATIC
    triggerview.h
    triggerview.cpp
    triggerview.ui

    ../../resources.qrc
    triggergeneralview.h triggergeneralview.cpp triggergeneralview.ui
)

target_compile_definitions(TriggerView PUBLIC -DGLM_ENABLE_EXPERIMENTAL)

target_include_directories(TriggerView PRIVATE
    ./
)

target_include_directories(TriggerView SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/xxhash-0.8.3
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(TriggerView PRIVATE
    arclight-widgets
    nw
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    renderer
    VariableTableView
)
