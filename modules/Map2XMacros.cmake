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
function(map2x_add_test test_name moc_header source_file)
    foreach(library ${ARGN})
        set(libraries ${library} ${libraries})
    endforeach()

    qt4_wrap_cpp(${test_name}_MOC ${moc_header})
    add_executable(${test_name} ${source_file} ${${test_name}_MOC})
    target_link_libraries(${test_name} ${libraries} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
    add_test(${test_name} ${test_name})
endfunction()

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
        COMMAND map2x-rc ${name} ${group_name} ${arguments} > ${CMAKE_CURRENT_BINARY_DIR}/${out}
        DEPENDS map2x-rc ${dependencies}
        COMMENT "Compiling data resource file ${out}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    # Save output filename
    set(${name} ${CMAKE_CURRENT_BINARY_DIR}/${out} PARENT_SCOPE)
endfunction()
