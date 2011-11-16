corrade_add_plugin(HotDog ${CMAKE_CURRENT_BINARY_DIR} HotDog.conf HotDog.cpp)
corrade_add_plugin(HotDogWithSnail ${CMAKE_CURRENT_BINARY_DIR} HotDogWithSnail.conf HotDog.cpp)

if(WIN32)
    target_link_libraries(HotDog CorradePluginManager Dog)
endif()
