if(UNIX)
    #################################
    # Common declarations.
    #################################

    # Specify C standard.
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_C_EXTENSIONS OFF)

    # Enable compiler warnings
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
    )

    #################################
    # Find libraries.
    #################################

    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)
    link_libraries(Threads::Threads)
    include_directories(${CMAKE_BINARY_DIR}/src)
    add_compile_definitions(
        UNIX
        USE_CMAKE
    )

    #################################
    # Build executables.
    #################################

    add_executable(bin2s19 bin2s19.c)
    add_executable(fromflex fromflex.c)
    add_executable(rcvflex rcvflex.c typedefs.h)
    add_executable(toflex toflex.c)
    add_executable(send2flx send2flx.c)
    add_executable(testdaa testdaa.c)

    install(TARGETS bin2s19 fromflex toflex)
endif()
