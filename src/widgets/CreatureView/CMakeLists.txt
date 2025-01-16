find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS OpenGL OpenGLWidgets)

add_library(CreatureView STATIC
    creatureappearanceview.cpp
    creatureappearanceview.h
    creatureappearanceview.ui
    creatureequipview.cpp
    creatureequipview.h
    creatureequipview.ui
    creaturefeatselector.cpp
    creaturefeatselector.h
    creaturefeatselector.ui
    creaturefeatselectormodel.cpp
    creaturefeatselectormodel.h
    creaturefeatselectormodel.h
    creatureinventorypanel.cpp
    creatureinventorypanel.h
    creatureinventorypanel.ui
    creatureinventoryview.cpp
    creatureinventoryview.h
    creatureinventoryview.ui
    creaturepartsview.h
    creaturepartsview.cpp
    creaturepropertiesview.cpp
    creaturepropertiesview.h
    creaturestatsview.cpp
    creaturestatsview.h
    creaturestatsview.ui
    creatureview.cpp
    creatureview.h
    creatureview.ui
    inventoryslot.cpp
    inventoryslot.h

    resources.qrc
)

target_compile_definitions(CreatureView PUBLIC -DGLM_ENABLE_EXPERIMENTAL)

target_include_directories(CreatureView PRIVATE
    ./
    ../
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
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    nw
    renderer
    ColorSelectorDialog
    VariableTableView
)
