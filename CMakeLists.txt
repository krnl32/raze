cmake_minimum_required(VERSION 3.30)

project(RAZE
	VERSION 1.0.0
	LANGUAGES C
)

set(RAZE_TARGET_NAME raze)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Function to apply compile options
function(set_compile_options_for_target target)
	if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
		target_compile_options(${target} PRIVATE
			# Debug Flags
			$<$<CONFIG:Debug>:-ggdb3>
			$<$<CONFIG:Debug>:-Og>
			$<$<CONFIG:Debug>:-O0>
			$<$<CONFIG:Debug>:-DDEBUG>
			$<$<CONFIG:Debug>:-fno-omit-frame-pointer>

			# Basic warnings
			$<$<CONFIG:Debug>:-Wall>
			$<$<CONFIG:Debug>:-Wextra>

			# Stricter warnings
			$<$<CONFIG:Debug>:-Wpedantic>
			$<$<CONFIG:Debug>:-Werror>
			$<$<CONFIG:Debug>:-Wconversion>
			$<$<CONFIG:Debug>:-Wsign-conversion>
			$<$<CONFIG:Debug>:-Wcast-align>
			$<$<CONFIG:Debug>:-Wcast-qual>
			$<$<CONFIG:Debug>:-Wshadow>
			$<$<CONFIG:Debug>:-Wfloat-equal>
			$<$<CONFIG:Debug>:-Wundef>
			$<$<CONFIG:Debug>:-Wwrite-strings>
			$<$<CONFIG:Debug>:-Wstrict-prototypes>
			$<$<CONFIG:Debug>:-Wmissing-prototypes>
			$<$<CONFIG:Debug>:-Wmissing-declarations>
			$<$<CONFIG:Debug>:-Wstrict-overflow=5>
			$<$<CONFIG:Debug>:-Wuninitialized>
			$<$<CONFIG:Debug>:-Wreturn-type>
			$<$<CONFIG:Debug>:-Wswitch-default>
			$<$<CONFIG:Debug>:-Wswitch-enum>
			$<$<CONFIG:Debug>:-Wimplicit-function-declaration>
			$<$<CONFIG:Debug>:-Wunreachable-code>
			$<$<CONFIG:Debug>:-Wdouble-promotion>

			# Release Flags
			$<$<CONFIG:Release>:-O3>
			$<$<CONFIG:Release>:-DNDEBUG>
			$<$<CONFIG:Release>:-Wall>
			$<$<CONFIG:Release>:-Wextra>
			$<$<CONFIG:Release>:-Wpedantic>
			$<$<CONFIG:Release>:-Wconversion>
			$<$<CONFIG:Release>:-Wsign-conversion>
			$<$<CONFIG:Release>:-s>
		)
	elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
		target_compile_options(${target} PRIVATE
			# Debug flags
			$<$<CONFIG:Debug>:/Zi>
			$<$<CONFIG:Debug>:/Od>
			$<$<CONFIG:Debug>:/DDEBUG>
			$<$<CONFIG:Debug>:/W4>
			$<$<CONFIG:Debug>:/WX>
			$<$<CONFIG:Debug>:/Oy->
			$<$<CONFIG:Debug>:/EHsc>

			# Release flags
			$<$<CONFIG:Release>:/O2>
			$<$<CONFIG:Release>:/DNDEBUG>
			$<$<CONFIG:Release>:/GL>
			$<$<CONFIG:Release>:/W3>
			$<$<CONFIG:Release>:/Gy>
			$<$<CONFIG:Release>:/EHsc>
		)
	else ()
		message(WARNING "Unknown compiler: ${CMAKE_C_COMPILER_ID}. No compiler flags set.")
	endif ()
endfunction()

# Extensions
include(FetchContent)

add_subdirectory(${RAZE_TARGET_NAME})
