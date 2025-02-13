find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_library(arclight-external STATIC
    fzy/match.c
    ZFontIcon/ZFontIcon/ZFontIcon.cpp
)

target_link_libraries(arclight-external PRIVATE
    Qt6::Widgets
)

add_subdirectory(rollnw)
