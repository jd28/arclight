find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(CreatureView STATIC
    creatureabilitiesselector.cpp
    creatureabilitiesselector.h
    creatureabilitiesselector.ui
    creatureappearanceview.cpp
    creatureappearanceview.h
    creatureappearanceview.ui
    creaturecharsheetview.cpp
    creaturecharsheetview.h
    creaturecharsheetview.ui
    creatureequipview.cpp
    creatureequipview.h
    creatureequipview.ui
    creaturefeatselector.cpp
    creaturefeatselector.h
    creaturefeatselector.ui
    creatureinventorypanel.cpp
    creatureinventorypanel.h
    creatureinventorypanel.ui
    creaturepartsview.cpp
    creaturepartsview.h
    creaturepropertiestab.cpp
    creaturepropertiestab.h
    creaturepropertiestab.ui
    creaturepropertiesview.cpp
    creaturepropertiesview.h
    creaturespellselector.cpp
    creaturespellselector.h
    creaturespellselector.ui
    creaturestatsview.cpp
    creaturestatsview.h
    creaturestatsview.ui
    creatureview.cpp
    creatureview.h
    creatureview.ui

    ../../resources.qrc
)

target_compile_definitions(CreatureView PUBLIC -DGLM_ENABLE_EXPERIMENTAL)

target_include_directories(CreatureView PRIVATE
    ./
)

target_include_directories(CreatureView SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/xxhash-0.8.3
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(CreatureView PRIVATE
    arclight-widgets
    arclight-external
    nw
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    nw
    renderer
    ColorSelectorDialog
    InventoryView
    VariableTableView
)
