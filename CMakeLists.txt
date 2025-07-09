cmake_minimum_required(VERSION 3.19)
project(audioFileAnalyzer LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Multimedia Charts)

qt_standard_project_setup()

qt_add_executable(audioFileAnalyzer
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    audiomodel.h audiomodel.cpp
    waveformview.h waveformview.cpp
    spectrumview.h spectrumview.cpp
)

target_link_libraries(audioFileAnalyzer
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::Multimedia
        Qt6::Charts
)

include(GNUInstallDirs)

install(TARGETS audioFileAnalyzer
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET audioFileAnalyzer
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
