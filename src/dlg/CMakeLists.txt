find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)

set(SRC_FILES
    main.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    fontchooserdialog.h
    fontchooserdialog.cpp
    fontchooserdialog.ui

    ${CMAKE_SOURCE_DIR}/external/ZFontIcon/ZFontIcon/Fonts.qrc
)

if(APPLE)
add_executable(dlg MACOSX_BUNDLE
    ${SRC_FILES}
)

set_target_properties(dlg
    PROPERTIES
    BUNDLE True
    MACOSX_BUNDLE_GUI_IDENTIFIER my.domain.style.identifier.dlg
    MACOSX_BUNDLE_BUNDLE_NAME dlg
    MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
)
elseif(MSVC)
add_executable(dlg WIN32
    ${SRC_FILES}
)
else()
add_executable(dlg
    ${SRC_FILES}
)
endif()

target_include_directories(dlg PRIVATE
    ./
    ../
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
    ${CMAKE_SOURCE_DIR}/src/widgets/
)

target_link_libraries(dlg PRIVATE
    arclight-widgets
    nw
    sqlite3
    arclight-external
    DialogView
    Qt6::Widgets
    Qt6::Multimedia
)

if(LINUX)
target_link_libraries(dlg PRIVATE
    dl
)
endif()

if(APPLE)
install(TARGETS dlg RUNTIME DESTINATION bin BUNDLE DESTINATION bin)
else()
install(TARGETS dlg)
endif()
