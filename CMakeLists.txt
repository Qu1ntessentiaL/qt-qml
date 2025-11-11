cmake_minimum_required(VERSION 3.16)

project(qt-qml VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.9.2/msvc2022_64")

find_package(Qt6 6.9 REQUIRED COMPONENTS Quick SerialPort)

qt_standard_project_setup(REQUIRES 6.9)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

qt_add_executable(appqt-qml
        src/main.cpp
        src/SerialManager.cpp
        src/ft4222/ft4222.cpp
        src/ft4222/ft4222_wrapper.cpp
        src/si7210/si7210.cpp
)

qt_add_qml_module(appqt-qml
        URI qt-qml
        VERSION 1.0
        QML_FILES
        qml/Main.qml
)

include_directories(
        ${CMAKE_SOURCE_DIR}/src/
        ${CMAKE_SOURCE_DIR}/src/ft4222
        ${CMAKE_SOURCE_DIR}/src/si7210
        ${CMAKE_SOURCE_DIR}/libs/ftd2xx/
        ${CMAKE_SOURCE_DIR}/libs/LibFT4222/inc/
)

link_directories(
        ${CMAKE_SOURCE_DIR}/libs/ftd2xx/amd64/
        ${CMAKE_SOURCE_DIR}/libs/LibFT4222/dll/amd64/
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appqt-qml PROPERTIES
        #    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appqt-qml
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(appqt-qml PRIVATE
        Qt6::Quick
        Qt6::SerialPort
        ${CMAKE_SOURCE_DIR}/libs/ftd2xx/amd64/ftd2xx.lib
        ${CMAKE_SOURCE_DIR}/libs/LibFT4222/dll/amd64/LibFT4222-64.lib
)

include(GNUInstallDirs)
install(TARGETS appqt-qml
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set(QT_DEPLOY_TOOL "C:/Qt/6.9.2/msvc2022_64/bin/windeployqt.exe")

add_custom_command(TARGET appqt-qml POST_BUILD
        COMMAND ${QT_DEPLOY_TOOL} --no-translations --qmldir ${CMAKE_SOURCE_DIR}/qml
        $<TARGET_FILE:appqt-qml>
        COMMENT "Running windeployqt..."
)


add_custom_command(TARGET appqt-qml POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/libs/ftd2xx/amd64/ftd2xx64.dll
        $<TARGET_FILE_DIR:appqt-qml>
)
add_custom_command(TARGET appqt-qml POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/libs/LibFT4222/dll/amd64/LibFT4222-64.dll
        $<TARGET_FILE_DIR:appqt-qml>
)