set(PLUGINMANAGER_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PluginTestConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/PluginTestConfigure.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(deletable)
add_subdirectory(food)

corrade_add_plugin(Snail ${CMAKE_CURRENT_BINARY_DIR} Snail.conf Snail.cpp)
corrade_add_plugin(Dog ${CMAKE_CURRENT_BINARY_DIR} Dog.conf Dog.cpp)
corrade_add_plugin(Chihuahua ${CMAKE_CURRENT_BINARY_DIR} Chihuahua.conf Chihuahua.cpp)
corrade_add_static_plugin(CorradePluginManagerTestPlugins Canary Canary.conf Canary.cpp)
corrade_add_test(PluginManagerPluginTest PluginTest.cpp LIBRARIES CorradePluginManager ${CorradePluginManagerTestPlugins})

if(WIN32)
    target_link_libraries(Snail CorradePluginManager)
    target_link_libraries(Dog CorradePluginManager)
    target_link_libraries(Canary CorradePluginManager)
    target_link_libraries(Chihuahua CorradePluginManager Dog)
endif()
