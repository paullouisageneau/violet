cmake_minimum_required(VERSION 3.7)
project(violet
	VERSION 0.5.0
	LANGUAGES C)

set(PROJECT_DESCRIPTION "Lightweight STUN/TURN server")

# Options
option(USE_SYSTEM_JUICE "Use system libjuice" OFF)
option(WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

set(CMAKE_C_STANDARD 11)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

if(WIN32)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	if(MSVC)
		add_definitions(-DNOMINMAX)
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
		add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
	endif()
endif()

set(VIOLET_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/daemon.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/options.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/utils.c
)

set(VIOLET_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/src/daemon.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/options.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/utils.h
)

add_executable(violet ${VIOLET_HEADERS} ${VIOLET_SOURCES})
target_compile_definitions(violet PRIVATE VIOLET_VERSION="${PROJECT_VERSION}")

if(USE_SYSTEM_JUICE)
	find_package(LibJuice REQUIRED)
	target_link_libraries(violet PRIVATE LibJuice::LibJuice)
else()
	option(NO_TESTS "Disable tests for libjuice" ON)
	option(NO_EXAMPLES "Disable examples for libjuice" ON)
	add_subdirectory(deps/libjuice EXCLUDE_FROM_ALL)
	target_link_libraries(violet PRIVATE LibJuice::LibJuiceStatic)
endif()

install(TARGETS violet RUNTIME DESTINATION bin)

target_compile_options(violet PRIVATE -Wall -Wextra)

if(WARNINGS_AS_ERRORS)
	target_compile_options(violet PRIVATE -Werror)
endif()

