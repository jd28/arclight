find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(StoreView STATIC
    storeview.h
    storeview.cpp
    storeview.ui
    storeinventoryview.cpp
    storeinventoryview.h
    storeinventoryview.ui
    storegeneralview.cpp
    storegeneralview.h
    storegeneralview.ui

    ../../resources.qrc
)

target_compile_definitions(StoreView PUBLIC -DGLM_ENABLE_EXPERIMENTAL)

target_include_directories(StoreView PRIVATE
    ./
)

target_include_directories(StoreView SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/xxhash-0.8.3
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(StoreView PRIVATE
    arclight-widgets
    nw
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    InventoryView
    VariableTableView
)
