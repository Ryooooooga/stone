add_library(fmt STATIC
	fmt/src/format.cc
	fmt/src/posix.cc
)

target_include_directories(fmt
	PUBLIC fmt/include
)
