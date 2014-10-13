cmake_minimum_required(VERSION 2.8)
project(StepMania)

if (MSVC)
	# Borrow from http://binglongx.wordpress.com/2013/06/30/add-arbitrary-compilelink-flags-in-cmake/
	macro(my_append_target_property target property str)

		get_target_property(current_property ${target} ${property})
		if (current_property)
			set_target_properties(${target} PROPERTIES ${property} "${current_property} ${str}")
		else(current_property)
			set_target_properties(${target} PROPERTIES ${property} ${str})
		endif(current_property)

	endmacro(my_append_target_property)

	macro(my_add_compile_flags target flags)
		my_append_target_property(${target} COMPILE_FLAGS ${flags})
	endmacro(my_add_compile_flags)

	macro(my_add_link_flags target flags)
		my_append_target_property(${target} LINK_FLAGS ${flags})
	endmacro(my_add_link_flags)
endif(MSVC)

# Put predefined targets in separate group.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Do setup.
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

include(FindDirectX)

add_subdirectory(extern)
add_subdirectory(src)