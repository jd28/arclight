find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)
find_package(Qt6 REQUIRED COMPONENTS OpenGL OpenGLWidgets)

set(SRC_FILES
    main.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon/ZFontIcon/Fonts.qrc
    ${CMAKE_SOURCE_DIR}/src/widgets/ColorSelectorDialog/resources.qrc
    ${CMAKE_SOURCE_DIR}/src/widgets/CreatureView/resources.qrc
)

if(APPLE)
add_executable(arclight MACOSX_BUNDLE
    ${SRC_FILES}
)

set_target_properties(arclight
    PROPERTIES
    BUNDLE True
    MACOSX_BUNDLE_GUI_IDENTIFIER com.github.jd28.arclight
    MACOSX_BUNDLE_BUNDLE_NAME arclight
    MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
)
elseif(MSVC)
add_executable(arclight WIN32
    ${SRC_FILES}
)
else()
add_executable(arclight
    ${SRC_FILES}
)
endif()

target_include_directories(arclight SYSTEM PRIVATE
    ../
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
    ${CMAKE_SOURCE_DIR}/src/widgets/
)

target_link_libraries(arclight PUBLIC
    arclight-widgets
    nw
    sqlite3
    arclight-external
    qtpropertybrowser
    LanguageMenu
    AreaView
    ColorSelectorDialog
    CreatureView
    DialogView
    DoorView
    PlaceableView
    VariableTableView
    WaitingSpinnerWidget
    Qt6::Widgets
    Qt6::Multimedia
)

if(LINUX)
target_link_libraries(arclight PRIVATE
    dl
)
endif()

if(APPLE)
install(TARGETS arclight RUNTIME DESTINATION bin BUNDLE DESTINATION bin)
else()
install(TARGETS arclight)
endif()
