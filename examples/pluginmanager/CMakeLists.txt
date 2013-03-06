corrade_add_plugin(Dog ${CMAKE_CURRENT_BINARY_DIR} Dog.conf Dog.cpp)
corrade_add_static_plugin(StaticPlugins Canary Canary.conf Canary.cpp)

add_executable(PluginTest main.cpp)
target_link_libraries(PluginTest ${StaticPlugins} ${CORRADE_PLUGINMANAGER_LIBRARIES})
