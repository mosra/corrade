#
# Set variable for current and also parent scope, if parent scope exists.
#
# Workaround for ugly CMake bug.
#
macro(set_parent_scope name)
    set(${name} ${ARGN})

    # Set to parent scope only if parent exists
    if(NOT ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
        set(${name} ${${name}} PARENT_SCOPE)
    endif()
endmacro()

#
# Function for adding QtTest unit tests
#
# These tests contain mainly from one source file and one header, which is
# processed by Qt meta-object compiler. The executable is then linked to QtCore
# and QtTest library, more libraries can be specified
#
# Example:
#       map2x_add_test(MySimpleTest SimpleTest.h SimpleTest.cpp CoreLibrary AnotherLibrary)
#
# is expanded to:
#       qt4_wrap_cpp(MySimpleTest_MOC SimpleTest.h)
#       add_executable(MySimpleTest SimpleTest.cpp ${MySimpleTest_MOC})
#       target_link_libraries(MySimpleTest CoreLibrary AnotherLibrary ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
#       add_test(MySimpleTest MySimpleTest)
#
# Test name is also executable name. Header file is processed with moc. Linked
# library count is unlimited. Note: the enable_testing() function must be called
# explicitly.
#
if(QT4_FOUND)
function(map2x_add_test test_name moc_header source_file)
    foreach(library ${ARGN})
        set(libraries ${library} ${libraries})
    endforeach()

    qt4_wrap_cpp(${test_name}_MOC ${moc_header})
    add_executable(${test_name} ${source_file} ${${test_name}_MOC})
    target_link_libraries(${test_name} ${libraries} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
    add_test(${test_name} ${test_name})
endfunction()
endif()

#
# Function for adding QtTest unit test with multiple source files
#
# Useful when there is need to compile more than one cpp/h file into the test.
#
# Example:
#       set(test_headers ComplexTest.h MyObject.h)
#       set(test_sources ComplexTest.cpp MyObject.cpp)
#       map2x_add_test(MyComplexTest test_headers test_sources CoreLibrary AnotherLibrary)
#
# is expanded to:
#       qt4_wrap_cpp(MyComplexTest_MOC ComplexTest.h MyObject.h)
#       add_executable(MyComplexTest ComplexTest.cpp MyObject.cpp ${MyComplexTest_MOC})
#       target_link_libraries(MyComplexTest CoreLibrary AnotherLibrary ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
#       add_test(MyComplexTest MyComplexTest)
#
function(map2x_add_multifile_test test_name moc_headers_variable source_files_variable)
    foreach(library ${ARGN})
        set(libraries ${library} ${libraries})
    endforeach()

    qt4_wrap_cpp(${test_name}_MOC ${${moc_headers_variable}})
    add_executable(${test_name} ${${source_files_variable}} ${${test_name}_MOC})
    target_link_libraries(${test_name} ${libraries} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
    add_test(${test_name} ${test_name})
endfunction()

#
# Macro for compiling data resources into application binary
#
# Depends on map2x-rc, which is part of Map2X utilities.
#
# Example usage:
#       map2x_add_resource(name group_name file1 ALIAS alias1 file2 file3 ALIAS alias3 ...)
#       add_executable(app source1 source2 ... ${name})
#
# This command generates resource file with group group_name from given
# files in current build directory. Argument name is name under which the
# resources can be explicitly loaded. Variable 'name' contains compiled
# resource filename, which is then used for compiling library / executable.
#
function(map2x_add_resource name group_name)
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
        COMMAND ${MAP2X_RC_EXECUTABLE} ${name} ${group_name} ${arguments} > ${CMAKE_CURRENT_BINARY_DIR}/${out}
        DEPENDS ${MAP2X_RC_EXECUTABLE} ${dependencies}
        COMMENT "Compiling data resource file ${out}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    # Save output filename
    set(${name} ${CMAKE_CURRENT_BINARY_DIR}/${out} PARENT_SCOPE)
endfunction()

#
# Function for adding dynamic plugins
#
# Usage:
#       map2x_add_plugin(plugin_name install_dir metadata_file file1.cpp file2.cpp ...)
#
# Additional libraries can be linked in via target_link_libraries(plugin_name ...).
#
# If install_dir is set to CMAKE_CURRENT_BINARY_DIR (e.g. for testing purposes),
# the files are copied directly, without need to run 'make install'.
#
function(map2x_add_plugin plugin_name install_dir metadata_file)
    add_library(${plugin_name} MODULE ${ARGN})
    if(${install_dir} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
        add_custom_command(
            OUTPUT ${plugin_name}.conf
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${metadata_file} ${CMAKE_CURRENT_BINARY_DIR}/${plugin_name}.conf
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${metadata_file}
        )
        add_custom_target(${plugin_name}-metadata ALL DEPENDS ${plugin_name}.conf)
    else()
        install(TARGETS ${plugin_name} DESTINATION "${install_dir}")
        install(FILES ${metadata_file} DESTINATION "${install_dir}" RENAME "${plugin_name}.conf")
    endif()
endfunction()

#
# Function for adding static plugins
#
# Usage:
#       map2x_add_static_plugin(static_plugins_variable plugin_name metadata_file file1.cpp ...)
#
# Additional libraries can be linked in via target_link_libraries(plugin_name ...).
#
# Plugin library name will be added at the end of static_plugins_variable and the
# variable is meant to be used while linking plugins to main executable/library,
# e.g:
#       target_link_libraries(app lib1 lib2 ... ${static_plugins_variable})
#
# This variable is set with parent scope so is available in parent directory. If
# there is more intermediate directories between plugin directory and main
# executable directory, the variable can be propagated to parent scope like
# this:
#       set(static_plugins_variable ${static_plugins_variable} PARENT_SCOPE)
#
macro(map2x_add_static_plugin static_plugins_variable plugin_name metadata_file)
    foreach(source ${ARGN})
        set(sources ${sources} ${source})
    endforeach()

    map2x_add_resource(${plugin_name} plugins ${metadata_file} ALIAS "${plugin_name}.conf")
    add_library(${plugin_name} STATIC ${sources} ${${plugin_name}})

    if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND CMAKE_COMPILER_IS_GNUCC AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set_target_properties(${plugin_name} PROPERTIES COMPILE_FLAGS -fPIC)
    endif()

    # Unset sources array (it's a macro, thus variables stay between calls)
    unset(sources)

    set_parent_scope(${static_plugins_variable} ${${static_plugins_variable}} ${plugin_name})
endmacro()
