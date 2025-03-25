find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(PlaceableView STATIC
    placeablegeneralview.h
    placeablegeneralview.cpp
    placeablegeneralview.ui
    placeableproperties.h
    placeableproperties.cpp
    placeableview.h
    placeableview.cpp
    placeableview.ui
)

target_compile_definitions(PlaceableView PUBLIC -DGLM_ENABLE_EXPERIMENTAL)

target_include_directories(PlaceableView PRIVATE
    ./
)

target_include_directories(PlaceableView SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/xxhash-0.8.3
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(PlaceableView PRIVATE
    arclight-widgets
    arclight-external
    nw
    renderer
    VariableTableView

    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)
