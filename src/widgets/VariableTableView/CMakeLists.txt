find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(VariableTableView STATIC
    variabletableview.cpp
    variabletableview.h
    variabletableview.ui
    vartabledialog.h vartabledialog.cpp vartabledialog.ui
)

target_include_directories(VariableTableView PRIVATE
    ./
    ../
)

target_include_directories(VariableTableView SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(VariableTableView PRIVATE
    arclight-widgets
    arclight-external
    nw
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)
