# Enable policy to run automoc on generated files.
if(POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_subdirectory(AreaView)
add_subdirectory(ColorSelectorDialog)
add_subdirectory(ContainerView)
add_subdirectory(CreatureView)
add_subdirectory(DialogView)
add_subdirectory(DoorView)
add_subdirectory(LanguageMenu)
add_subdirectory(PlaceableView)
add_subdirectory(renderer)
add_subdirectory(TlkView)
add_subdirectory(VariableTableView)
add_subdirectory(QtWaitingSpinner)

add_library(arclight-widgets STATIC
    AbstractTreeModel.cpp
    AbstractTreeModel.hpp

    ArclightView.h
    ArcLightView.cpp

    TlkSelector/tlkselector.h
    TlkSelector/tlkselector.cpp
    TlkSelector/tlkselector.ui

    util/restypeicons.cpp
    util/restypeicons.h
    util/objects.cpp
    util/objects.h
    util/strings.cpp
    util/strings.h

    checkboxdelegate.h
    checkboxdelegate.cpp
    arealistview.h
    arealistview.cpp
    projectview.h
    projectview.cpp
    proxymodels.h
    proxymodels.cpp
    explorerview.h
    explorerview.cpp
    arclighttreeview.h
    arclighttreeview.cpp
    strreflineedit.h
    strreflineedit.cpp
    propertiesview.h
    propertiesview.cpp
    strreftextedit.h
    strreftextedit.cpp
    strreftextedit.ui
    comboboxdelegate.h
    comboboxdelegate.cpp
)

target_include_directories(arclight-widgets SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(arclight-widgets PRIVATE
    nw
    arclight-external
    WaitingSpinnerWidget
    qtpropertybrowser
    Qt6::Widgets
)
