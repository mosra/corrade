kompas_add_plugin(HotDog ${CMAKE_CURRENT_BINARY_DIR} HotDog.conf HotDog.cpp)
kompas_add_plugin(HotDogWithSnail ${CMAKE_CURRENT_BINARY_DIR} HotDogWithSnail.conf HotDog.cpp)

if(WIN32)
    target_link_libraries(HotDog KompasPluginManager Dog)
endif()
