include(CheckIncludeFiles)
include(CheckTypeSize)
include(CheckFunctionExists)
include(TestBigEndian)

# Check if C bitfields are stored in little or big endian format.
#
# Input param:
# byteOrderVarName variable name containing byte order string
# - not defined => byte order is estimated.
# - is "LITTLE_ENDIAN"
# - is "BIG_ENDIAN"
#
# Output param:
# outVarName variable name of result (with PARENT_SCOPE)
# - is set to 1 => little endian
# - is not defined => big endian

function(ESTIMATE_BITFIELD_BYTE_ORDER byteOrderVarName outVarName)
    if (DEFINED ${byteOrderVarName})
        # Rely on defined variable.
        string(TOUPPER "${${byteOrderVarName}}" ${byteOrderVarName})
        if(NOT (${${byteOrderVarName}} STREQUAL "BIG_ENDIAN") AND
           NOT (${${byteOrderVarName}} STREQUAL "LITTLE_ENDIAN"))
           message(FATAL_ERROR "${byteOrderVarName} has unsupported value \"${${byteOrderVarName}}\". Only \"BIG_ENDIAN\" or \"LITTLE_ENDIAN\" is supported.")
        endif()
        if(${${byteOrderVarName}} STREQUAL "LITTLE_ENDIAN")
            set(${outVarName} 1 PARENT_SCOPE)
        endif()
    else()
        if (CMAKE_CROSSCOMPILING)
            message(FATAL_ERROR "In cross compilation mode the bitfield byte order can not be estimated automatically. To avoid this error set the variable BITFIELD_BYTE_ORDER to \"BIG_ENDIAN\" or \"LITTLE_ENDIAN\".")
        else()
        # Try to run a test program to check if C bitfields are stored in
        # little or big endian format.
            try_run(
                BITFIELD_RUN_RESULT
                BITFIELD_COMPILE_RESULT
                ${CMAKE_CURRENT_BINARY_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/bitfield.cpp
                RUN_OUTPUT_VARIABLE BITFIELD_STDOUT)
            if("${BITFIELD_COMPILE_RESULT}" AND ("${BITFIELD_RUN_RESULT}" EQUAL 0))
                string(STRIP "${BITFIELD_STDOUT}" BITFIELD_STDOUT)
                if("${BITFIELD_STDOUT}" STREQUAL "1")
                    set(${outVarName} 1 PARENT_SCOPE)
                endif()
            else()
                message(FATAL_ERROR "Test bitfield.cpp can not be compiled or executed. To avoid this error set the variable ${byteOrderVarName} to \"BIG_ENDIAN\" or \"LITTLE_ENDIAN\".")
            endif()
        endif()
    endif()
endfunction()

# Create a file config.h based on a file template config.h.in
#
# Input param:
# inFile  file name of cmake template file
# outFile file name of output file
function(CREATE_CONFIG_FILE inFile outFile)

    set(USE_CMAKE ON)
    if (CURSES_LIBRARIES)
        string(FIND "${CURSES_LIBRARIES}" "ncursesw" STR_INDEX)
        if (NOT (${STR_INDEX} EQUAL -1))
            set(HAVE_NCURSESW ON)
        endif()
    endif()
    if (CURSES_HAVE_NCURSES_H)
        string(FIND "${CURSES_HAVE_NCURSES_H}" "ncursesw.h" STR_INDEX)
        if (NOT (${STR_INDEX} EQUAL -1))
            set(HAVE_NCURSESW_H ON)
        endif()
        string(FIND "${CURSES_HAVE_NCURSES_H}" "ncurses.h" STR_INDEX)
        if (NOT (${STR_INDEX} EQUAL -1))
            set(HAVE_NCURSES_H ON)
        endif()
        set(HAVE_NCURSES_H ON)
    endif()
    if (CURSES_HAVE_NCURSES_NCURSES_H)
        string(FIND "${CURSES_HAVE_NCURSES_NCURSES_H}" "ncursesw/" STR_INDEX)
        if (NOT (${STR_INDEX} EQUAL -1))
            set(HAVE_NCURSESW_NCURSES_H ON)
        endif()
        string(FIND "${CURSES_HAVE_NCURSES_CURSES_H}" "ncurses/" STR_INDEX)
        if (NOT (${STR_INDEX} EQUAL -1))
            set(HAVE_NCURSES_NCURSES_H ON)
        endif()
    endif()
    if (CURSES_HAVE_NCURSES_CURSES_H)
        string(FIND "${CURSES_HAVE_NCURSES_CURSES_H}" "ncursesw/" STR_INDEX)
        if (NOT (${STR_INDEX} EQUAL -1))
            set(HAVE_NCURSESW_CURSES_H ON)
        endif()
        string(FIND "${CURSES_HAVE_NCURSES_CURSES_H}" "ncurses/" STR_INDEX)
        if (NOT (${STR_INDEX} EQUAL -1))
            set(HAVE_NCURSES_CURSES_H ON)
        endif()
    endif()

    check_function_exists(setenv HAVE_DECL_SETENV)
    check_function_exists(unsetenv HAVE_DECL_UNSETENV)

    check_include_files("dirent.h" HAVE_DIRENT_H)
    check_include_files("fcntl.h" HAVE_FCNTL_H)
    check_include_files("inttypes.h" HAVE_INTTYPES_H)
    check_include_files("ndir.h" HAVE_NDIR_H)
    check_include_files("stdint.h" HAVE_STDINT_H)
    check_include_files("sys/dir.h" HAVE_SYS_DIR_H)
    check_include_files("sys/ndir.h" HAVE_SYS_NDIR_H)
    check_include_files("sys/statvfs.h" HAVE_SYS_STATVFS_H)
    check_include_files("sys/time.h" HAVE_SYS_TIME_H)
    check_include_files("sys/termios.h" HAVE_TERMIOS_H)
    check_include_files("sys/unistd.h" HAVE_UNISTD_H)
    check_include_files("linux/joystick.h" LINUX_JOYSTICK_IS_PRESENT)

    check_type_size("int" SIZEOF_INT)
    check_type_size("long" SIZEOF_LONG)
    check_type_size("short" SIZEOF_SHORT)

    if(CMAKE_CXX_BYTE_ORDER STREQUAL "BIG_ENDIAN")
        set(WORDS_BIGENDIAN 1)
    endif()

    # Estimate variable BITFIELDS_LSB_FIRST.
    # Check if C bitfields are stored in little or big endian format.
    # This is either done by a cmake command line parameter
    # -DFLEXEMU_BITFIELD_BYTE_ORDER=LITTLE_ENDIAN or
    # -DFLEXEMU_BITFIELD_BYTE_ORDER=BIG_ENDIAN.
    # (Most CPU architectures have LITTLE_ENDIAN).
    # or if not cross compiling by estimation by a *.cpp code snippet.
    estimate_bitfield_byte_order(FLEXEMU_BITFIELD_BYTE_ORDER BITFIELDS_LSB_FIRST)

    set(PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
    set(VERSION "${CMAKE_PROJECT_VERSION}")

    configure_file(${inFile} ${outFile} NO_SOURCE_PERMISSIONS @ONLY)

endfunction()
