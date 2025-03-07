cmake_minimum_required(VERSION 3.16)

project(Reminder VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

set(PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Root source directory of client.")

find_package(Qt6 COMPONENTS Widgets Core Gui 
    Multimedia MultimediaWidgets 
    #WebEngineWidgets
    REQUIRED)
#find_package(Vulkan REQUIRED)

set(CMAKE_PREFIX_PATH ${Qt6Core_INCLUDE_DIRS}/../.. ${CMAKE_PREFIX_PATH})


set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} "${PROJECT_SOURCE_DIR}/ui")

set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")

file(GLOB_RECURSE  HEADERS 
    ${PROJECT_SOURCE_DIR}/include/*.h
    ${PROJECT_SOURCE_DIR}/include/*/*.h
)
file(GLOB_RECURSE  SOURCES 
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*/*.cpp
)
file(GLOB_RECURSE UIS ${PROJECT_SOURCE_DIR}/ui/*.ui)
file(GLOB_RECURSE RESOURCES 
    ${PROJECT_SOURCE_DIR}/res/*.qrc 
    ${PROJECT_SOURCE_DIR}/res/*.rc
    ${PROJECT_SOURCE_DIR}/res/*.png
    ${PROJECT_SOURCE_DIR}/res/*.ani
)

# place header and source files into group folders to help IDE's present the files in a logical manner
function(ASSIGN_SOURCE_GROUPS GROUP_NAME ROOT_FOLDER)
    foreach(FILE IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${FILE}")
            file(RELATIVE_PATH RELATIVE_SOURCE "${ROOT_FOLDER}" "${FILE}")
        else()
            set(RELATIVE_SOURCE "${FILE}")
        endif()
        get_filename_component(SOURCE_PATH "${RELATIVE_SOURCE}" PATH)
        string(REPLACE "/" "\\" SOURCE_PATH_MSVC "${SOURCE_PATH}")
        source_group("${GROUP_NAME}\\${SOURCE_PATH_MSVC}" FILES "${FILE}")
    endforeach()
endfunction(ASSIGN_SOURCE_GROUPS)

# enable folders for MSVC
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# group headers, source files, ui form files and resource files 
ASSIGN_SOURCE_GROUPS("Source Files" "${PROJECT_SOURCE_DIR}/src" ${SOURCES})
ASSIGN_SOURCE_GROUPS("Header Files" "${PROJECT_SOURCE_DIR}/include" ${HEADERS})
ASSIGN_SOURCE_GROUPS("Form Files" "${PROJECT_SOURCE_DIR}/ui" ${UIS})
ASSIGN_SOURCE_GROUPS("Resources Files" "${PROJECT_SOURCE_DIR}/res" ${RESOURCES})


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "debug postfix" FORCE)
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install" CACHE PATH "Install prefix" FORCE)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	message(STATUS "Windows verison: ${CMAKE_SYSTEM_VERSION}")
	if(MSVC)
		# This option is to enable the /MP switch for Visual Studio 2005 and above compilers
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        add_compile_options(/Zc:__cplusplus)
        add_compile_options(/permissive-)
	endif()
	add_definitions(-DNOMINMAX -DVK_USE_PLATFORM_WIN32_KHR)
endif()

add_executable(${PROJECT_NAME}
    ${UIS} 
    ${HEADERS}
    ${SOURCES} 
    ${RESOURCES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${Qt6Core_INCLUDE_DIRS} 
    ${Qt6Gui_INCLUDE_DIRS} 
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6Multimedia_INCLUDE_DIRS}
    ${Qt6MultimediaWidgets_INCLUDE_DIRS}
    #${Qt6WebEngineWidgets_INCLUDE_DIRS}
    #${Vulkan_INCLUDE_DIRS}
    "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core 
    Qt6::Gui    
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    #Qt6::WebEngineWidgets
    #Vulkan::Vulkan
)

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")