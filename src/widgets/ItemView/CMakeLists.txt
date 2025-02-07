find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS OpenGL OpenGLWidgets)

add_library(ItemView STATIC
    itemgeneralview.cpp
    itemgeneralview.h
    itemgeneralview.ui
    itemproperties.cpp
    itemproperties.h
    itemproperties.ui
    itemsimplemodelselector.cpp 
    itemsimplemodelselector.h 
    itemsimplemodelselector.ui
    itemsimplemodelselectordialog.cpp
    itemsimplemodelselectordialog.h
    itemsimplemodelselectordialog.ui
    itemview.cpp
    itemview.h
    itemview.ui
)

target_compile_definitions(ItemView PUBLIC -DGLM_ENABLE_EXPERIMENTAL)

target_include_directories(ItemView PRIVATE
    ./
    ../
)

target_include_directories(ItemView SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
)

target_link_libraries(ItemView PRIVATE
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
