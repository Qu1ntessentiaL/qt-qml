cmake_minimum_required(VERSION 3.16)

project(qt-qml VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Quick SerialPort)
qt_standard_project_setup(REQUIRES 6.9)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(FTD2XX_ROOT "${CMAKE_SOURCE_DIR}/libs/ftd2xx" CACHE PATH "Path to the FTD2XX SDK")
set(FT4222_ROOT "${CMAKE_SOURCE_DIR}/libs/LibFT4222" CACHE PATH "Path to the LibFT4222 SDK")

add_library(FTD2XX::FTD2XX SHARED IMPORTED)
set_target_properties(FTD2XX::FTD2XX PROPERTIES
        IMPORTED_IMPLIB "${FTD2XX_ROOT}/amd64/ftd2xx.lib"
        IMPORTED_LOCATION "${FTD2XX_ROOT}/amd64/ftd2xx64.dll"
        INTERFACE_INCLUDE_DIRECTORIES "${FTD2XX_ROOT}"
)

add_library(LibFT4222::LibFT4222 SHARED IMPORTED)
set_target_properties(LibFT4222::LibFT4222 PROPERTIES
        IMPORTED_IMPLIB "${FT4222_ROOT}/dll/amd64/LibFT4222-64.lib"
        IMPORTED_LOCATION "${FT4222_ROOT}/dll/amd64/LibFT4222-64.dll"
        INTERFACE_INCLUDE_DIRECTORIES "${FT4222_ROOT}/inc"
)

qt_add_executable(appqt-qml
        src/main.cpp
        src/SerialManager.cpp
        src/ft4222/ft4222.cpp
        src/ft4222/ft4222_wrapper.cpp
)

target_include_directories(appqt-qml PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/ft4222
)

target_link_libraries(appqt-qml PRIVATE
        Qt6::Quick
        Qt6::SerialPort
        FTD2XX::FTD2XX
        LibFT4222::LibFT4222
)

qt_add_qml_module(appqt-qml
        URI qt-qml
        VERSION 1.0
        QML_FILES
        qml/Main.qml
)

set_target_properties(appqt-qml PROPERTIES
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE FALSE
)

include(GNUInstallDirs)
install(TARGETS appqt-qml
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

find_program(QT_DEPLOY_TOOL windeployqt
        HINTS
        ENV QTDIR
        "$ENV{Qt6_DIR}/../bin"
        "$ENV{Qt6_ROOT}/bin"
        "${Qt6_DIR}/../bin"
)

if (QT_DEPLOY_TOOL)
    add_custom_command(TARGET appqt-qml POST_BUILD
            COMMAND "${QT_DEPLOY_TOOL}" --no-translations --qmldir "${CMAKE_SOURCE_DIR}/qml"
            $<TARGET_FILE:appqt-qml>
            COMMENT "Running windeployqt..."
    )
else ()
    message(WARNING "windeployqt not found. Deployment step will be skipped.")
endif ()

add_custom_command(TARGET appqt-qml POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FTD2XX_ROOT}/amd64/ftd2xx64.dll"
        $<TARGET_FILE_DIR:appqt-qml>
)

add_custom_command(TARGET appqt-qml POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FT4222_ROOT}/dll/amd64/LibFT4222-64.dll"
        $<TARGET_FILE_DIR:appqt-qml>
)