cmake_minimum_required(VERSION 2.8.8)

PROJECT(CellularAutomaton)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Widgets finds its own dependencies.
find_package(Qt5Widgets REQUIRED)

# process headers with moc
#qt5_wrap_cpp(ca_HEADERS_MOC ${ca_HEADERS})
#QT5_WRAP_UI(ca_FORMS_HEADERS ${ca_FORMS})
QT5_WRAP_UI(UIS_HDRS
    mainwindow.ui
)

add_executable(CellularAutomaton eca.cpp main.cpp mainwindow.cpp ${UIS_HDRS})

qt5_use_modules(CellularAutomaton Widgets)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
