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
