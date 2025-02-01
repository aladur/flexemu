# cmake 3.16 supports MSVC 2019 which currently is the default IDE
# for flexemu on Windows.
# To support MSVC 2022 the minimum cmake version has to be set to 3.21

cmake_minimum_required(VERSION 3.16)

cmake_policy(SET CMP0115 NEW)

project(Flexemu
    VERSION 3.28
    DESCRIPTION "A Motorola MC6809 emulator running FLEX"
    HOMEPAGE_URL "https://flexemu.neocities.org/"
    LANGUAGES C CXX
)

# Optionally enable precompiled header support. Default is OFF.
# Using precompiled headers significantly increase the needed disk space
# during build with only little decrease of build time.
if(NOT DEFINED FLEXEMU_USE_PRECOMPILE_HEADERS)
    set(FLEXEMU_USE_PRECOMPILE_HEADERS OFF)
endif()

# Optionally disable post install steps on Unix like OS. Default is ON,
# on Apple systems the default is OFF.
# This is about mime file type support and desktop icons.
# For details see:
# https://specifications.freedesktop.org/desktop-entry-spec/latest/
# https://specifications.freedesktop.org/mime-apps-spec/latest/
if(NOT DEFINED FLEXEMU_POST_INSTALL AND NOT APPLE)
    set(FLEXEMU_POST_INSTALL ON)
endif()

# Optionally define the Qt major version to use. Supported is 5 or 6.
# If not set the default version used is 5.
if(NOT DEFINED FLEXEMU_QT_MAJOR_VERSION)
    set(FLEXEMU_QT_MAJOR_VERSION 5)
else()
    if (NOT (${FLEXEMU_QT_MAJOR_VERSION} EQUAL 5) AND
        NOT (${FLEXEMU_QT_MAJOR_VERSION} EQUAL 6))
        message(FATAL_ERROR "FLEXEMU_QT_MAJOR_VERSION only supports 5 or 6. Aborted")
    endif()
endif()

# Specify compiler settings.
if(NOT DEFINED CMAKE_BUILD_TYPE OR NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_DEBUG_POSTFIX d)
if (MSVC)
    set(CMAKE_C_FLAGS_DEBUG "")
    set(CMAKE_C_FLAGS_RELEASE "/O2")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
    set(CMAKE_C_FLAGS_DEBUG "-g")
    set(CMAKE_C_FLAGS_RELEASE "-O2")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif()

# Build 3rd party libraries.
# Hide all warnings for 3rdparty code
if (MSVC)
    add_compile_options(/W0)
endif()
if (UNIX)
    add_compile_options(-w)
endif()

add_subdirectory(fmt EXCLUDE_FROM_ALL)
set(fmt_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fmt/include")

option(INSTALL_GTEST OFF)
add_subdirectory(googletest EXCLUDE_FROM_ALL)
enable_testing()

option(JSONCPP_WITH_TESTS OFF)
add_subdirectory(jsoncpp EXCLUDE_FROM_ALL)
set(jsoncpp_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp/include")

# Prepare for clang-tidy.
if(UNIX)
# The clang-tidy command can be defined in environment variable CLANG_TIDY.
# If not defined the default is "clang-tidy".
set(CLANG_TIDY_STR $ENV{CLANG_TIDY})
find_program(CLANG_TIDY_CMD NAMES ${CLANG_TIDY_STR} clang-tidy)
string(FIND ${CLANG_TIDY_CMD} "NOTFOUND" STR_INDEX)
if (${STR_INDEX} EQUAL -1)
    message("-- Looking for clang-tidy - found ${CLANG_TIDY_CMD}")
    # The compile commands file is needed for clang-tidy support.
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
else()
    message(WARNING "${CLANG_TIDY_STR} or clang-tidy not found, 'make clang-tidy-check' will not work.")
endif()

# The run-clang-tidy command can be defined in environment variable CLANG_TIDY.
# If not defined the default is "run-clang-tidy".
set(RUN_CLANG_TIDY_STR $ENV{RUN_CLANG_TIDY})
find_program(RUN_CLANG_TIDY_CMD NAMES ${RUN_CLANG_TIDY_STR} run-clang-tidy)
string(FIND ${RUN_CLANG_TIDY_CMD} "NOTFOUND" STR_INDEX)
if (${STR_INDEX} EQUAL -1)
    message("-- Looking for run-clang-tidy - found ${RUN_CLANG_TIDY_CMD}")
    # The compile commands file is needed for clang-tidy support.
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
else()
    message(WARNING "${RUN_CLANG_TIDY_STR} or run-clang-tidy not found, 'make fast-clang-tidy-check' will not work.")
endif()
endif()

include(GNUInstallDirs)
set(FLEXEMU_FULL_DATADIR "${CMAKE_INSTALL_FULL_DATADIR}/flexemu")

set(install_FILES
    imhex/patterns/flex_binary.hexpat
    imhex/patterns/flex_dskflx.hexpat
    imhex/patterns/flex_random.hexpat
)

install(FILES ${install_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/flexemu/imhex/patterns)

# Build libraries and executables of this project
add_subdirectory(disks)
add_subdirectory(doc)
add_subdirectory(monitor)
add_subdirectory(test)
add_subdirectory(src)

if(UNIX)
# Add custom target always executed to update metadata.json.
add_custom_target(
    metadata ALL
    ${CMAKE_BINARY_DIR}/src/updatemd -c .clang-tidy -f metadata.json
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS updatemd .clang-tidy
    COMMENT "Updating metadata.json"
    USES_TERMINAL
    VERBATIM
)
endif()

# uninstall target
# see also file uninstall.cmake.in
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
