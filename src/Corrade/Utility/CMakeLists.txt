#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>
#
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.
#

if(WITH_UTILITY)
    set(CorradeUtility_SRCS
        Debug.cpp
        Directory.cpp
        Configuration.cpp
        ConfigurationValue.cpp
        MurmurHash2.cpp
        Sha1.cpp
        System.cpp)

    set(CorradeUtility_GracefulAssert_SRCS
        Algorithms.cpp
        Arguments.cpp
        ConfigurationGroup.cpp
        Format.cpp
        Resource.cpp
        String.cpp
        Unicode.cpp

        ../Containers/ArrayTuple.cpp
        ../Containers/String.cpp
        ../Containers/StringView.cpp)

    set(CorradeUtility_HEADERS
        Algorithms.h
        Arguments.h
        AbstractHash.h
        Assert.h
        Configuration.h
        ConfigurationGroup.h
        ConfigurationValue.h
        Debug.h
        DebugStl.h
        Directory.h
        Endianness.h
        EndiannessBatch.h
        Format.h
        FormatStl.h
        Macros.h
        MurmurHash2.h
        Resource.h
        Sha1.h
        String.h
        StlForwardArray.h
        StlForwardString.h
        StlForwardTuple.h
        StlForwardVector.h
        StlMath.h
        System.h
        TypeTraits.h
        Unicode.h
        utilities.h
        Utility.h
        VisibilityMacros.h
        visibility.h)

    set(CorradeUtility_PRIVATE_HEADERS
        Implementation/Resource.h)

    # Unix-specific / non-RT-Windows-specific functionality. Also Emscripten.
    if(CORRADE_TARGET_UNIX OR (CORRADE_TARGET_WINDOWS AND NOT CORRADE_TARGET_WINDOWS_RT) OR CORRADE_TARGET_EMSCRIPTEN)
        list(APPEND CorradeUtility_SRCS
            FileWatcher.cpp
            Tweakable.cpp
            TweakableParser.cpp)
        list(APPEND CorradeUtility_HEADERS
            FileWatcher.h
            Tweakable.h
            TweakableParser.h)
        list(APPEND CorradeUtility_PRIVATE_HEADERS
            Implementation/tweakable.h)
    endif()

    # Android-specific functionality
    if(CORRADE_TARGET_ANDROID)
        list(APPEND CorradeUtility_SRCS AndroidLogStreamBuffer.cpp)
        list(APPEND CorradeUtility_HEADERS AndroidLogStreamBuffer.h)
    endif()

    # Functionality specific to static Windows builds
    if(CORRADE_TARGET_WINDOWS AND NOT CORRADE_TARGET_WINDOWS_RT AND CORRADE_BUILD_STATIC)
        list(APPEND CorradeUtility_SRCS Implementation/WindowsWeakSymbol.cpp)
        list(APPEND CorradeUtility_PRIVATE_HEADERS Implementation/WindowsWeakSymbol.h)
    endif()

    # Functionality specific to Windows builds
    if(CORRADE_TARGET_WINDOWS)
        list(APPEND CorradeUtility_SRCS Implementation/WindowsError.cpp)
        list(APPEND CorradeUtility_PRIVATE_HEADERS Implementation/WindowsError.h)
    endif()

    # Objects shared between main and test library
    add_library(CorradeUtilityObjects OBJECT
        ${CorradeUtility_SRCS}
        ${CorradeUtility_HEADERS}
        ${CorradeUtility_PRIVATE_HEADERS})
    target_include_directories(CorradeUtilityObjects PUBLIC $<TARGET_PROPERTY:CorradeUtility,INTERFACE_INCLUDE_DIRECTORIES>)
    if(NOT BUILD_STATIC)
        target_compile_definitions(CorradeUtilityObjects PRIVATE "-DCorradeUtilityObjects_EXPORTS")
    endif()
    if(NOT BUILD_STATIC OR BUILD_STATIC_PIC)
        set_target_properties(CorradeUtilityObjects PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()
    set_target_properties(CorradeUtilityObjects PROPERTIES FOLDER "Corrade/Utility")

    # Main Utility library
    add_library(CorradeUtility ${SHARED_OR_STATIC}
        $<TARGET_OBJECTS:CorradeUtilityObjects>
        ${CorradeUtility_GracefulAssert_SRCS})
    target_include_directories(CorradeUtility PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_BINARY_DIR}/src)
    # Require (at least) C++11 for users
    set_property(TARGET CorradeUtility PROPERTY
        INTERFACE_CORRADE_CXX_STANDARD 11)
    set_property(TARGET CorradeUtility APPEND PROPERTY
        COMPATIBLE_INTERFACE_NUMBER_MAX CORRADE_CXX_STANDARD)
    set_target_properties(CorradeUtility PROPERTIES
        DEBUG_POSTFIX "-d"
        FOLDER "Corrade/Utility")
    if(NOT BUILD_STATIC)
        set_target_properties(CorradeUtility PROPERTIES VERSION ${CORRADE_LIBRARY_VERSION} SOVERSION ${CORRADE_LIBRARY_SOVERSION})
    elseif(BUILD_STATIC_PIC)
        set_target_properties(CorradeUtility PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()

    # Directory::libraryLocation() needs this
    if(CORRADE_TARGET_UNIX)
        target_link_libraries(CorradeUtility PUBLIC ${CMAKE_DL_LIBS})
    endif()
    # AndroidLogStreamBuffer class needs to be linked to log library
    if(CORRADE_TARGET_ANDROID)
        target_link_libraries(CorradeUtility PUBLIC log)
    endif()

    install(TARGETS CorradeUtility
            RUNTIME DESTINATION ${CORRADE_BINARY_INSTALL_DIR}
            LIBRARY DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR}
            ARCHIVE DESTINATION ${CORRADE_LIBRARY_INSTALL_DIR})
    install(FILES ${CorradeUtility_HEADERS} DESTINATION ${CORRADE_INCLUDE_INSTALL_DIR}/Utility)

    if(BUILD_TESTS)
        # Library with graceful assert for testing
        add_library(CorradeUtilityTestLib ${SHARED_OR_STATIC} ${CorradeUtility_GracefulAssert_SRCS})
        target_compile_definitions(CorradeUtilityTestLib PRIVATE "CORRADE_GRACEFUL_ASSERT")
        set_target_properties(CorradeUtilityTestLib PROPERTIES
            DEBUG_POSTFIX "-d"
            FOLDER "Corrade/Utility")
        if(BUILD_STATIC_PIC)
            set_target_properties(CorradeUtilityTestLib PROPERTIES POSITION_INDEPENDENT_CODE ON)
        endif()

        target_link_libraries(CorradeUtilityTestLib CorradeUtility)

        add_subdirectory(Test)
    endif()

    # Corrade::Utility target alias for superprojects
    add_library(Corrade::Utility ALIAS CorradeUtility)
endif()

# TODO: https://cmake.org/pipermail/cmake-developers/2015-January/024242.html
# mentions that in the future it might be possible to not require external
# corrade-rc when generating WinRT targets by setting
#  set_target_properties(corrade-rc PROPERTIES VS_WINRT_COMPONENT OFF)
# Then this would be if(NOT CMAKE_CROSSCOMPILING OR CORRADE_TARGET_WINDOWS_RT).
# However, it seems like this feature never materialized, as doing this will
# result in corrade-rc that's looking for vcruntime140_app.dll in order to be
# run. Last checked: Nov 2019.
if(NOT CMAKE_CROSSCOMPILING)
    # Sources for standalone corrade-rc
    set(CorradeUtilityRc_SRCS
        Arguments.cpp
        Debug.cpp
        Directory.cpp
        Configuration.cpp
        ConfigurationGroup.cpp
        Format.cpp
        Resource.cpp
        String.cpp

        ../Containers/String.cpp
        ../Containers/StringView.cpp)
    if(CORRADE_TARGET_WINDOWS)
        list(APPEND CorradeUtilityRc_SRCS
            # Needed for dealing with the design failure that's called "Unicode WINAPI"
            Unicode.cpp
            Implementation/WindowsError.cpp)

        # Functionality specific to static Windows builds. Not really needed
        # for corrade-rc as it doesn't load any plugins yet but easier than
        # having a special handling.
        if(NOT CORRADE_TARGET_WINDOWS_RT)
            list(APPEND CorradeUtilityRc_SRCS Implementation/WindowsWeakSymbol.cpp)
        endif()
    endif()

    # Having a tool used during a build depend on a shared library brings a lot
    # of annoyances especially for CMake and RPATH beginners (and Windows users
    # as there is no RPATH), so I'm making it dependency-less here by compiling
    # everything the executable needs directly inside. In case the library is
    # already built as static, there's no need to do any extra work -- just
    # link it.
    if(NOT BUILD_STATIC OR NOT WITH_UTILITY)
        add_executable(corrade-rc rc.cpp ${CorradeUtilityRc_SRCS})
        # Same as Utility, but since we can't depend on it, we can't use its
        # INTERFACE_INCLUDE_DIRECTORIES
        target_include_directories(corrade-rc PRIVATE
            ${PROJECT_SOURCE_DIR}/src
            ${PROJECT_BINARY_DIR}/src)
        target_compile_definitions(corrade-rc PRIVATE "CORRADE_BUILD_STATIC")
    else()
        add_executable(corrade-rc rc.cpp)
        target_link_libraries(corrade-rc PRIVATE CorradeUtility)
    endif()
    # Directory::libraryLocation() needs this
    if(CORRADE_TARGET_UNIX)
        target_link_libraries(corrade-rc PRIVATE ${CMAKE_DL_LIBS})
    endif()
    set_target_properties(corrade-rc PROPERTIES FOLDER "Corrade/Utility")
    install(TARGETS corrade-rc DESTINATION ${CORRADE_BINARY_INSTALL_DIR})

    # Corrade::rc target alias for superprojects
    add_executable(Corrade::rc ALIAS corrade-rc)
endif()
