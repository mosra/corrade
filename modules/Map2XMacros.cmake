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
