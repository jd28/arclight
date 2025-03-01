find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

add_library(SoundView STATIC
    soundgeneralview.cpp
    soundgeneralview.h
    soundgeneralview.ui
    soundview.h
    soundview.cpp
    soundview.ui

    ../../resources.qrc
)

target_compile_definitions(SoundView PUBLIC -DGLM_ENABLE_EXPERIMENTAL)

target_include_directories(SoundView PRIVATE
    ./
)

target_include_directories(SoundView SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/xxhash-0.8.3
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(SoundView PRIVATE
    arclight-widgets
    nw
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
    VariableTableView
)
