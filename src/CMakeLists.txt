include(functions.cmake)

#################################
# Common declarations.
#################################

set(CMAKE_DEBUG_POSTFIX d)

# Compiler tests
include(CheckCXXCompilerFlag)

# Specify the C++ standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#################################
# Find libraries.
#################################

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_subdirectory(bitmaps)
add_subdirectory(tools)

if(UNIX)
set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)
find_package(Curses)
if(NOT CURSES_FOUND)
    # Having a second try is at least needed for FreeBSD for
    # non wide character support which never the less results in
    # wide character support :-)).
    unset(CURSES_INCLUDE_DIRS)
    unset(CURSES_LIBRARIES)
    unset(CURSES_CFLAGS)
    unset(CURSES_HAVE_CURSES_H)
    unset(CURSES_HAVE_NCURSES_H)
    unset(CURSES_HAVE_NCURSES_CURSES_H)
    unset(CURSES_HAVE_NCURSES_NCURSES_H)
    set(CURSES_NEED_WIDE FALSE)
    find_package(Curses REQUIRED)
endif()

find_package(X11 REQUIRED)
endif()

# Enable compiler warnings
if(MSVC)
    add_compile_options(/W4 /wd"4996")
endif()
if(UNIX)
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wswitch-enum
        -Wfloat-conversion
        -Wundef
        -Wconversion
        -Wsign-conversion
        -Wfloat-conversion
    )
endif()

# Create file config.h
create_config_file("config.h.in" "config.h")

#################################
# Build libflex.a static library.
#################################

set(flex_SOURCES
    bdate.cpp
    bdir.cpp
    benv.cpp
    binifile.cpp
    bmembuf.cpp
    bobservd.cpp
    bprocess.cpp
    brcfile.cpp
    bregistr.cpp
    breltime.cpp
    btime.cpp
    cvtwchar.cpp
    dircont.cpp
    fattrib.cpp
    fcinfo.cpp
    fcnffile.cpp
    fcopyman.cpp
    fdirent.cpp
    ffilebuf.cpp
    ffilecnt.cpp
    filecnts.cpp
    fileread.cpp
    filfschk.cpp
    flexerr.cpp
    free.cpp
    idircnt.cpp
    iffilcnt.cpp
    ifilecnt.cpp
    mdcrtape.cpp
    memory.cpp
    misc1.cpp
    rfilecnt.cpp
    rndcheck.cpp
)
set(flex_HEADER
    bcommand.h
    bdate.h
    bdir.h
    benv.h
    binifile.h
    bmembuf.h
    bobserv.h
    bobservd.h
    bobshelp.h
    bprocess.h
    brcfile.h
    bregistr.h
    breltime.h
    btime.h
    cistring.h
    config.h
    cvtwchar.h
    dircont.h
    efiletim.h
    fattrib.h
    fcinfo.h
    fcnffile.h
    fcopyman.h
    fdirent.h
    ffilebuf.h
    ffilecnt.h
    filecntb.h
    filecnts.h
    fileread.h
    filecont.h
    filfschk.h
    flexemu.h
    flexerr.h
    free.h
    fversion.h
    idircnt.h
    iffilcnt.h
    ifilcnti.h
    ifilecnt.h
    mdcrtape.h
    memory.h
    memtype.h
    misc1.h
    ostype.h
    rfilecnt.h
    rndcheck.h
    typedefs.h
    windefs.h
)
add_library(flex STATIC ${flex_SOURCES} ${flex_HEADER})
add_library(flexemu::libflex ALIAS flex)

target_include_directories(flex SYSTEM PRIVATE ${fmt_INCLUDE_DIR})
target_include_directories(flex PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
if(FLEXEMU_USE_PRECOMPILE_HEADERS)
    target_precompile_headers(flex PRIVATE pch.h)
endif()
if(UNIX)
    target_compile_definitions(flex PRIVATE UNIX)
endif()
target_compile_definitions(flex PRIVATE
    USE_CMAKE
    F_DATADIR=\"${FLEXEMU_FULL_DATADIR}\"
    F_SYSCONFDIR=\"${CMAKE_INSTALL_FULL_SYSCONFDIR}\"
)

#################################
# Build updatemd executable.
# (internal only, Unix like OS only.).
#################################

if(UNIX)
set(updatemd_SOURCES
    fversion.cpp
    updatemd.cpp
)
set(updatemd_HEADER
    benv.h
    bprocess.h
    free.h
    fversion.h
    ostype.h
    misc1.h
)
add_executable(updatemd ${updatemd_SOURCES} ${updatemd_HEADER})

target_include_directories(updatemd SYSTEM PRIVATE
    ${fmt_INCLUDE_DIR}
    ${jsoncpp_INCLUDE_DIR}
)
target_include_directories(updatemd PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
if(FLEXEMU_USE_PRECOMPILE_HEADERS)
    target_precompile_headers(updatemd REUSE_FROM flexemu::libflex)
endif()
target_compile_definitions(updatemd PRIVATE
    UNIX
    USE_CMAKE
    ADD_JSONCPP_VERSION
)
target_link_libraries(updatemd PRIVATE
    flexemu::libflex
    fmt::fmt
    Threads::Threads
    jsoncpp_lib
)
endif()

#################################
# Build flex2hex executable.
#################################

set(flex2hex_SOURCES
    flex2hex.cpp
    fversion.cpp
    wingtopt.cpp
    wmain.cpp
)
set(flex2hex_HEADER
    bintervl.h
    bmembuf.h
    config.h
    cvtwchar.h
    fileread.h
    flexerr.h
    free.h
    fversion.h
    memsrc.h
    memtgt.h
    misc1.h
    ostype.h
    typedefs.h
    wingtopt.h
    wmain.h
)
add_executable(flex2hex ${flex2hex_SOURCES} ${flex2hex_HEADER})
if(WIN32)
    target_sources(flex2hex PRIVATE flex2hex.rc)
endif()

target_include_directories(flex2hex SYSTEM PRIVATE ${fmt_INCLUDE_DIR})
target_include_directories(flex2hex PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
if(FLEXEMU_USE_PRECOMPILE_HEADERS)
    target_precompile_headers(flex2hex REUSE_FROM flexemu::libflex)
endif()
if(UNIX)
    target_compile_definitions(flex2hex PRIVATE UNIX)
endif()
if(WIN32)
    target_compile_definitions(flex2hex PRIVATE _CONSOLE)
endif()
target_compile_definitions(flex2hex PRIVATE USE_CMAKE)
target_link_libraries(flex2hex PRIVATE
    flexemu::libflex
    fmt::fmt
    Threads::Threads
)

#################################
# Build hex2flex executable.
#################################

set(hex2flex_SOURCES
    hex2flex.cpp
    fversion.cpp
    wingtopt.cpp
    wmain.cpp
)
set(hex2flex_HEADER
    bintervl.h
    bmembuf.h
    config.h
    cvtwchar.h
    fileread.h
    flexerr.h
    free.h
    fversion.h
    memsrc.h
    memtgt.h
    misc1.h
    ostype.h
    typedefs.h
    wingtopt.h
    wmain.h
)
add_executable(hex2flex ${hex2flex_SOURCES} ${hex2flex_HEADER})
if(WIN32)
    target_sources(hex2flex PRIVATE hex2flex.rc)
endif()

target_include_directories(hex2flex SYSTEM PRIVATE ${fmt_INCLUDE_DIR})
target_include_directories(hex2flex PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
if(FLEXEMU_USE_PRECOMPILE_HEADERS)
    target_precompile_headers(hex2flex REUSE_FROM flexemu::libflex)
endif()
if(UNIX)
    target_compile_definitions(hex2flex PRIVATE UNIX)
endif()
if(WIN32)
    target_compile_definitions(hex2flex PRIVATE _CONSOLE)
endif()
target_compile_definitions(hex2flex PRIVATE USE_CMAKE)
target_link_libraries(hex2flex PRIVATE
    flexemu::libflex
    fmt::fmt
    Threads::Threads
)

#################################
# Build mdcrtool executable.
#################################

set(mdcrtool_SOURCES
    fversion.cpp
    mdcrfs.cpp
    mdcrtool.cpp
    wingtopt.cpp
    wmain.cpp
)
set(mdcrtool_HEADER
    bdir.h
    bintervl.h
    bmembuf.h
    config.h
    fileread.h
    flexerr.h
    free.h
    fversion.h
    mdcrfs.h
    mdcrtape.h
    memsrc.h
    memtgt.h
    misc1.h
    ostype.h
    typedefs.h
    wingtopt.h
    wmain.h
)
add_executable(mdcrtool ${mdcrtool_SOURCES} ${mdcrtool_HEADER})
if(WIN32)
    target_sources(mdcrtool PRIVATE mdcrtool.rc)
endif()

target_include_directories(mdcrtool SYSTEM PRIVATE ${fmt_INCLUDE_DIR})
target_include_directories(mdcrtool PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
if(FLEXEMU_USE_PRECOMPILE_HEADERS)
    target_precompile_headers(mdcrtool REUSE_FROM flexemu::libflex)
endif()
if(UNIX)
    target_compile_definitions(mdcrtool PRIVATE UNIX)
endif()
if(WIN32)
    target_compile_definitions(mdcrtool PRIVATE _CONSOLE)
endif()
target_compile_definitions(mdcrtool PRIVATE USE_CMAKE)
target_link_libraries(mdcrtool PRIVATE
    flexemu::libflex
    fmt::fmt
    Threads::Threads
)

#################################
# Build dsktool executable.
#################################

set(dsktool_SOURCES
    dsktool.cpp
    fversion.cpp
    wingtopt.cpp
    wmain.cpp
)
set(dsktool_HEADER
    bdate.h
    bdir.h
    btime.h
    cistring.h
    config.h
    cvtwchar.h
    dircont.h
    efiletim.h
    fcinfo.h
    fcopyman.h
    fdirent.h
    ffilebuf.h
    ffilecnt.h
    filecntb.h
    filecnts.h
    filecont.h
    filfschk.h
    flexemu.h
    flexerr.h
    free.h
    fversion.h
    idircnt.h
    iffilcnt.h
    ifilcnti.h
    ifilecnt.h
    misc1.h
    ostype.h
    rfilecnt.h
    typedefs.h
    wingtopt.h
    wmain.h
)
add_executable(dsktool ${dsktool_SOURCES} ${dsktool_HEADER})
if(WIN32)
    target_sources(dsktool PRIVATE dsktool.rc)
endif()

target_include_directories(dsktool SYSTEM PRIVATE ${fmt_INCLUDE_DIR})
target_include_directories(dsktool PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
if(FLEXEMU_USE_PRECOMPILE_HEADERS)
    target_precompile_headers(dsktool REUSE_FROM flexemu::libflex)
endif()
if(UNIX)
    target_compile_definitions(dsktool PRIVATE UNIX)
endif()
if(WIN32)
    target_compile_definitions(dsktool PRIVATE _CONSOLE)
endif()
target_compile_definitions(dsktool PRIVATE USE_CMAKE)
target_link_libraries(dsktool PRIVATE
    flexemu::libflex
    fmt::fmt
    Threads::Threads
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (${FLEXEMU_QT_MAJOR_VERSION} EQUAL 5)
    find_package(Qt5 REQUIRED Core Widgets Gui PrintSupport Concurrent)
    if (UNIX)
        if (APPLE)
            find_package(Qt5 REQUIRED MacExtras)
        else()
            find_package(Qt5 REQUIRED X11Extras)
        endif()
    endif()
endif()
if (${FLEXEMU_QT_MAJOR_VERSION} EQUAL 6)
    find_package(Qt6 REQUIRED Core Widgets Gui PrintSupport Concurrent)
    if (UNIX AND APPLE)
        find_package(Qt6 REQUIRED MacExtras)
    endif()
endif()

#################################
# Build flexemu executable.
#################################

set(flexemu_gen_SOURCES
    flexemu.qrc
    about.ui
    brkpt.ui
    cpustat.ui
    find.ui
    fsetup.ui
    logfil.ui
    memset.ui
    pprev.ui
)
set(flexemu_SOURCES
    absgui.cpp
    acia1.cpp
    apprun.cpp
    bjoystck.cpp
    blinxsys.cpp
    brkptui.cpp
    bui.cpp
    bytereg.cpp
    cacttrns.cpp
    ccopymem.cpp
    clogfile.cpp
    colors.cpp
    command.cpp
    csetfreq.cpp
    cwritmem.cpp
    da6809.cpp
    drawnwid.cpp
    drisel.cpp
    e2floppy.cpp
    e2screen.cpp
    efslctle.cpp
    fdoptman.cpp
    finddata.cpp
    findui.cpp
    flblfile.cpp
    foptman.cpp
    fsetupui.cpp
    fversion.cpp
    hexdump.cpp
    inout.cpp
    iodevdbg.cpp
    joystick.cpp
    keyboard.cpp
    logfilui.cpp
    main.cpp
    mc146818.cpp
    mc6809.cpp
    mc6809in.cpp
    mc6809lg.cpp
    mc6809st.cpp
    mc6821.cpp
    mc6850.cpp
    memsetui.cpp
    memwin.cpp
    memwinmg.cpp
    mmu.cpp
    mwtedit.cpp
    ndircont.cpp
    pagedet.cpp
    pagedetd.cpp
    pia1.cpp
    pia2.cpp
    pia2v5.cpp
    poutwin.cpp
    poverhlp.cpp
    qtfree.cpp
    qtgui.cpp
    schedule.cpp
    sodiff.cpp
    soptions.cpp
    termimpc.cpp
    termimpd.cpp
    termimpf.cpp
    termimps.cpp
    terminal.cpp
    tstdev.cpp
    vico1.cpp
    vico2.cpp
    wd1793.cpp
    wingtopt.cpp
    winmain.cpp
)
set(flexemu_HEADER
    asciictl.h
    absdisas.h
    absgui.h
    acia1.h
    apprun.h
    bcommand.h
    bdate.h
    bdir.h
    benv.h
    binifile.h
    bintervl.h
    bitops.h
    blinxsys.h
    bjoystck.h
    bmembuf.h
    bmp.h
    bobserv.h
    bobservd.h
    bobshelp.h
    brcfile.h
    bregistr.h
    brkptui.h
    breltime.h
    bscopeex.h
    btime.h
    bui.h
    bytereg.h
    cacttrns.h
    ccopymem.h
    cistring.h
    clogfile.h
    colors.h
    command.h
    config.h
    cpustate.h
    crc.h
    csetfreq.h
    cvtwchar.h
    cwritmem.h
    da6809.h
    debug.h
    drawnwid.h
    drisel.h
    e2.h
    e2floppy.h
    e2screen.h
    efiletim.h
    efslctle.h
    engine.h
    fattrib.h
    fcinfo.h
    fcnffile.h
    fcopyman.h
    fdirent.h
    fdoptman.h
    ffilebuf.h
    ffilecnt.h
    filecntb.h
    filecnts.h
    fileread.h
    finddata.h
    findui.h
    flblfile.h
    flexemu.h
    flexerr.h
    foptman.h
    free.h
    fsetupui.h
    fversion.h
    hexdump.h
    iffilcnt.h
    ifilcnti.h
    ifilecnt.h
    inout.h
    iodevdbg.h
    iodevice.h
    joystick.h
    keyboard.h
    logfilui.h
    mc146818.h
    mc6809.h
    mc6809lg.h
    mc6809st.h
    mc6821.h
    mc6850.h
    mdcrtape.h
    memory.h
    memsetui.h
    memsrc.h
    memtgt.h
    memwin.h
    memwinmg.h
    mwtedit.h
    misc1.h
    mmu.h
    ndircont.h
    ostype.h
    pagedet.h
    pagedetd.h
    pia1.h
    pia2.h
    pia2v5.h
    poutwin.h
    poverhlp.h
    propsui.h
    qtfree.h
    qtgui.h
    rfilecnt.h
    rndcheck.h
    schedcpu.h
    schedule.h
    scpulog.h
    sodiff.h
    soptions.h
    termimpc.h
    termimpd.h
    termimpf.h
    termimpi.h
    termimps.h
    terminal.h
    tstdev.h
    typedefs.h
    vico1.h
    vico2.h
    warnoff.h
    warnon.h
    wd1793.h
    winctxt.h
    windefs.h
    wingtopt.h
    winmain.h
)
add_executable(flexemu WIN32 ${flexemu_gen_SOURCES} ${flexemu_SOURCES} ${flexemu_HEADER})
if(WIN32)
    target_sources(flexemu PRIVATE flexemu.rc)
endif()

target_include_directories(flexemu PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(flexemu SYSTEM PRIVATE
    ${fmt_INCLUDE_DIR}
    ${Qt${FLEXEMU_QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
    ${Qt${FLEXEMU_QT_MAJOR_VERSION}Widgets_INCLUDE_DIRS}
    ${Qt${FLEXEMU_QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
    ${Qt${FLEXEMU_QT_MAJOR_VERSION}PrintSupport_INCLUDE_DIRS}
    ${Qt${FLEXEMU_QT_MAJOR_VERSION}Concurrent_INCLUDE_DIRS}
)
if(UNIX)
    if(APPLE)
        target_include_directories(flexemu SYSTEM PRIVATE
            ${Qt${FLEXEMU_QT_MAJOR_VERSION}MacExtras_INCLUDE_DIRS}
        )
    else()
        target_include_directories(flexemu SYSTEM PRIVATE
            ${Qt${FLEXEMU_QT_MAJOR_VERSION}X11Extras_INCLUDE_DIRS}
        )
    endif()
    target_include_directories(flexemu SYSTEM PRIVATE
        ${X11_INCLUDE_DIR}
        ${CURSES_INCLUDE_DIRS}
    )
endif()
if(FLEXEMU_USE_PRECOMPILE_HEADERS)
    target_precompile_headers(flexemu REUSE_FROM flexemu::libflex)
endif()
if(UNIX)
    target_compile_definitions(flexemu PRIVATE UNIX)
endif()
if(WIN32)
    target_compile_definitions(flexemu PRIVATE _WINDOWS)
endif()
target_compile_definitions(flexemu PRIVATE
    USE_CMAKE
    ADD_NCURSES_VERSION
    F_DATADIR=\"${FLEXEMU_FULL_DATADIR}\"
    F_SYSCONFDIR=\"${CMAKE_INSTALL_FULL_SYSCONFDIR}\"
)
if (ALTERNATE_MC6809)
    target_compile_definitions(flexemu PRIVATE -DALTERNATE_MC6809)
    message("-- Using Mc6809 implemenation: Alternate")
else()
    message("-- Using Mc6809 implemenation: Default")
endif()

if (${FLEXEMU_QT_MAJOR_VERSION} EQUAL 5)

    if(UNIX)
        if(APPLE)
            target_link_libraries(flexemu PRIVATE Qt5::MacExtras)
        else()
            target_link_libraries(flexemu PRIVATE Qt5::X11Extras)
        endif()
    endif()
    target_link_libraries(flexemu PRIVATE
        Qt5::Concurrent
        Qt5::PrintSupport
        Qt5::Gui
        Qt5::Widgets
        Qt5::Core
    )
endif()
if (${FLEXEMU_QT_MAJOR_VERSION} EQUAL 6)
    if(UNIX AND APPLE)
        target_link_libraries(flexemu PRIVATE Qt6::MacExtras)
    endif()
    target_link_libraries(flexemu PRIVATE
        Qt6::Concurrent
        Qt6::PrintSupport
        Qt6::Gui
        Qt6::Widgets
        Qt6::Core
    )
endif()
target_link_libraries(flexemu PRIVATE
    flexemu::libflex
    fmt::fmt
)
if(UNIX)
    target_link_libraries(flexemu PRIVATE
        X11::Xkb
        X11::X11
        ${CURSES_LIBRARIES}
        Threads::Threads
    )
endif()

#################################
# Build flexplorer executable.
#################################

set(flexplorer_gen_SOURCES
    fpmain.qrc
    about.ui
    fpnew.ui
    fpopt.ui
    fpcnv.ui
)
set(flexplorer_SOURCES
    drawnwid.cpp
    fpattrui.cpp
    fpcnvui.cpp
    fpdnd.cpp
    fpedit.cpp
    fpmain.cpp
    fpmdich.cpp
    fpmodel.cpp
    fpnewui.cpp
    fpoptman.cpp
    fpoptui.cpp
    fpwin.cpp
    fversion.cpp
    qtfree.cpp
)
set(flexplorer_HEADER
    bdate.h
    bdir.h
    benv.h
    bintervl.h
    bmembuf.h
    bprocess.h
    brcfile.h
    bregistr.h
    btime.h
    cistring.h
    config.h
    cvtwchar.h
    dircont.h
    drawnwid.h
    efilesiz.h
    efiletim.h
    fattrib.h
    fcinfo.h
    fcopyman.h
    fdirent.h
    ffilebuf.h
    ffilecnt.h
    filecntb.h
    filecnts.h
    filecont.h
    flexemu.h
    flexerr.h
    fpattrui.h
    fpcnvui.h
    fpdnd.h
    fpedit.h
    fpmdich.h
    fpmodel.h
    fpnewui.h
    fpoptman.h
    fpoptui.h
    fpwin.h
    free.h
    fversion.h
    idircnt.h
    iffilcnt.h
    ifilcnti.h
    ifilecnt.h
    mdcrtape.h
    memsrc.h
    memtgt.h
    misc1.h
    ostype.h
    propsui.h
    qtfree.h
    rndcheck.h
    rfilecnt.h
    sfpopts.h
    warnoff.h
    warnon.h
    windefs.h
)
add_executable(flexplorer WIN32 ${flexplorer_gen_SOURCES} ${flexplorer_SOURCES} ${flexplorer_HEADER})
if(WIN32)
    target_sources(flexplorer PRIVATE flexdisk.rc)
endif()

target_include_directories(flexplorer PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(flexplorer SYSTEM PRIVATE
    ${fmt_INCLUDE_DIR}
    ${Qt${FLEXEMU_QT_MAJOR_VERSION}Core_INCLUDE_DIRS}
    ${Qt${FLEXEMU_QT_MAJOR_VERSION}Widgets_INCLUDE_DIRS}
    ${Qt${FLEXEMU_QT_MAJOR_VERSION}Gui_INCLUDE_DIRS}
    ${X11_INCLUDE_DIR}
)
if(FLEXEMU_USE_PRECOMPILE_HEADERS)
    target_precompile_headers(flexplorer REUSE_FROM flexemu::libflex)
endif()
if(UNIX)
    target_compile_definitions(flexplorer PRIVATE UNIX)
endif()
if(WIN32)
    target_compile_definitions(flexplorer PRIVATE _WINDOWS)
endif()
target_compile_definitions(flexplorer PRIVATE
    USE_CMAKE
    F_DATADIR=\"${FLEXEMU_FULL_DATADIR}\"
)
if (${FLEXEMU_QT_MAJOR_VERSION} EQUAL 5)
    target_link_libraries(flexplorer PRIVATE
        Qt5::Gui
        Qt5::Widgets
        Qt5::Core
    )
endif()
if (${FLEXEMU_QT_MAJOR_VERSION} EQUAL 6)
    target_link_libraries(flexplorer PRIVATE
        Qt6::Gui
        Qt6::Widgets
        Qt6::Core
    )
endif()
target_link_libraries(flexplorer PRIVATE
    flexemu::libflex
    fmt::fmt
)
if(UNIX)
    target_link_libraries(flexplorer PRIVATE
        X11::X11
        Threads::Threads
    )
endif()

if(UNIX)
# Add custom target always executed to update src/metadata.json.
add_custom_target(
    metadata_src ALL
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/updatemd -l -k src -f metadata.json
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS updatemd ${flex_SOURCES} ${flex2hex_SOURCES} ${dsktool_SOURCES} ${mdcrtool_SOURCES} ${flexemu_SOURCES} ${flexplorer_SOURCES}
    COMMENT "Updating src/metadata.json"
    USES_TERMINAL
    VERBATIM
)
endif()

#################################
# clang-tidy support
#################################

if(UNIX)
    set(ARGUMENTS bitfield.cpp)
    list(APPEND ARGUMENTS
        ${updatemd_SOURCES}
        ${flex_SOURCES}
        ${flex2hex_SOURCES}
        ${dsktool_SOURCES}
        ${mdcrtool_SOURCES}
        ${flexemu_SOURCES}
        ${flexplorer_SOURCES}
    )
    list(APPEND ARGUMENTS -p=${CMAKE_BINARY_DIR})
    list(APPEND ARGUMENTS --)
    list(APPEND ARGUMENTS -isystem${CMAKE_CURRENT_BINARY_DIR})
    list(APPEND ARGUMENTS -isystem${CMAKE_CURRENT_BINARY_DIR}/flexemu_autogen/include)
    list(APPEND ARGUMENTS -isystem${CMAKE_CURRENT_BINARY_DIR}/flexplorer_autogen/include)
    list(APPEND ARGUMENTS -isystem${fmt_INCLUDE_DIR})
    list(APPEND ARGUMENTS -isystem${jsoncpp_INCLUDE_DIR})
    foreach(QT_INCLUDE_DIR ${Qt${FLEXEMU_QT_MAJOR_VERSION}PrintSupport_INCLUDE_DIRS})
        list(APPEND ARGUMENTS -isystem${QT_INCLUDE_DIR})
    endforeach()
    if (${FLEXEMU_QT_MAJOR_VERSION} EQUAL 5 AND NOT APPLE)
        foreach(QT_INCLUDE_DIR ${Qt5X11Extras_INCLUDE_DIRS})
            list(APPEND ARGUMENTS -isystem${QT_INCLUDE_DIR})
        endforeach()
    endif()
    if(APPLE)
        foreach(QT_INCLUDE_DIR ${Qt${FLEXEMU_QT_MAJOR_VERSION}MacExtras_INCLUDE_DIRS})
            list(APPEND ARGUMENTS -isystem${QT_INCLUDE_DIR})
        endforeach()
    endif()
    list(APPEND ARGUMENTS -DF_DATADIR=\"${FLEXEMU_FULL_DATADIR}\")
    list(APPEND ARGUMENTS -DF_SYSCONFDIR=\"${CMAKE_INSTALL_FULL_SYSCONFDIR}\")
    list(APPEND ARGUMENTS -DUSE_CMAKE)
    list(APPEND ARGUMENTS -D_REENTRANT)
    list(APPEND ARGUMENTS -DUNIX)

    # Add custom target to execute clang-tidy, called clang-tidy-check.
    add_custom_target(
        clang-tidy-check
        ${CLANG_TIDY_CMD} ${ARGUMENTS}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Executing clang-tidy checks"
        USES_TERMINAL
        VERBATIM
    )
    add_dependencies(clang-tidy-check
        flexemu_autogen
        flexplorer_autogen
    )

    set(PROCESSES "")
    if(DEFINED ENV{RUN_CLANG_TIDY_PROCESSES})
        list(APPEND ARGUMENTS -j=$ENV{RUN_CLANG_TIDY_PROCESSES})
        if("$ENV{RUN_CLANG_TIDY_PROCESSES}" EQUAL "1")
            set(PROCESSES "(1 process)")
        else()
            set(PROCESSES "($ENV{RUN_CLANG_TIDY_PROCESSES} processes)")
        endif()
    endif()

    set(ARGUMENTS -clang-tidy-binary=${CLANG_TIDY_CMD})
    list(APPEND ARGUMENTS -p=${CMAKE_BINARY_DIR})
    list(APPEND ARGUMENTS -source-filter= ".*/(test/test_[a-z][a-z0-9]+\.cpp|src/[a-z][a-z0-9]+\.cpp)")
    list(APPEND ARGUMENTS -extra-arg=-I${CMAKE_CURRENT_BINARY_DIR})
    list(APPEND ARGUMENTS -extra-arg=-I${CMAKE_CURRENT_BINARY_DIR}/flexemu_autogen/include)
    list(APPEND ARGUMENTS -extra-arg=-I${CMAKE_CURRENT_BINARY_DIR}/flexplorer_autogen/include)

    # Add custom target to execute run-clang-tidy. It is called
    # fast-clang-tidy-check because it automatically distributes the clang-tidy
    # checks among the available CPU cores.
    add_custom_target(
        fast-clang-tidy-check
        ${RUN_CLANG_TIDY_CMD} ${ARGUMENTS}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Executing fast clang-tidy checks ${PROCESSES}"
        USES_TERMINAL
        VERBATIM
    )
    add_dependencies(fast-clang-tidy-check
        flexemu_autogen
        flexplorer_autogen
    )
endif()

set(APPLICATIONS_FULL_DIR "${CMAKE_INSTALL_FULL_DATADIR}/applications")

install(TARGETS flex2hex hex2flex mdcrtool dsktool flexemu flexplorer)
install(FILES boot DESTINATION ${CMAKE_INSTALL_DATADIR}/flexemu)
install(FILES flexemu.conf flexlabl.conf TYPE SYSCONF)
install(FILES flexemu.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/mime/packages)
install(FILES flexemu.desktop flexemu-flexplorer.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

if(UNIX AND FLEXEMU_POST_INSTALL)

    # Check for tool existence for updating icon cache and desktop/mime
    # database.
    set(UPDATE_ICON_CACHE_EXE "gtk-update-icon-cache")
    find_program(GTK_UPDATE_ICON_CACHE ${UPDATE_ICON_CACHE_EXE})
    set(UPDATE_MIME_DB_EXE "update-mime-database")
    find_program(XDG_UPDATE_MIME_DATABASE ${UPDATE_MIME_DB_EXE})
    set(UPDATE_DESKTOP_DB_EXE "update-desktop-database")
    find_program(XDG_UPDATE_DESKTOP_DATABASE ${UPDATE_DESKTOP_DB_EXE})

    set(ICONS_DIR "${CMAKE_INSTALL_FULL_DATADIR}/icons")
    set(HICOLOR_DIR "${ICONS_DIR}/hicolor")
    set(MIME_DIR "${CMAKE_INSTALL_FULL_DATADIR}/mime")
    set(APPLICATION_DIR "${MIME_DIR}/application")

    # Variables set with parent scope. They are needed for the uninstall
    # target used in the upper directory.
    set(HICOLOR_DIR "${HICOLOR_DIR}" PARENT_SCOPE)
    set(MIME_DIR "${MIME_DIR}" PARENT_SCOPE)
    set(APPLICATIONS_FULL_DIR "${APPLICATIONS_FULL_DIR}" PARENT_SCOPE)

    install(CODE "file(MAKE_DIRECTORY \"${HICOLOR_DIR}\" \"${APPLICATION_DIR}\")")

    string(FIND ${GTK_UPDATE_ICON_CACHE} "NOTFOUND" STR_INDEX)
    if (${STR_INDEX} EQUAL -1)
        install(CODE "execute_process(COMMAND echo \"-- Updating: icon cache\")")
        install(CODE "execute_process(COMMAND ${GTK_UPDATE_ICON_CACHE} -f -t -q \"${HICOLOR_DIR}\")")
    else()
        message(WARNING "${UPDATE_ICON_CACHE_EXE} executable not found - icon cache will not be updated.")
    endif()

    string(FIND ${XDG_UPDATE_MIME_DATABASE} "NOTFOUND" STR_INDEX)
    if (${STR_INDEX} EQUAL -1)
        install(CODE "execute_process(COMMAND echo \"-- Updating: mime database\")")
        install(CODE "execute_process(COMMAND ${XDG_UPDATE_MIME_DATABASE} \"${MIME_DIR}\")")
    else()
        message(WARNING "${UPDATE_MIME_DB_EXE} executable not found - mime database will not be updated.")
    endif()

    string(FIND ${XDG_UPDATE_DESKTOP_DATABASE} "NOTFOUND" STR_INDEX)
    if (${STR_INDEX} EQUAL -1)
        install(CODE "execute_process(COMMAND echo \"-- Updating: desktop database\")")
        install(CODE "execute_process(COMMAND ${XDG_UPDATE_DESKTOP_DATABASE} \"${APPLICATIONS_FULL_DIR}\")")
    else()
        message(WARNING "${UPDATE_DESKTOP_DB_EXE} executable not found - desktop database will not be updated.")
    endif()
endif()
