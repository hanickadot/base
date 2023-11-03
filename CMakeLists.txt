cmake_minimum_required(VERSION 3.14)

project(hana-base VERSION 1.0 LANGUAGES CXX)

if (PROJECT_IS_TOP_LEVEL) 
	list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
	
	if (APPLE)
		set(CMAKE_OSX_DEPLOYMENT_TARGET "13.3")
	endif() 
	
	include(colors)
	include(pedantic)
	
	option(BASE_COVERAGE "Enable code coverage" ON)
	
	if (BASE_COVERAGE)
		include(coverage)
		enable_coverage()
	else()
		message(STATUS "Test coverage measurement is OFF")
	endif()
	
	add_subdirectory(tests)
endif()

add_subdirectory(include)

add_executable(pokus pokus.cpp)
target_link_libraries(pokus base)
