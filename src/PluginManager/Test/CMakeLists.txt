find_package(Qt4 REQUIRED)

enable_testing()

set(PLUGINMANAGER_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PluginTestConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/PluginTestConfigure.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(food)

kompas_add_plugin(Snail ${CMAKE_CURRENT_BINARY_DIR} Snail.conf Snail.cpp)
kompas_add_plugin(Dog ${CMAKE_CURRENT_BINARY_DIR} Dog.conf Dog.cpp)
kompas_add_plugin(Chihuahua ${CMAKE_CURRENT_BINARY_DIR} Chihuahua.conf Chihuahua.cpp)
kompas_add_static_plugin(KompasCore_PluginManager_Test_Plugins Canary Canary.conf Canary.cpp)
kompas_add_test(PluginTest PluginTest.h PluginTest.cpp KompasPluginManager ${KompasCore_PluginManager_Test_Plugins})

if(WIN32)
    target_link_libraries(Snail KompasPluginManager)
    target_link_libraries(Dog KompasPluginManager)
    target_link_libraries(Chihuahua KompasPluginManager Dog)
endif()
