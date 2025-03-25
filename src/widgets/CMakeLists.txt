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
add_subdirectory(EncounterView)
add_subdirectory(InventoryView)
add_subdirectory(ItemView)
add_subdirectory(LanguageMenu)
add_subdirectory(PlaceableView)
add_subdirectory(SoundView)
add_subdirectory(StoreView)
add_subdirectory(renderer)
add_subdirectory(TlkView)
add_subdirectory(TriggerView)
add_subdirectory(VariableTableView)
add_subdirectory(WaypointView)
add_subdirectory(QtWaitingSpinner)

add_library(arclight-widgets STATIC
    AbstractTreeModel.cpp
    AbstractTreeModel.hpp

    ArclightView.cpp
    ArclightView.h
    arclighttab.cpp
    arclighttab.h

    TlkSelector/tlkselector.h
    TlkSelector/tlkselector.cpp
    TlkSelector/tlkselector.ui

    util/itemmodels.cpp
    util/itemmodels.h
    util/restypeicons.cpp
    util/restypeicons.h
    util/objects.cpp
    util/objects.h
    util/strings.cpp
    util/strings.h
    util/undocommands.cpp
    util/undocommands.h

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


    strreftextedit.h
    strreftextedit.cpp
    strreftextedit.ui
    comboboxdelegate.h
    comboboxdelegate.cpp
    loadscreensview.cpp
    loadscreensview.h
    loadscreensview.ui
    spinboxdelegate.h
    spinboxdelegate.cpp
    textboxdialog.h
    textboxdialog.cpp
    propertybrowser.h
    propertybrowser.cpp
    statictwodamodel.h
    statictwodamodel.cpp
)

target_include_directories(arclight-widgets SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/xxhash-0.8.3
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(arclight-widgets PRIVATE
    nw
    arclight-external
    toolset-service
    WaitingSpinnerWidget
    Qt6::Widgets
)
