cmake_minimum_required(VERSION 3.15)

if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

if((CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "DOS") AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_OSX_ARCHITECTURES "")
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif()

project(parson VERSION 1.5.3 LANGUAGES C)

if(CMAKE_SYSTEM_NAME STREQUAL "DOS")
    if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
        set(CMAKE_INSTALL_LIBDIR "lib" CACHE PATH "Object code libraries")
    endif()
    if(CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom")
        get_filename_component(_parson_ow_cc_name "${CMAKE_C_COMPILER}" NAME)
        if(_parson_ow_cc_name MATCHES "^wcl(\\.exe)?$")
            if(NOT CMAKE_C_FLAGS MATCHES "(^| )-m[smlchf]( |$)")
                set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mm -zm -zc -zt=128")
            endif()
            if(NOT CMAKE_EXE_LINKER_FLAGS MATCHES "stack=")
                set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} option stack=16384")
            endif()
        endif()
        unset(_parson_ow_cc_name)
    endif()
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(PARSON_IS_TOP_LEVEL ON)
else()
    set(PARSON_IS_TOP_LEVEL OFF)
endif()

option(PARSON_UNICODE "Use UNICODE charset" OFF)
option(PARSON_LTO "Enable Link-Time Optimization (LTO)" OFF)
option(PARSON_SINGLE_THREADED "Use Single-threaded CRT" OFF)
option(PARSON_SINGLE_HEADER "Enable STB-style single header mode" OFF)
option(PARSON_BUILD_TESTING "Build parson test suite" ${PARSON_IS_TOP_LEVEL})
option(PARSON_INSTALL "Enable install and CPack targets" ${PARSON_IS_TOP_LEVEL})

set(PARSON_CHARSET "" CACHE STRING "Charset (UNICODE or ANSI)")
set(PARSON_CRT_LINKAGE "" CACHE STRING "MSVC CRT Linkage (Static or Shared)")
set(PARSON_MSVC_RUNTIME_CHECKS "" CACHE STRING "MSVC Runtime Checks (/RTC1, /RTCs, /RTCu)")

# Consume CI matrix variables without unused-cli warnings
set(_parson_unused_ci_vars
    "${CDD_CHARSET}"
    "${CDD_THREADING}"
    "${CDD_DEPS}"
    "${CDD_CRT_LINKAGE}"
    "${CDD_MSVC_RTC}"
    "${VCPKG_CRT_LINKAGE}"
    "${TARGET_ARCH}"
    "${PARSON_TESTING}"
    "${parson_TESTING}"
    "${FETCHCONTENT_SOURCE_DIR_AUTO-WIN-MSVC}"
    "${FETCHCONTENT_SOURCE_DIR_AUTO_WIN_MSVC}"
    "${FETCHCONTENT_SOURCE_DIR_C-ABSTRACT-HTTP}"
    "${FETCHCONTENT_SOURCE_DIR_C_ABSTRACT_HTTP}"
    "${FETCHCONTENT_SOURCE_DIR_C-ORM}"
    "${FETCHCONTENT_SOURCE_DIR_C_ORM}"
    "${FETCHCONTENT_SOURCE_DIR_C-STR-SPAN}"
    "${FETCHCONTENT_SOURCE_DIR_C_STR_SPAN}"
    "${FETCHCONTENT_SOURCE_DIR_C89STRINGUTILS}"
    "${FETCHCONTENT_SOURCE_DIR_CDD-C}"
    "${FETCHCONTENT_SOURCE_DIR_CDD_C}"
    "${FETCHCONTENT_SOURCE_DIR_CFS}"
    "${FETCHCONTENT_SOURCE_DIR_C_FS}"
    "${FETCHCONTENT_SOURCE_DIR_PARSON}"
)
unset(_parson_unused_ci_vars)

if(DEFINED CDD_CHARSET AND NOT CDD_CHARSET STREQUAL "")
    set(PARSON_CHARSET "${CDD_CHARSET}")
endif()
if(PARSON_CHARSET STREQUAL "UNICODE" OR PARSON_CHARSET STREQUAL "Unicode")
    set(PARSON_UNICODE ON)
elseif(PARSON_CHARSET STREQUAL "ANSI")
    set(PARSON_UNICODE OFF)
endif()

if(DEFINED CDD_THREADING AND NOT CDD_THREADING STREQUAL "")
    if(CDD_THREADING STREQUAL "OFF" OR CDD_THREADING STREQUAL "Single-threaded")
        set(PARSON_SINGLE_THREADED ON)
    else()
        set(PARSON_SINGLE_THREADED OFF)
    endif()
endif()

if(DEFINED CDD_CRT_LINKAGE AND NOT CDD_CRT_LINKAGE STREQUAL "")
    set(PARSON_CRT_LINKAGE "${CDD_CRT_LINKAGE}")
elseif(DEFINED VCPKG_CRT_LINKAGE AND NOT VCPKG_CRT_LINKAGE STREQUAL "")
    set(PARSON_CRT_LINKAGE "${VCPKG_CRT_LINKAGE}")
endif()

if(DEFINED CDD_MSVC_RTC AND NOT CDD_MSVC_RTC STREQUAL "")
    if(CDD_MSVC_RTC STREQUAL "OFF")
        set(PARSON_MSVC_RUNTIME_CHECKS "")
    else()
        set(PARSON_MSVC_RUNTIME_CHECKS "${CDD_MSVC_RTC}")
    endif()
endif()

set(_parson_rtc_flag "")
if(MSVC AND NOT PARSON_MSVC_RUNTIME_CHECKS STREQUAL "" AND NOT PARSON_MSVC_RUNTIME_CHECKS STREQUAL "OFF")
    if(PARSON_MSVC_RUNTIME_CHECKS MATCHES "^/")
        set(_parson_rtc_flag "${PARSON_MSVC_RUNTIME_CHECKS}")
    else()
        set(_parson_rtc_flag "/${PARSON_MSVC_RUNTIME_CHECKS}")
    endif()
endif()

# Respect parent project's CMAKE_MSVC_RUNTIME_LIBRARY when consumed via FetchContent_MakeAvailable
if(MSVC AND PARSON_IS_TOP_LEVEL)
    if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY OR CMAKE_MSVC_RUNTIME_LIBRARY STREQUAL "")
        if(PARSON_CRT_LINKAGE STREQUAL "Static" OR PARSON_CRT_LINKAGE STREQUAL "static" OR PARSON_SINGLE_THREADED)
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        elseif(PARSON_CRT_LINKAGE STREQUAL "Shared" OR PARSON_CRT_LINKAGE STREQUAL "shared")
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        elseif(BUILD_SHARED_LIBS)
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        else()
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()
    endif()
endif()

set(_parson_enable_lto OFF)
if(PARSON_LTO OR CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _parson_ipo_supported)
    if(_parson_ipo_supported)
        set(_parson_enable_lto ON)
        if(MSVC AND PARSON_IS_TOP_LEVEL)
            string(REPLACE "/ZI" "/Zi" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
        endif()
    endif()
endif()

if(MSVC AND _parson_rtc_flag AND PARSON_IS_TOP_LEVEL)
    string(REGEX REPLACE "/RTC[1suc]+" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REGEX REPLACE "/O[12x]" "/Od" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
endif()

set(_parson_strict_c_flags "")
if(MSVC)
    list(APPEND _parson_strict_c_flags /W4 /WX)
    if(_parson_rtc_flag)
        list(APPEND _parson_strict_c_flags "$<$<CONFIG:Debug>:${_parson_rtc_flag}>")
    endif()
elseif(WATCOM OR CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom")
    list(APPEND _parson_strict_c_flags -wx -we)
else()
    list(APPEND _parson_strict_c_flags
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
        -Wundef
        -Wmissing-prototypes
        -Wmissing-declarations
    )
endif()

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
    if(PARSON_UNICODE)
        target_compile_definitions(parson INTERFACE UNICODE _UNICODE)
    endif()
else()
    add_library(parson parson.c)
    if(NOT TARGET parson::parson)
        add_library(parson::parson ALIAS parson)
    endif()
    target_include_directories(parson PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    if(PARSON_UNICODE)
        target_compile_definitions(parson PUBLIC UNICODE _UNICODE)
    endif()
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(parson
            PRIVATE PARSON_BUILD_SHARED
            INTERFACE PARSON_SHARED
        )
    endif()

    set_target_properties(parson PROPERTIES
        PUBLIC_HEADER "parson.h"
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION}
    )

    if(_parson_enable_lto)
        set_target_properties(parson PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()

    target_compile_options(parson PRIVATE ${_parson_strict_c_flags})
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

    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/tests" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

    add_executable(parson_tests tests/tests.c)
    target_link_libraries(parson_tests PRIVATE greatest::greatest)
    target_include_directories(parson_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(parson_tests PRIVATE TESTS_MAIN PARSON_SINGLE_HEADER)
    if(PARSON_UNICODE)
        target_compile_definitions(parson_tests PRIVATE UNICODE _UNICODE)
    endif()
    if(_parson_enable_lto)
        set_target_properties(parson_tests PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    target_compile_options(parson_tests PRIVATE ${_parson_strict_c_flags})
    if(EMSCRIPTEN)
        target_link_options(parson_tests PRIVATE "-sNODERAWFS=1" "-sSTACK_SIZE=5MB")
    endif()
    add_test(NAME parson_test COMMAND parson_tests)
    add_custom_command(TARGET parson_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_BINARY_DIR}/tests
    )

    if(NOT PARSON_SINGLE_HEADER)
        add_executable(parson_lib_tests tests/tests.c)
        target_link_libraries(parson_lib_tests PRIVATE parson::parson greatest::greatest)
        target_compile_definitions(parson_lib_tests PRIVATE TESTS_MAIN)
        if(_parson_enable_lto)
            set_target_properties(parson_lib_tests PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
        endif()
        target_compile_options(parson_lib_tests PRIVATE ${_parson_strict_c_flags})
        if(EMSCRIPTEN)
            target_link_options(parson_lib_tests PRIVATE "-sNODERAWFS=1" "-sSTACK_SIZE=5MB")
        endif()
        add_test(NAME parson_lib_test COMMAND parson_lib_tests)
    endif()
endif()

if(PARSON_INSTALL)
    if(PARSON_SINGLE_HEADER)
        install(
            TARGETS parson
            EXPORT parsonTargets
        )
        install(
            FILES parson.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    else()
        install(
            TARGETS parson
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
