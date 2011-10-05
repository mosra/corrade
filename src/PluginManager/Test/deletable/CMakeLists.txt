kompas_add_plugin(Deletable ${CMAKE_CURRENT_BINARY_DIR} Deletable.conf Deletable.cpp)

if(WIN32)
    target_link_libraries(Deletable KompasPluginManager)
endif()
