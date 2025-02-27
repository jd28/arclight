find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(AreaView STATIC
    areaview.cpp
    areaview.h
    areaview.ui
)

target_include_directories(AreaView PRIVATE
    ./
)

target_include_directories(AreaView SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/xxhash-0.8.3
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(AreaView PRIVATE
    arclight-widgets
    arclight-external
    nw
    renderer
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)
