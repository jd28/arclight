find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS OpenGL OpenGLWidgets)

add_library(CreatureView STATIC
    creaturefeatselector.cpp
    creaturefeatselector.h
    creaturefeatselector.ui
    creaturefeatselectormodel.h
    creaturefeatselectormodel.cpp
    creaturefeatselectormodel.h
    creatureview.cpp
    creatureview.h
    creatureview.ui
    creaturestatsview.h
    creaturestatsview.cpp
    creaturestatsview.ui
    creatureappearanceview.h
    creatureappearanceview.cpp
    creatureappearanceview.ui
    creatureequipview.h
    creatureequipview.cpp
    creatureequipview.ui
    inventoryslot.h
    inventoryslot.cpp

    resources.qrc
)

target_compile_definitions(CreatureView PUBLIC -DGLM_ENABLE_EXPERIMENTAL)

target_include_directories(CreatureView PRIVATE
    ./
    ../
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
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
    VariableTableView
)
