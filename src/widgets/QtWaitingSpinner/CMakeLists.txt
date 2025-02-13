cmake_minimum_required(VERSION 3.14)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(WaitingSpinnerWidget STATIC
    waitingspinnerwidget.cpp
    waitingspinnerwidget.h
)

target_include_directories(WaitingSpinnerWidget PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(WaitingSpinnerWidget PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)
