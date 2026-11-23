cmake_minimum_required(VERSION 3.15...3.31)

if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
endif()

if((CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "DOS") AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_OSX_ARCHITECTURES "")
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif()

project(parson VERSION 1.5.3 LANGUAGES C)

if(CMAKE_SYSTEM_NAME STREQUAL "DOS" AND NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR "lib" CACHE PATH "Object code libraries")
endif()

if(DEFINED PROJECT_IS_TOP_LEVEL)
    set(PARSON_IS_TOP_LEVEL "${PROJECT_IS_TOP_LEVEL}")
elseif(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(PARSON_IS_TOP_LEVEL ON)
else()
    set(PARSON_IS_TOP_LEVEL OFF)
endif()

# Shared c-ci matrix overrides
set(CDD_CHARSET "" CACHE STRING "c-ci matrix override for character set")
set(CDD_CRT_LINKAGE "" CACHE STRING "c-ci matrix override for MSVC CRT linkage")
set(CDD_DEPS "" CACHE STRING "c-ci matrix override for dependency resolution")
set(CDD_MSVC_RTC "" CACHE STRING "c-ci matrix override for MSVC runtime checks")
set(CDD_THREADING "" CACHE STRING "c-ci matrix override for threading mode")
mark_as_advanced(CDD_CHARSET CDD_CRT_LINKAGE CDD_DEPS CDD_MSVC_RTC CDD_THREADING)

option(PARSON_UNICODE "Use UNICODE charset" OFF)
option(PARSON_LTO "Enable Link-Time Optimization (LTO)" OFF)
option(PARSON_SINGLE_THREADED "Use Single-threaded mode" OFF)
option(PARSON_SINGLE_HEADER "Enable STB-style single header mode" OFF)
option(PARSON_BUILD_TESTING "Build parson test suite" ${PARSON_IS_TOP_LEVEL})
option(PARSON_INSTALL "Enable install and CPack targets" ${PARSON_IS_TOP_LEVEL})

set(PARSON_CHARSET "" CACHE STRING "Charset (UNICODE or ANSI)")
set(PARSON_THREADING "" CACHE STRING "Threading mode (Multi-threaded or Single-threaded)")
set(PARSON_CRT_LINKAGE "" CACHE STRING "MSVC CRT Linkage (Static or Shared)")
set(PARSON_MSVC_RUNTIME_CHECKS "" CACHE STRING "MSVC Runtime Checks (/RTC1, /RTCs, /RTCu)")

if(DEFINED PARSON_TESTING)
    set(PARSON_BUILD_TESTING "${PARSON_TESTING}")
elseif(DEFINED parson_TESTING)
    set(PARSON_BUILD_TESTING "${parson_TESTING}")
endif()

# Charset resolution (UNICODE vs ANSI)
set(_parson_charset "${PARSON_CHARSET}")
if(NOT "${CDD_CHARSET}" STREQUAL "")
    set(_parson_charset "${CDD_CHARSET}")
endif()
string(TOUPPER "${_parson_charset}" _parson_charset_upper)
if(_parson_charset_upper STREQUAL "UNICODE")
    set(_parson_use_unicode ON)
elseif(_parson_charset_upper STREQUAL "ANSI" OR _parson_charset_upper STREQUAL "MBCS")
    set(_parson_use_unicode OFF)
else()
    set(_parson_use_unicode ${PARSON_UNICODE})
endif()

# Threading resolution (Multi-threaded vs Single-threaded)
set(_parson_threading "${PARSON_THREADING}")
if(NOT "${CDD_THREADING}" STREQUAL "")
    set(_parson_threading "${CDD_THREADING}")
endif()
string(TOUPPER "${_parson_threading}" _parson_threading_upper)
if(_parson_threading_upper STREQUAL "OFF" OR _parson_threading_upper STREQUAL "SINGLE-THREADED" OR _parson_threading_upper STREQUAL "SINGLE")
    set(_parson_single_threaded ON)
elseif(_parson_threading_upper STREQUAL "ON" OR _parson_threading_upper STREQUAL "MULTI-THREADED" OR _parson_threading_upper STREQUAL "MULTI")
    set(_parson_single_threaded OFF)
else()
    set(_parson_single_threaded ${PARSON_SINGLE_THREADED})
endif()

if(NOT _parson_single_threaded AND NOT WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "DOS" AND NOT EMSCRIPTEN)
    find_package(Threads QUIET)
endif()

# MSVC CRT Linkage resolution (Static /MT,/MTd vs Shared /MD,/MDd)
set(_parson_crt "${PARSON_CRT_LINKAGE}")
if(NOT "${CDD_CRT_LINKAGE}" STREQUAL "")
    set(_parson_crt "${CDD_CRT_LINKAGE}")
elseif(DEFINED VCPKG_CRT_LINKAGE AND NOT "${VCPKG_CRT_LINKAGE}" STREQUAL "")
    set(_parson_crt "${VCPKG_CRT_LINKAGE}")
endif()
string(TOUPPER "${_parson_crt}" _parson_crt_upper)

set(_parson_msvc_runtime_library "")
set(_parson_msvc_debug_format "")
if(MSVC)
    if(DEFINED CMAKE_MSVC_RUNTIME_LIBRARY AND NOT "${CMAKE_MSVC_RUNTIME_LIBRARY}" STREQUAL "")
        set(_parson_msvc_runtime_library "${CMAKE_MSVC_RUNTIME_LIBRARY}")
    elseif(PARSON_IS_TOP_LEVEL OR NOT "${_parson_crt_upper}" STREQUAL "")
        if(_parson_crt_upper STREQUAL "STATIC" OR _parson_crt_upper STREQUAL "MT" OR _parson_single_threaded)
            set(_parson_msvc_runtime_library "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        else()
            set(_parson_msvc_runtime_library "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif()
    endif()

    if(POLICY CMP0141)
        if(DEFINED CMAKE_MSVC_DEBUG_INFORMATION_FORMAT AND NOT "${CMAKE_MSVC_DEBUG_INFORMATION_FORMAT}" STREQUAL "")
            set(_parson_msvc_debug_format "${CMAKE_MSVC_DEBUG_INFORMATION_FORMAT}")
        elseif(PARSON_IS_TOP_LEVEL)
            set(_parson_msvc_debug_format "Embedded")
        endif()
    endif()
endif()

# MSVC Runtime Checks (/RTC1, /RTCs, /RTCu)
set(_parson_rtc "${PARSON_MSVC_RUNTIME_CHECKS}")
if(NOT "${CDD_MSVC_RTC}" STREQUAL "")
    set(_parson_rtc "${CDD_MSVC_RTC}")
endif()
set(_parson_rtc_flag "")
if(MSVC AND NOT "${_parson_rtc}" STREQUAL "" AND NOT "${_parson_rtc}" STREQUAL "OFF")
    string(REGEX REPLACE "^/" "" _parson_rtc_clean "${_parson_rtc}")
    if(_parson_rtc_clean STREQUAL "1" OR _parson_rtc_clean STREQUAL "s" OR _parson_rtc_clean STREQUAL "u")
        set(_parson_rtc_flag "/RTC${_parson_rtc_clean}")
    else()
        set(_parson_rtc_flag "/${_parson_rtc_clean}")
    endif()
    if(PARSON_IS_TOP_LEVEL)
        # Prevent CMake's default Debug /RTC1 or Release /O2 from conflicting with explicit /RTC* flag (D8016)
        foreach(_flag_var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS_MINSIZEREL)
            if(DEFINED ${_flag_var})
                string(REGEX REPLACE "/RTC[1csu]+" "" ${_flag_var} "${${_flag_var}}")
                string(REGEX REPLACE "/O[12x]" "/Od" ${_flag_var} "${${_flag_var}}")
            endif()
        endforeach()
    endif()
endif()

# Link-Time Optimization (LTO / IPO)
set(_parson_enable_lto OFF)
if(PARSON_LTO OR CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _parson_ipo_supported)
    if(_parson_ipo_supported)
        set(_parson_enable_lto ON)
    endif()
endif()

# Target-scoped strict compiler and linker flags via INTERFACE target
add_library(parson_internal_flags INTERFACE)
if(MSVC)
    target_compile_options(parson_internal_flags INTERFACE /W4 /WX)
    if(_parson_rtc_flag)
        target_compile_options(parson_internal_flags INTERFACE "$<$<CONFIG:Debug>:${_parson_rtc_flag}>")
    endif()
elseif(WATCOM OR CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom")
    target_compile_options(parson_internal_flags INTERFACE -wx -we)
    get_filename_component(_parson_ow_cc_name "${CMAKE_C_COMPILER}" NAME)
    if(_parson_ow_cc_name MATCHES "^wcl(\.exe)?$" AND NOT CMAKE_C_FLAGS MATCHES "(^| )-m[smlchf]( |$)")
        target_compile_options(parson_internal_flags INTERFACE -mm -zm -zc -zt=128)
    endif()
    unset(_parson_ow_cc_name)
else()
    target_compile_options(parson_internal_flags INTERFACE
        -ansi
        -pedantic
        -Wall
        -Wextra
        -Werror
        -Wformat=2
        -Wshadow
        -Wwrite-strings
        -Wstrict-prototypes
        -Wold-style-definition
        -Wredundant-decls
        -Wnested-externs
        -Wconversion
        -Wsign-conversion
        -Wcast-qual
        -Wcast-align
        -Wpointer-arith
        -Wdeclaration-after-statement
        -Wundef
        -Wmissing-prototypes
        -Wmissing-declarations
    )
endif()

function(parson_configure_target target_name)
    if(PARSON_IS_TOP_LEVEL)
        set_target_properties(${target_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        )
    endif()

    if("c_std_90" IN_LIST CMAKE_C_COMPILE_FEATURES)
        set_target_properties(${target_name} PROPERTIES
            C_STANDARD 90
            C_STANDARD_REQUIRED ON
            C_EXTENSIONS OFF
        )
    endif()

    if(NOT "${_parson_msvc_runtime_library}" STREQUAL "")
        set_property(TARGET ${target_name} PROPERTY MSVC_RUNTIME_LIBRARY "${_parson_msvc_runtime_library}")
    endif()

    if(NOT "${_parson_msvc_debug_format}" STREQUAL "")
        set_property(TARGET ${target_name} PROPERTY MSVC_DEBUG_INFORMATION_FORMAT "${_parson_msvc_debug_format}")
    endif()

    if(_parson_enable_lto)
        set_property(TARGET ${target_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()

    if(NOT _parson_single_threaded)
        if(EMSCRIPTEN)
            target_compile_options(${target_name} PRIVATE "-pthread")
            target_link_options(${target_name} PRIVATE "-pthread")
        elseif(TARGET Threads::Threads)
            target_link_libraries(${target_name} PRIVATE Threads::Threads)
        endif()
    endif()

    target_link_libraries(${target_name} PRIVATE $<BUILD_INTERFACE:parson_internal_flags>)
endfunction()

include(GNUInstallDirs)

if(PARSON_SINGLE_HEADER)
    add_library(parson INTERFACE)
    if(NOT TARGET parson::parson)
        add_library(parson::parson ALIAS parson)
    endif()
    target_include_directories(parson INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    target_compile_definitions(parson INTERFACE PARSON_SINGLE_HEADER)
    if(_parson_use_unicode)
        target_compile_definitions(parson INTERFACE UNICODE _UNICODE)
    elseif(WIN32 AND _parson_charset_upper STREQUAL "MBCS")
        target_compile_definitions(parson INTERFACE _MBCS)
    endif()
else()
    add_library(parson parson.c)
    if(NOT TARGET parson::parson)
        add_library(parson::parson ALIAS parson)
    endif()
    parson_configure_target(parson)

    target_include_directories(parson PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    if(_parson_use_unicode)
        target_compile_definitions(parson PUBLIC UNICODE _UNICODE)
    elseif(WIN32 AND _parson_charset_upper STREQUAL "MBCS")
        target_compile_definitions(parson PRIVATE _MBCS)
    endif()
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(parson
            PRIVATE PARSON_BUILD_SHARED
            INTERFACE PARSON_SHARED
        )
        set_target_properties(parson PROPERTIES
            C_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN ON
        )
    endif()

    set_target_properties(parson PROPERTIES
        PUBLIC_HEADER "parson.h"
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endif()

if((BUILD_TESTING OR PARSON_BUILD_TESTING) AND PARSON_IS_TOP_LEVEL)
    include(CTest)
    include(FetchContent)

    if(DEFINED FETCHCONTENT_SOURCE_DIR_GREATEST AND EXISTS "${FETCHCONTENT_SOURCE_DIR_GREATEST}/greatest.h")
        if(NOT TARGET greatest::greatest)
            add_library(greatest INTERFACE)
            target_include_directories(greatest INTERFACE "${FETCHCONTENT_SOURCE_DIR_GREATEST}")
            add_library(greatest::greatest ALIAS greatest)
        endif()
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../greatest/CMakeLists.txt")
        FetchContent_Declare(
            greatest
            SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../greatest"
        )
        set(GREATEST_BUILD_TESTING OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(greatest)
    elseif(CMAKE_GENERATOR STREQUAL "Watcom WMake")
        set(_parson_greatest_dir "${CMAKE_CURRENT_BINARY_DIR}/_deps/greatest-header")
        file(MAKE_DIRECTORY "${_parson_greatest_dir}")
        if(NOT EXISTS "${_parson_greatest_dir}/greatest.h")
            file(DOWNLOAD
                "https://raw.githubusercontent.com/SamuelMarks/greatest/master/greatest.h"
                "${_parson_greatest_dir}/greatest.h"
                TLS_VERIFY ON
                STATUS _parson_dl_status
            )
            list(GET _parson_dl_status 0 _parson_dl_code)
            if(NOT _parson_dl_code EQUAL 0)
                message(FATAL_ERROR "Failed to download greatest.h: ${_parson_dl_status}")
            endif()
        endif()
        if(NOT TARGET greatest::greatest)
            add_library(greatest INTERFACE)
            target_include_directories(greatest INTERFACE "${_parson_greatest_dir}")
            add_library(greatest::greatest ALIAS greatest)
        endif()
    else()
        FetchContent_Declare(
            greatest
            GIT_REPOSITORY https://github.com/SamuelMarks/greatest.git
            GIT_TAG master
        )
        set(GREATEST_BUILD_TESTING OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(greatest)
    endif()

    function(parson_configure_test_executable target_name)
        parson_configure_target(${target_name})
        if(EMSCRIPTEN)
            target_link_options(${target_name} PRIVATE "-sNODERAWFS=1" "-sSTACK_SIZE=5MB")
        elseif(CMAKE_SYSTEM_NAME STREQUAL "DOS" AND CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom")
            get_filename_component(_parson_ow_cc_name "${CMAKE_C_COMPILER}" NAME)
            if(_parson_ow_cc_name MATCHES "^wcl(\.exe)?$" AND NOT CMAKE_EXE_LINKER_FLAGS MATCHES "stack=")
                target_link_options(${target_name} PRIVATE "option" "stack=16384")
            endif()
            unset(_parson_ow_cc_name)
        endif()
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/tests"
            "${CMAKE_CURRENT_BINARY_DIR}/tests"
        )
    endfunction()

    add_executable(parson_tests tests/tests.c)
    parson_configure_test_executable(parson_tests)
    target_link_libraries(parson_tests PRIVATE greatest::greatest)
    target_include_directories(parson_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(parson_tests PRIVATE TESTS_MAIN PARSON_SINGLE_HEADER)
    if(_parson_use_unicode)
        target_compile_definitions(parson_tests PRIVATE UNICODE _UNICODE)
    elseif(WIN32 AND _parson_charset_upper STREQUAL "MBCS")
        target_compile_definitions(parson_tests PRIVATE _MBCS)
    endif()
    add_test(NAME parson_test COMMAND parson_tests)

    if(NOT PARSON_SINGLE_HEADER)
        add_executable(parson_lib_tests tests/tests.c)
        parson_configure_test_executable(parson_lib_tests)
        target_link_libraries(parson_lib_tests PRIVATE parson::parson greatest::greatest)
        target_compile_definitions(parson_lib_tests PRIVATE TESTS_MAIN)
        add_test(NAME parson_lib_test COMMAND parson_lib_tests)
    endif()
endif()

if(PARSON_INSTALL)
    if(PARSON_SINGLE_HEADER)
        install(
            TARGETS parson parson_internal_flags
            EXPORT parsonTargets
        )
        install(
            FILES parson.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    else()
        install(
            TARGETS parson parson_internal_flags
            EXPORT parsonTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT shlib
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    endif()

    install(
        EXPORT parsonTargets
        FILE parsonConfig.cmake
        NAMESPACE parson::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    include(CPack)
endif()
