
# (the blank line is here so CMake doesn't generate documentation from it)

#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013
#             Vladimír Vondruš <mosra@centrum.cz>
#
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.
#

# Already included, nothing to do
if(_CORRADE_USE_INCLUDED)
    return()
endif()

# Mandatory C++ flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

# Optional C++ flags
set(CORRADE_CXX_FLAGS "-Wall -Wextra -Wold-style-cast -Winit-self -Werror=return-type -Wmissing-declarations -pedantic -fvisibility=hidden")

# Some flags are not yet supported everywhere
# TODO: do this with check_c_compiler_flags()
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if(NOT "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.7.0")
        set(CORRADE_CXX_FLAGS "${CORRADE_CXX_FLAGS} -Wzero-as-null-pointer-constant")
    endif()
    if(NOT "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.6.0")
        set(CORRADE_CXX_FLAGS "${CORRADE_CXX_FLAGS} -Wdouble-promotion")
    endif()
endif()

# Set variable for current and also parent scope, if parent scope exists.
#  set_parent_scope(name value)
# Workaround for ugly CMake bug.
macro(set_parent_scope name)
    if("${ARGN}" STREQUAL "")
        set(${name} "")
    else()
        set(${name} ${ARGN})
    endif()

    # Set to parent scope only if parent exists
    if(NOT ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
        if("${ARGN}" STREQUAL "")
            # CMake bug: nothing is set in parent scope
            set(${name} "" PARENT_SCOPE)
        else()
            set(${name} ${${name}} PARENT_SCOPE)
        endif()
    endif()
endmacro()

function(corrade_add_test test_name)
    # Get DLL and path lists
    foreach(arg ${ARGN})
        if(${arg} STREQUAL LIBRARIES)
            set(__DOING_LIBRARIES ON)
        else()
            if(__DOING_LIBRARIES)
                set(libraries ${libraries} ${arg})
            else()
                set(sources ${sources} ${arg})
            endif()
        endif()
    endforeach()

    add_executable(${test_name} ${sources})
    target_link_libraries(${test_name} ${CORRADE_TESTSUITE_LIBRARIES} ${libraries})
    if(CORRADE_TARGET_EMSCRIPTEN)
        find_package(NodeJs REQUIRED)
        add_test(NAME ${test_name} COMMAND ${NODEJS_EXECUTABLE} $<TARGET_FILE:${test_name}>)
    else()
        add_test(${test_name} ${test_name})
    endif()
endfunction()

function(corrade_add_resource name group_name)
    set(IS_ALIAS OFF)
    foreach(argument ${ARGN})

        # Next argument is alias
        if(${argument} STREQUAL "ALIAS")
            set(IS_ALIAS ON)

        # This argument is alias
        elseif(IS_ALIAS)
            set(arguments ${arguments} -a ${argument})
            set(IS_ALIAS OFF)

        # Filename
        else()
            set(arguments ${arguments} ${argument})
            set(dependencies ${dependencies} ${argument})
        endif()
    endforeach()

    # Run command
    set(out resource_${name}.cpp)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${out}
        COMMAND ${CORRADE_RC_EXECUTABLE} ${name} ${group_name} ${arguments} > ${CMAKE_CURRENT_BINARY_DIR}/${out}
        DEPENDS ${CORRADE_RC_EXECUTABLE} ${dependencies}
        COMMENT "Compiling data resource file ${out}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    # Save output filename
    set(${name} ${CMAKE_CURRENT_BINARY_DIR}/${out} PARENT_SCOPE)
endfunction()

function(corrade_add_plugin plugin_name install_dir metadata_file)
    if(WIN32)
        add_library(${plugin_name} SHARED ${ARGN})
    else()
        add_library(${plugin_name} MODULE ${ARGN})
    endif()

    # Plugins doesn't have any prefix (e.g. 'lib' on Linux)
    set_target_properties(${plugin_name} PROPERTIES
        PREFIX ""
        COMPILE_FLAGS -DCORRADE_DYNAMIC_PLUGIN)

    if(${install_dir} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
        add_custom_command(
            OUTPUT ${plugin_name}.conf
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${metadata_file} ${CMAKE_CURRENT_BINARY_DIR}/${plugin_name}.conf
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${metadata_file})
        add_custom_target(${plugin_name}-metadata ALL DEPENDS ${plugin_name}.conf)
    else()
        install(TARGETS ${plugin_name} DESTINATION "${install_dir}")
        install(FILES ${metadata_file} DESTINATION "${install_dir}" RENAME "${plugin_name}.conf")
    endif()
endfunction()

macro(corrade_add_static_plugin static_plugins_variable plugin_name metadata_file)
    foreach(source ${ARGN})
        set(sources ${sources} ${source})
    endforeach()

    corrade_add_resource(${plugin_name} plugins ${metadata_file} ALIAS "${plugin_name}.conf")
    add_library(${plugin_name} STATIC ${sources} ${${plugin_name}})

    set_target_properties(${plugin_name} PROPERTIES COMPILE_FLAGS "-DCORRADE_STATIC_PLUGIN ${CMAKE_SHARED_LIBRARY_CXX_FLAGS}")

    # Unset sources array (it's a macro, thus variables stay between calls)
    unset(sources)

    set_parent_scope(${static_plugins_variable} ${${static_plugins_variable}} ${plugin_name})
endmacro()

set(_CORRADE_USE_INCLUDED TRUE)
