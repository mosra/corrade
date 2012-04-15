include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/NativePluginAccessorConfigure.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/NativePluginAccessorConfigure.h)

corrade_add_static_plugin(PluginManagerStaticPlugins NativePluginAccessor NativePluginAccessor.conf NativePluginAccessor.cpp)