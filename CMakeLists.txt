cmake_minimum_required(VERSION 3.15)
project(parson VERSION 1.5.3 LANGUAGES C)

option(PARSON_UNICODE "Use UNICODE charset" OFF)
option(PARSON_LTO "Enable Link-Time Optimization (LTO)" OFF)
option(PARSON_SINGLE_THREADED "Use Single-threaded CRT" OFF)

set(PARSON_MSVC_RUNTIME_CHECKS "" CACHE STRING "MSVC Runtime Checks (/RTC1, /RTCs, /RTCu)")

if(MSVC)
    if(PARSON_SINGLE_THREADED)
        # MSVC removed single-threaded CRT in VS2005, but for compatibility options:
        # We handle MT vs MD via standard MSVC_RUNTIME_LIBRARY
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        if(BUILD_SHARED_LIBS)
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        else()
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()
    endif()
endif()

if(PARSON_UNICODE)
    add_compile_definitions(UNICODE _UNICODE)
endif()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(GNUInstallDirs)

add_library(parson parson.c)
target_include_directories(parson PUBLIC $<INSTALL_INTERFACE:include>)

if(PARSON_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported)
    if(ipo_supported)
        set_target_properties(parson PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

if(MSVC)
    if(PARSON_MSVC_RUNTIME_CHECKS)
        target_compile_options(parson PRIVATE ${PARSON_MSVC_RUNTIME_CHECKS})
    endif()
endif()

set_target_properties(parson PROPERTIES PUBLIC_HEADER "parson.h")
set_target_properties(parson PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(parson PROPERTIES SOVERSION ${PROJECT_VERSION})

if(BUILD_TESTING)
    enable_testing()
    add_executable(parson_tests tests.c)
    target_compile_definitions(parson_tests PRIVATE TESTS_MAIN)
    target_link_libraries(parson_tests PRIVATE parson)
    
    if(PARSON_LTO AND ipo_supported)
        set_target_properties(parson_tests PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    if(MSVC AND PARSON_MSVC_RUNTIME_CHECKS)
        target_compile_options(parson_tests PRIVATE ${PARSON_MSVC_RUNTIME_CHECKS})
    endif()
    
    add_test(NAME parson_test COMMAND parson_tests)
    add_custom_command(TARGET parson_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_BINARY_DIR}/tests)
endif()

install(
    TARGETS parson
    EXPORT parsonTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT shlib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT parsonTargets
    FILE parsonConfig.cmake
    NAMESPACE parson::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

include(CPack)


