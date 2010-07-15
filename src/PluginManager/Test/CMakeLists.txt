find_package(Qt4 REQUIRED)

enable_testing()

set(PLUGINMANAGER_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PluginTestConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/PluginTestConfigure.h)

include_directories(${Map2XCore_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})

add_library(Dog STATIC Dog.cpp)
add_library(Canary MODULE Canary.cpp)

set(Map2X_PluginManager_PluginTest_SRCS
    PluginTest.cpp
)

qt4_wrap_cpp(Map2X_PluginManager_PluginTest_MOC
    PluginTest.h
)

add_executable(PluginTest ${Map2X_PluginManager_PluginTest_SRCS} ${Map2X_PluginManager_PluginTest_MOC})
target_link_libraries(PluginTest Map2XUtility Map2XPluginManager Dog ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

add_test(PluginText PluginTest)
