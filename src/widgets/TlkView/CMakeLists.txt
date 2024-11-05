find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(TlkView STATIC
    TlkModel.cpp
    TlkModel.hpp
    TlkView.cpp
    TlkView.hpp
    TlkView.ui
)

target_include_directories(TlkView SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(TlkView PRIVATE
    arclight-widgets
    arclight-external
    nw
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)
