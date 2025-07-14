cmake_minimum_required(VERSION 3.19)
project(audioFileAnalyzer LANGUAGES CXX)

# Настройка путей для исходников и заголовков
set(SOURCE_DIR src)
set(INCLUDE_DIR include)

# Поиск и настройка Qt
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Multimedia Charts)
qt_standard_project_setup()

# Настройка библиотеки kissfft
set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(kissfft)

# Автоматическое добавление исходников
file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${INCLUDE_DIR}/*.h")
file(GLOB UI_FILES "*.ui")

# Создание исполняемого файла
qt_add_executable(audioFileAnalyzer
    WIN32 MACOSX_BUNDLE
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
)

# Настройка путей для заголовков
target_include_directories(audioFileAnalyzer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/kissfft  # Путь к заголовкам kissfft
)

# Подключение зависимостей
target_link_libraries(audioFileAnalyzer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Charts
    kissfft
)

# Установка и деплой
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
