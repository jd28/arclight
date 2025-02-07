find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(InventoryView STATIC
    inventoryview.cpp
    inventoryview.h
    inventoryview.ui
    inventoryslot.cpp
    inventoryslot.h
)

target_compile_definitions(InventoryView PUBLIC -DGLM_ENABLE_EXPERIMENTAL)

target_include_directories(InventoryView PRIVATE
    ./
    ../
)

target_include_directories(InventoryView SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/xxhash-0.8.3
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(InventoryView PRIVATE
    arclight-external
    nw
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)
