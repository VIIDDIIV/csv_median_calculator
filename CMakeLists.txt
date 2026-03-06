cmake_minimum_required(VERSION 3.23)
project(csv_median_calculator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ------------------------------------------------------------
# 1. toml++
# ------------------------------------------------------------
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

# ------------------------------------------------------------
# 2. spdlog
# ------------------------------------------------------------
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)

# ------------------------------------------------------------
# 3. Boost (только program_options) через FetchContent
# ------------------------------------------------------------
set(BOOST_ENABLE_CMAKE ON)  # включает поддержку CMake для Boost
set(BOOST_INCLUDE_LIBRARIES "program_options")  # выбираем только нужную библиотеку

FetchContent_Declare(
    Boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG boost-1.85.0  # или другая версия >=1.82
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR .        # корень содержит все модули
)
FetchContent_GetProperties(Boost)
if(NOT Boost_POPULATED)
    FetchContent_Populate(Boost)
    # Добавляем поддиректорию с CMake-поддержкой (есть в новых версиях Boost)
    add_subdirectory(${boost_SOURCE_DIR} ${boost_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# ------------------------------------------------------------
# Исполняемый файл
# ------------------------------------------------------------
add_executable(${PROJECT_NAME} src/main.cpp ...  "src/streaming_processor.cpp" "src/csv_parser.cpp")

target_link_libraries(${PROJECT_NAME} PRIVATE
    tomlplusplus::tomlplusplus
    spdlog::spdlog
    Boost::program_options  # цель создаётся благодаря BOOST_ENABLE_CMAKE
)
target_sources(${PROJECT_NAME} PRIVATE
    src/main.cpp
    src/config_parser.cpp
    src/csv_reader.cpp
    src/median_calculator.cpp
)
configure_file(config.toml ${CMAKE_CURRENT_BINARY_DIR}/config.toml COPYONLY)
# Остальные настройки (флаги, etc.)