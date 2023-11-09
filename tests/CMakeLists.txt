find_package(Catch2 3 REQUIRED)

#file(GLOB_RECURSE TESTS_SOURCES  LINK_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

list(APPEND TESTS_SOURCES bit-buffer.cpp chunk-of-bits.cpp base.cpp)

add_executable(test-runner ${TESTS_SOURCES})

target_link_libraries(test-runner PRIVATE Catch2::Catch2WithMain base)
target_compile_features(test-runner PUBLIC cxx_std_23)

add_custom_target(test test-runner --skip-benchmarks --colour-mode ansi "" DEPENDS test-runner)

if (LLVM_COVERAGE_ENABLED)
	coverage_report_after(test test-runner)
endif()