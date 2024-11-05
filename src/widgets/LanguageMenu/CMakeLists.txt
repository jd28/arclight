add_library(LanguageMenu STATIC
    LanguageMenu.cpp
    LanguageMenu.h
)

target_include_directories(LanguageMenu SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(LanguageMenu PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)
