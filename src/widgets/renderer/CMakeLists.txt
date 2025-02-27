find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(renderer STATIC
    areamodelview.cpp
    areamodelview.h
    basicmodelview.cpp
    basicmodelview.h
    renderwidget.cpp
    renderwidget.h
)

target_compile_definitions(renderer PUBLIC -DGLM_ENABLE_EXPERIMENTAL)

target_include_directories(renderer PRIVATE
    ./
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/xxhash-0.8.3
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(renderer PUBLIC
    arclight-widgets
    arclight-external
    renderer-service
    nw

    Qt6::Widgets
)

