find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)

add_library(DialogView STATIC
    dialogview.cpp
    dialogview.h
    dialogview.ui
    dialogtreeview.cpp
    dialogtreeview.h
    dialogmodel.cpp
    dialogmodel.h
)

target_include_directories(DialogView PRIVATE
    ./
)

target_include_directories(DialogView SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)


target_link_libraries(DialogView PRIVATE
    nw
    arclight-external
    arclight-widgets
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
)
