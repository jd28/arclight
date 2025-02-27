find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_library(toolset-service
    toolsetservice.cpp
    toolsetservice.h
    rulesetmodels.cpp
    rulesetmodels.h

    ${CMAKE_SOURCE_DIR}/src/resources.qrc
)

target_include_directories(toolset-service SYSTEM PRIVATE
    ../
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/xxhash-0.8.3
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
    ${CMAKE_SOURCE_DIR}/src/widgets/
)

target_link_libraries(toolset-service PRIVATE
    nw
    arclight-external
    Qt6::Widgets
)
