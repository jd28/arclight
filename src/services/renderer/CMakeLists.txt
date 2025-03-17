add_library(renderer-service
    placeholder_texture.h
    renderservice.cpp
    renderservice.h
    shadermanager.cpp
    shadermanager.h
    TextureCache.cpp
    TextureCache.hpp
    model.cpp
    model.hpp
    renderpipelinestate.cpp
    renderpipelinestate.h

    ${CMAKE_SOURCE_DIR}/src/resources.qrc
)

target_include_directories(renderer-service SYSTEM PRIVATE
    ../
    ${CMAKE_SOURCE_DIR}/external/rollnw/external
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/sqlite-3.45.2
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/xxhash-0.8.3
    ${CMAKE_SOURCE_DIR}/external/rollnw/external/minizip/include
    ${CMAKE_SOURCE_DIR}/external/rollnw/lib
    ${CMAKE_SOURCE_DIR}/external/ZFontIcon
    ${CMAKE_SOURCE_DIR}/external/
    ${CMAKE_SOURCE_DIR}/src/widgets/
)

target_link_libraries(renderer-service PUBLIC
    nw
    arclight-external
    Diligent-GraphicsEngine
    Diligent-Common
)

if(WIN32)
    target_link_libraries(renderer-service PUBLIC
        Diligent-GraphicsEngineD3D12-static
    )
elseif(APPLE)
    target_link_libraries(renderer-service PUBLIC
        Diligent-GraphicsEngineMetal-static
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(renderer-service PUBLIC
        Diligent-GraphicsEngineVk-static
    )
endif()
