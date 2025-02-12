find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(ColorSelectorDialog STATIC
    colorselector.cpp
    colorselector.h
    colorselectorview.cpp
    colorselectorview.h
    colorselectorview.ui
    colorselectordialog.h
    colorselectordialog.cpp
    creaturecolorselectordialog.cpp
    creaturecolorselectordialog.h
    creaturecolorselectorview.h
    creaturecolorselectorview.cpp
    creaturecolorselectorview.ui

    ../../resources.qrc
)

target_compile_definitions(ColorSelectorDialog PUBLIC -DGLM_ENABLE_EXPERIMENTAL)

target_include_directories(ColorSelectorDialog PRIVATE
    ./
    ../
)

target_include_directories(ColorSelectorDialog SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(ColorSelectorDialog PRIVATE
    arclight-external
    nw
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)
