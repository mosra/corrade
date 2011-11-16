if(NOT WIN32)
    install(FILES
        FindCorrade.cmake
        CorradeLibSuffix.cmake
        CorradeMacros.cmake
        DESTINATION ${CORRADE_CMAKE_MODULE_INSTALL_DIR})
endif()
