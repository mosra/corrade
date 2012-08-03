if(NOT CMAKE_CROSSCOMPILING)
    install(FILES
        FindCorrade.cmake
        CorradeLibSuffix.cmake
        CorradeMacros.cmake
        DESTINATION ${CORRADE_CMAKE_MODULE_INSTALL_DIR})
endif()
