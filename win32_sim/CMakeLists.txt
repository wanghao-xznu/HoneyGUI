cmake_minimum_required (VERSION 3.10)




set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS ON) 


set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS ON)
include(${CMAKE_SOURCE_DIR}/config.cmake)

project(HoneyGUI)
add_executable(gui main.c)
set_target_properties(gui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)

set(GUI_BASE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
message("HoneyGUI base path ${GUI_BASE_PATH}")

add_subdirectory(../ HoneyGUI.out)
if(CONFIG_LVGL)

    add_subdirectory(port//lvgl_port)
        list(APPEND gui_sources 
			${lvgl_port_src}
			)

    list(APPEND gui_headers
    ${lvgl_port_inc}
    )

endif()
    add_subdirectory(port//realgui_port)
    list(APPEND gui_sources 
			${realgui_port_src}
			)

    list(APPEND gui_headers
    ${realgui_port_inc}
    )




target_sources(gui PRIVATE ${gui_sources})

target_include_directories(gui PUBLIC ${gui_headers})


set(LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/RTE/SDL2-2.26.0/x86_64-w64-mingw32/lib/")


add_library(LibFirst STATIC IMPORTED)
add_library(LibSecond STATIC IMPORTED)


set_target_properties(LibFirst PROPERTIES IMPORTED_LOCATION "${LIBRARY_PATH}/libSDL2main.a")
set_target_properties(LibSecond PROPERTIES IMPORTED_LOCATION "${LIBRARY_PATH}/libSDL2.a")



target_link_libraries(gui mingw32 opengl32 LibFirst LibSecond setupapi
dinput8 dxguid dxerr8 user32 gdi32 winmm imm32 ole32 oleaut32 shell32 setupapi version uuid)



add_custom_command(
    TARGET gui PRE_BUILD
    COMMAND taskkill /F /IM gui.exe /T >nul 2>&1|| exit 0
    COMMENT "Attempting to close any running instance of gui.exe..."
)



add_custom_target(menuconfig
    COMMAND ${CMAKE_COMMAND} -E echo "Running menuconfig"
    COMMAND kconfig-mconf Kconfig  
    COMMAND ${CMAKE_COMMAND} -E echo "Generating config.cmake"
    COMMAND python ${CMAKE_SOURCE_DIR}/parse_kconfig.py ${CMAKE_SOURCE_DIR}/.config ${CMAKE_SOURCE_DIR}/config.cmake
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running menuconfig and generating config.cmake"
    COMMAND ${CMAKE_COMMAND} -E echo "Removing CMakeCache.txt"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/CMakeCache.txt"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/Makefile"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    COMMAND ${CMAKE_COMMAND} -E echo "Reconfiguring project with CMake"
    COMMAND ${CMAKE_COMMAND} -G "MinGW Makefiles" -S "${CMAKE_SOURCE_DIR}" -B "${CMAKE_BINARY_DIR}"
    VERBATIM
)

set(CUSTOM_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/default.ld")


target_link_options(gui PRIVATE "-T${CUSTOM_LINKER_SCRIPT}")
