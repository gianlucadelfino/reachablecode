cmake_minimum_required(VERSION 3.24.1)

project(qtHelloWorld)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

qt5_wrap_cpp(MAINWINDOW_MOC include/MainWindow.h)
add_executable(qtHelloWorld src/main.cpp ${MAINWINDOW_MOC})

target_include_directories(qtHelloWorld PUBLIC include)
target_link_libraries(qtHelloWorld ${QT_QTCORE_LIBRARIES} ${QT_QTGUI_LIBRARIES} Qt5::Widgets)
