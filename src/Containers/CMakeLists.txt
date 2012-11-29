set(CorradeContainers_HEADERS
    EnumSet.h
    LinkedList.h)

install(FILES ${CorradeContainers_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}/Containers)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Test)
endif()
