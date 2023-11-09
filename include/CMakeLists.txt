add_library(base INTERFACE)

target_include_directories(base INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(base INTERFACE cxx_std_23)

target_sources(base INTERFACE FILE_SET headers TYPE HEADERS FILES 
	base/bit-buffer.hpp
	base/concepts.hpp
	base/encodings.hpp
) 

set_target_properties(base PROPERTIES 
  LINKER_LANGUAGE CXX
  VERSION ${hana-base_VERSION}
  SOVERSION 1.0
)