set(CorradeInterconnect_SRCS
    Connection.cpp
    Emitter.cpp
    Receiver.cpp)

set(CorradeInterconnect_HEADERS
    Connection.h
    Emitter.h
    Receiver.h

    corradeInterconnectVisibility.h)

add_library(CorradeInterconnect SHARED ${CorradeInterconnect_SRCS})
set_target_properties(CorradeInterconnect PROPERTIES VERSION ${CORRADE_LIBRARY_VERSION} SOVERSION ${CORRADE_LIBRARY_SOVERSION})
target_link_libraries(CorradeInterconnect CorradeUtility)

install(TARGETS CorradeInterconnect DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})
install(FILES ${CorradeInterconnect_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}/Interconnect)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Test)
endif()
