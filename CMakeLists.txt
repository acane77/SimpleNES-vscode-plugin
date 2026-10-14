cmake_minimum_required(VERSION 3.12)
project(SimpleNES)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(BUILD_STATIC FALSE CACHE STRING "Set this to link external libraries statically")

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/;${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}")

file(GLOB SOURCES
    "${PROJECT_SOURCE_DIR}/main.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/APU/*.cpp"
)

file(GLOB VENDOR_SOURCES
    "${PROJECT_SOURCE_DIR}/vendor/miniaudio/*.c"
)

function(define_file_basename_for_sources targetname)
    get_target_property(source_files "${targetname}" SOURCES)
    foreach(sourcefile ${source_files})
        get_property(defs SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS)
        get_filename_component(basename "${sourcefile}" NAME)
        list(APPEND defs "__FILENAME__=\"${basename}\"")
        set_property(
            SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS ${defs})
    endforeach()
endfunction()

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/vendor/miniaudio/"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(EMSCRIPTEN)
    list(APPEND SOURCES "${PROJECT_SOURCE_DIR}/src/wasm/WasmHost.cpp")
    set_source_files_properties(${VENDOR_SOURCES} PROPERTIES LANGUAGE C)
    add_executable(SimpleNES ${SOURCES} ${VENDOR_SOURCES})
    set_property(TARGET SimpleNES PROPERTY CXX_STANDARD 17)
    set_property(TARGET SimpleNES PROPERTY CXX_STANDARD_REQUIRED ON)
    target_link_options(SimpleNES PRIVATE
        "SHELL:-sMODULARIZE=1"
        "SHELL:-sEXPORT_NAME=createSimpleNES"
        "SHELL:-sSINGLE_FILE=1"
        "SHELL:-sENVIRONMENT=web"
        "SHELL:-sALLOW_MEMORY_GROWTH=1"
        "SHELL:-sINITIAL_MEMORY=67108864"
        "SHELL:-sEXPORTED_FUNCTIONS=[_nes_load_rom,_nes_set_key,_nes_release_keys,_malloc,_free,_ma_device_process_pcm_frames_playback__webaudio,_ma_device_process_pcm_frames_capture__webaudio,_ma_device__on_notification_unlocked]"
        "SHELL:-sEXPORTED_RUNTIME_METHODS=[HEAPU8]"
        "SHELL:-sEXIT_RUNTIME=0"
    )
    set_target_properties(SimpleNES PROPERTIES OUTPUT_NAME "SimpleNES" SUFFIX ".js")
else()
    file(COPY keybindings.conf DESTINATION .)

    if (BUILD_STATIC)
        set(SFML_STATIC_LIBRARIES TRUE)
        if(CMAKE_COMPILER_IS_GNUCXX)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
        endif()
    endif()

    if (SFML_OS_WINDOWS AND SFML_COMPILER_MSVC)
        find_package(SFML 2 COMPONENTS main audio graphics window system REQUIRED)
    else()
        find_package(SFML 2 COMPONENTS audio graphics window system REQUIRED)
    endif()

    if(SFML_FOUND)
            include_directories(${SFML_INCLUDE_DIR})
    else()
            set(SFML_ROOT "" CACHE PATH "SFML top-level directory")
            message("\nSFML directory not found. Set SFML_ROOT to SFML's top-level path (containing \"include\" and \"lib\" directories).")
            message("Make sure the SFML libraries with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
    endif()

    add_executable(SimpleNES ${SOURCES} ${VENDOR_SOURCES})
    target_link_libraries(SimpleNES PRIVATE ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
    set_property(TARGET SimpleNES PROPERTY CXX_STANDARD 11)
    set_property(TARGET SimpleNES PROPERTY CXX_STANDARD_REQUIRED ON)
    install(TARGETS SimpleNES RUNTIME DESTINATION bin)
endif()

define_file_basename_for_sources(SimpleNES)
