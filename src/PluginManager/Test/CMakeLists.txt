find_package(Qt4 REQUIRED)

enable_testing()
include(Map2XMacros)

set(PLUGINMANAGER_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PluginTestConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/PluginTestConfigure.h)

include_directories(${Map2XCore_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})

map2x_add_plugin(Dog ${CMAKE_CURRENT_BINARY_DIR} Dog.conf Dog.cpp)
map2x_add_static_plugin(Map2XCore_PluginManager_Test_Plugins Canary Canary.conf Canary.cpp)
map2x_add_test(PluginTest PluginTest.h PluginTest.cpp Map2XPluginManager ${Map2XCore_PluginManager_Test_Plugins})
