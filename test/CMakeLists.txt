#################################
# Common declarations.
#################################

enable_testing()

# Compiler tests
include(CheckCXXCompilerFlag)

# Specify the C++ standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Specify compiler settings.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
    add_compile_options(/W4 /wd"4996")
endif()
if(UNIX)
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
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

#################################
# Find libraries.
#################################

set(THREADS_PREFER_PTHREAD_FLAG TRUE)

# Copy test files to binary dir. They are needed when executing unittests.
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(unittests_SOURCES
    test_benv.cpp
    test_binifile.cpp
    test_blinxsys.cpp
    test_bobserv.cpp
    test_brcfile.cpp
    test_cistring.cpp
    test_colors.cpp
    test_da6809.cpp
    test_main.cpp
    test_mc6809lg.cpp
    test_misc1.cpp
    test_fcnffile.cpp
    test_fcinfo.cpp
    test_ffilebuf.cpp
    test_ffilecnts.cpp
    test_filecntb.cpp
    test_filecnts.cpp
    test_filecont.cpp
    test_fileread.cpp
    test_filesystem.cpp
    test_filfschk.cpp
    test_fdirent.cpp
    test_free.cpp
    test_bdir.cpp
    test_bdate.cpp
    test_bintervl.cpp
    test_bmembuf.cpp
    test_breltime.cpp
    test_btime.cpp
    test_rndcheck.cpp
    ../src/blinxsys.cpp
    ../src/colors.cpp
    ../src/da6809.cpp
    ../src/fdoptman.cpp
    ../src/flblfile.cpp
    ../src/free.cpp
    ../src/fversion.cpp
    ../src/mc6809lg.cpp
    ../src/mc6809st.cpp
    ../src/ndircont.cpp
    ../src/rndcheck.cpp
)
set(unittests_HEADER
    ../src/bdate.h
    ../src/bdir.h
    ../src/benv.h
    ../src/binifile.h
    ../src/blinxsys.h
    ../src/bintervl.h
    ../src/bobserv.h
    ../src/bobservd.h
    ../src/bobshelp.h
    ../src/breltime.h
    ../src/btime.h
    ../src/cistring.h
    ../src/colors.h
    ../src/da6809.h
    ../src/dircont.h
    ../src/fattrib.h
    ../src/fcinfo.h
    ../src/fcnffile.h
    ../src/fcopyman.h
    ../src/fdirent.h
    ../src/fdoptman.h
    ../src/ffilebuf.h
    ../src/ffilecnt.h
    ../src/filecnts.h
    ../src/fileread.h
    ../src/filfschk.h
    fixt_filecont.h
    ../src/flblfile.h
    ../src/flexerr.h
    ../src/free.h
    ../src/fversion.h
    ../src/idircnt.h
    ../src/iffilcnt.h
    ../src/ifilcnti.h
    ../src/ifilecnt.h
    ../src/mc6809lg.h
    ../src/mc6809st.h
    ../src/misc1.h
    ../src/ndircont.h
    ../src/rfilecnt.h
    ../src/rndcheck.h
    ../src/scpulog.h
)
add_executable(unittests ${unittests_SOURCES} ${unittests_HEADER})
target_include_directories(unittests PRIVATE
    "../src"
    ${CMAKE_BINARY_DIR}/src
)
target_include_directories(unittests SYSTEM PRIVATE
    ${gtest_SOURCE_DIR}/include
    ${gmock_SOURCE_DIR}/include
    ${fmt_INCLUDE_DIR}
)
if(FLEXEMU_USE_PRECOMPILE_HEADERS)
    target_precompile_headers(unittests PRIVATE pch.h)
endif()
if(UNIX)
    target_compile_definitions(unittests PRIVATE UNIX)
endif()
target_compile_definitions(unittests PRIVATE
    USE_CMAKE
    F_DATADIR=\"${FLEXEMU_FULL_DATADIR}\"
    F_SYSCONFDIR=\"${CMAKE_INSTALL_FULL_SYSCONFDIR}\"
)
target_link_libraries(unittests PRIVATE
    flexemu::libflex
    fmt::fmt
    gtest
)
# Working directory of the unittests executable is test subdirectory
# of the cmake build directory.
add_test(NAME unittests
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unittests
)

if(UNIX)
    set(test_gccasm_SOURCES
        ../src/clogfile.cpp
        ../src/colors.cpp
        ../src/command.cpp
        ../src/da6809.cpp
        ../src/flblfile.cpp
        ../src/foptman.cpp
        ../src/fversion.cpp
        ../src/inout.cpp
        ../src/mc6809.cpp
        ../src/mc6809in.cpp
        ../src/mc6809lg.cpp
        ../src/mc6809st.cpp
        ../src/schedule.cpp
        ../src/soptions.cpp
        test_gccasm.cpp
        testmain.cpp
    )
    set(test_gccasm_HEADER
        ../src/bdate.h
        ../src/bobserv.h
        ../src/bobservd.h
        ../src/bobshelp.h
        ../src/breltime.h
        ../src/btime.h
        ../src/clogfile.h
        ../src/command.h
        ../src/config.h
        ../src/cpustate.h
        ../src/csetfreq.h
        ../src/da6809.h
        ../src/debug.h
        ../src/e2.h
        ../src/engine.h
        ../src/fcinfo.h
        ../src/flblfile.h
        ../src/flexerr.h
        ../src/foptman.h
        ../src/fversion.h
        ../src/inout.h
        ../src/mc6809.h
        ../src/mc6809lg.h
        ../src/mc6809st.h
        ../src/memory.h
        ../src/misc1.h
        ../src/schedcpu.h
        ../src/schedule.h
        ../src/soptions.h
        test_gccasm.h
        ../src/typedefs.h
        ../src/warnoff.h
        ../src/warnon.h
    )
    add_executable(test_gccasm ${test_gccasm_SOURCES} ${test_gccasm_HEADER})
    target_include_directories(test_gccasm PRIVATE
        "../src"
        ${CMAKE_BINARY_DIR}/src
    )
    target_include_directories(test_gccasm SYSTEM PRIVATE
        ${gtest_SOURCE_DIR}/include
        ${gmock_SOURCE_DIR}/include
        ${fmt_INCLUDE_DIR}
    )
    if(FLEXEMU_USE_PRECOMPILE_HEADERS)
        target_precompile_headers(test_gccasm REUSE_FROM unittests)
    endif()
    if(UNIX)
        target_compile_definitions(test_gccasm PRIVATE UNIX)
    endif()
    target_compile_definitions(test_gccasm PRIVATE
        USE_CMAKE
        F_DATADIR=\"${FLEXEMU_FULL_DATADIR}\"
        F_SYSCONFDIR=\"${CMAKE_INSTALL_FULL_SYSCONFDIR}\"
    )
    target_link_libraries(test_gccasm PRIVATE
        flexemu::libflex
        fmt::fmt
    )

# Add custom target always executed to update test/metadata.json.
    add_custom_target(
        metadata_test ALL
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/updatemd -l -k test -f metadata.json
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS updatemd ${test_gccasm_SOURCES} ${unittests_SOURCES}
        COMMENT "Updating test/metadata.json"
        USES_TERMINAL
        VERBATIM
    )
endif()
