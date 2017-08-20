
# (the blank line is here so CMake doesn't generate documentation from it)

#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017 Vladimír Vondruš <mosra@centrum.cz>
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

# Already included, nothing to do
if(_CORRADE_USE_INCLUDED)
    return()
endif()

# Check compiler version
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Don't allow to use compilers older than what compatibility mode allows
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.0")
        message(FATAL_ERROR "Corrade cannot be used with GCC < 4.7. Sorry.")
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.1")
        if(NOT CORRADE_GCC47_COMPATIBILITY)
            message(FATAL_ERROR "To use Corrade with GCC 4.7, build it with GCC47_COMPATIBILITY enabled")
        endif()
    endif()

    # Don't allow to use compiler newer than what compatibility mode allows
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.1")
        if(CORRADE_GCC47_COMPATIBILITY)
            message(FATAL_ERROR "GCC >= 4.8.1 cannot be used if Corrade is built with GCC47_COMPATIBILITY")
        endif()
    endif()
elseif(MSVC)
    # Don't allow to use compilers older than what compatibility mode allows
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.0")
        message(FATAL_ERROR "Corrade cannot be used with MSVC < 2015. Sorry.")
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.10")
        if(NOT CORRADE_MSVC2015_COMPATIBILITY)
            message(FATAL_ERROR "To use Corrade with MSVC 2015, build it with MSVC2015_COMPATIBILITY enabled")
        endif()
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.20")
        if(NOT CORRADE_MSVC2017_COMPATIBILITY)
            message(FATAL_ERROR "To use Corrade with MSVC 2017, build it with MSVC2017_COMPATIBILITY enabled")
        endif()
    endif()

    # Don't allow to use compiler newer than what compatibility mode allows
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.10")
        if(CORRADE_MSVC2015_COMPATIBILITY)
            message(FATAL_ERROR "MSVC >= 2017 cannot be used if Corrade is built with MSVC2015_COMPATIBILITY")
        endif()
    endif()
endif()

# GCC/Clang-specific compiler flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?Clang" OR CORRADE_TARGET_EMSCRIPTEN)
    set(CORRADE_PEDANTIC_COMPILER_OPTIONS
        "-Wall" "-Wextra"
        "$<$<STREQUAL:$<TARGET_PROPERTY:LINKER_LANGUAGE>,CXX>:-Wold-style-cast>"
        "-Winit-self"
        "-Werror=return-type"
        "-Wmissing-declarations"
        "-pedantic"
        "-fvisibility=hidden")

    # Some flags are not yet supported everywhere
    # TODO: do this with check_c_compiler_flags()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        list(APPEND CORRADE_PEDANTIC_COMPILER_OPTIONS
            "$<$<STREQUAL:$<TARGET_PROPERTY:LINKER_LANGUAGE>,CXX>:-Wzero-as-null-pointer-constant>"

            # TODO: enable when this gets to Clang (not in 3.9, but in master
            # since https://github.com/llvm-mirror/clang/commit/0a022661c797356e9c28e4999b6ec3881361371e)
            "-Wdouble-promotion")
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?Clang" OR CORRADE_TARGET_EMSCRIPTEN)
        list(APPEND CORRADE_PEDANTIC_COMPILER_OPTIONS
            # Clang's -Wmissing-declarations does something else and the
            # behavior we want is under -Wmissing-prototypes. See
            # https://llvm.org/bugs/show_bug.cgi?id=16286.
            "-Wmissing-prototypes"

            # Fixing it in all places would add too much noise to the code.
            "-Wno-shorten-64-to-32")
    endif()

# MSVC-specific compiler flags
elseif(MSVC)
    set(CORRADE_PEDANTIC_COMPILER_OPTIONS
        # Enable extra warnings (similar to -Wall)
        "/W4"

        # "needs to have dll-interface to be used by clients", as the fix for
        # that would effectively prevent using STL completely.
        "/wd4251"

        # "conversion from '<bigger int type>' to '<smaller int type>'",
        # "conversion from 'size_t' to '<smaller int type>', possible loss of
        # data", fixing this would add too much noise. Equivalent to
        # -Wshorten-64-to-32 on Clang.
        "/wd4244"
        "/wd4267"

        # "new behavior: elements of array will be default initialized".
        # YES. YES I KNOW WHAT I'M DOING.
        "/wd4351"

        # "previous versions of the compiler did not override when parameters
        # only differed by const/volatile qualifiers". Okay. So you had bugs.
        # And?
        "/wd4373"

        # "default constructor could not be generated/can never be
        # instantiated". Apparently it can.
        "/wd4510"
        "/wd4610"

        # "assignment operator could not be generated". Do I want one? NO I
        # DON'T.
        "/wd4512"

        # "no suitable definition for explicit template instantiation". No. The
        # problem is apparently that I'm having the definitions in *.cpp file
        # and instantiating them explicitly. Common practice here.
        "/wd4661"

        # "unreachable code". *Every* assertion has return after std::abort().
        # So?
        "/wd4702"

        # "assignment within conditional expression". It's not my problem that
        # it doesn't get the hint with extra parentheses (`if((a = b))`).
        "/wd4706"

        # "forcing value to bool 'true' or 'false' (performance warning)". So
        # what. I won't wrap everything in bool(). This is a _language feature_,
        # dammit.
        "/wd4800"

        # "dllexport and extern are incompatible on an explicit instantiation".
        # Why the error is emitted only on classes? Functions are okay with
        # dllexport extern?!
        "/wd4910")
    set(CORRADE_PEDANTIC_COMPILER_DEFINITIONS
        # Disabling warning for not using "secure-but-not-standard" STL algos
        "_CRT_SECURE_NO_WARNINGS" "_SCL_SECURE_NO_WARNINGS"

        # Disabling all minmax nonsense macros
        "NOMINMAX"

        # Disabling GDI and other mud in windows.h
        "WIN32_LEAN_AND_MEAN")
endif()

define_property(TARGET PROPERTY CORRADE_CXX_STANDARD INHERITED
    BRIEF_DOCS "C++ standard to require for given target"
    FULL_DOCS "Sets compiler-specific flags to enable C++11 or later standard
        when building given target or targets in given directory. Set in
        combination with INTERFACE_CORRADE_CXX_STANDARD to force the standard
        also on users of given target.")
define_property(TARGET PROPERTY INTERFACE_CORRADE_CXX_STANDARD INHERITED
    BRIEF_DOCS "C++ standard to require for users of given target"
    FULL_DOCS "Sets compiler-specific flags to enable C++11 or later standard
        when using given target or targets in given directory.")
define_property(TARGET PROPERTY CORRADE_USE_PEDANTIC_FLAGS INHERITED
    BRIEF_DOCS "Use pedantic compiler/linker flags"
    FULL_DOCS "Enables additional pedantic C, C++ and linker flags on given
        targets or directories.")

# Enable C++11 on GCC/Clang if CORRADE_CXX_STANDARD is set to 11 on the target.
# Does nothing in case the user specified CXX_STANDARD property or put "-std="
# in CMAKE_CXX_FLAGS nothing would be added. It doesn't cover adding flags
# using target_compile_options(), though.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?Clang" OR CORRADE_TARGET_EMSCRIPTEN AND NOT CMAKE_CXX_FLAGS MATCHES "-std=")
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9) OR ((CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?Clang" OR CORRADE_TARGET_EMSCRIPTEN) AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.5))
        set(_CORRADE_CXX14_STANDARD_FLAG "-std=c++14")
    else()
        set(_CORRADE_CXX14_STANDARD_FLAG "-std=c++1y")
    endif()

    set(_CORRADE_CXX_STANDARD_ONLY_IF_NOT_ALREADY_SET
        "$<STREQUAL:$<TARGET_PROPERTY:LINKER_LANGUAGE>,CXX>,$<NOT:$<BOOL:$<TARGET_PROPERTY:CXX_STANDARD>>>")

    set_property(DIRECTORY APPEND PROPERTY COMPILE_OPTIONS
        "$<$<AND:${_CORRADE_CXX_STANDARD_ONLY_IF_NOT_ALREADY_SET},$<STREQUAL:$<TARGET_PROPERTY:CORRADE_CXX_STANDARD>,11>>:-std=c++11>"
        "$<$<AND:${_CORRADE_CXX_STANDARD_ONLY_IF_NOT_ALREADY_SET},$<STREQUAL:$<TARGET_PROPERTY:CORRADE_CXX_STANDARD>,14>>:${_CORRADE_CXX14_STANDARD_FLAG}>"
        "$<$<AND:${_CORRADE_CXX_STANDARD_ONLY_IF_NOT_ALREADY_SET},$<STREQUAL:$<TARGET_PROPERTY:CORRADE_CXX_STANDARD>,17>>:-std=c++1z>")

    # See FindCorrade.cmake for a juicy rant about why I have to use *also*
    # CORRADE_CXX_STANDARD_ on 2.8.12. GODDAMIT.
    if(CMAKE_VERSION VERSION_LESS 3.0.0)
        set_property(DIRECTORY APPEND PROPERTY COMPILE_OPTIONS
            "$<$<AND:${_CORRADE_CXX_STANDARD_ONLY_IF_NOT_ALREADY_SET},$<STREQUAL:$<TARGET_PROPERTY:CORRADE_CXX_STANDARD_>,11>>:-std=c++11>"
            "$<$<AND:${_CORRADE_CXX_STANDARD_ONLY_IF_NOT_ALREADY_SET},$<STREQUAL:$<TARGET_PROPERTY:CORRADE_CXX_STANDARD_>,14>>:${_CORRADE_CXX14_STANDARD_FLAG}>"
            "$<$<AND:${_CORRADE_CXX_STANDARD_ONLY_IF_NOT_ALREADY_SET},$<STREQUAL:$<TARGET_PROPERTY:CORRADE_CXX_STANDARD_>,17>>:-std=c++1z>")
    endif()
endif()

# On-demand pedantic compiler flags
set_property(DIRECTORY APPEND PROPERTY COMPILE_OPTIONS "$<$<BOOL:$<TARGET_PROPERTY:CORRADE_USE_PEDANTIC_FLAGS>>:${CORRADE_PEDANTIC_COMPILER_OPTIONS}>")
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS "$<$<BOOL:$<TARGET_PROPERTY:CORRADE_USE_PEDANTIC_FLAGS>>:${CORRADE_PEDANTIC_COMPILER_DEFINITIONS}>")

# Provide CORRADE_CXX_FLAGS for backwards compatibility
if(CORRADE_BUILD_DEPRECATED)
    if(CORRADE_PEDANTIC_COMPILER_DEFINITIONS)
        string(REPLACE ";" " -D" CORRADE_CXX_FLAGS "-D${CORRADE_PEDANTIC_COMPILER_DEFINITIONS}")
    endif()
    string(REPLACE ";" " " CORRADE_CXX_FLAGS "${CORRADE_CXX_FLAGS}${CORRADE_PEDANTIC_COMPILER_OPTIONS}")

    # Remove generator expressions that distinct between C and C++
    string(REGEX REPLACE "\\$<\\$<STREQUAL:\\$<TARGET_PROPERTY:LINKER_LANGUAGE>,CXX>:([^>]+)>" "\\1" CORRADE_CXX_FLAGS ${CORRADE_CXX_FLAGS})
endif()

# Provide a way to distinguish between debug and release builds via
# preprocessor define
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS "$<$<CONFIG:Debug>:CORRADE_IS_DEBUG_BUILD>")

if(CORRADE_TESTSUITE_TARGET_XCTEST)
    find_package(XCTest)

    # Workaround for CMake iOS generator expression bug, see below
    if(CORRADE_TARGET_IOS)
        if(CMAKE_OSX_SYSROOT MATCHES "iPhoneOS")
            set(_CORRADE_EFFECTIVE_PLATFORM_NAME "-iphoneos")
        elseif(CMAKE_OSX_SYSROOT MATCHES "iPhoneSimulator")
            set(_CORRADE_EFFECTIVE_PLATFORM_NAME "-iphonesimulator")
        endif()
    endif()
endif()

if(CORRADE_TARGET_EMSCRIPTEN)
    # For bundling files to the tests
    include(UseEmscripten)
endif()

if(CORRADE_TARGET_IOS AND NOT CORRADE_TESTSUITE_TARGET_XCTEST)
    set(CORRADE_TESTSUITE_BUNDLE_IDENTIFIER_PREFIX ${PROJECT_NAME} CACHE STRING
        "Bundle identifier prefix for tests ran on iOS device")
endif()

function(corrade_add_test test_name)
    set(_corrade_file_pair_match "^(.+)@([^@]+)$")
    set(_corrade_file_pair_replace "\\1;\\2")

    # Get DLL and path lists
    foreach(arg ${ARGN})
        if(arg STREQUAL LIBRARIES)
            set(_DOING_LIBRARIES ON)
        elseif(arg STREQUAL FILES)
            set(_DOING_LIBRARIES OFF)
            set(_DOING_FILES ON)
        else()
            if(_DOING_LIBRARIES)
                list(APPEND libraries ${arg})
            elseif(_DOING_FILES)
                # If the file is already a pair of file and destination, just
                # extract them
                if(${arg} MATCHES ${_corrade_file_pair_match})
                    set(input_filename ${CMAKE_MATCH_1})
                    set(output_filename ${CMAKE_MATCH_2})

                # Otherwise create the output filename from the input
                else()
                    set(input_filename ${arg})

                    # Extract only the leaf component from absolute filename
                    # (applies also to paths with ..)
                    if(IS_ABSOLUTE ${arg} OR ${arg} MATCHES "\\.\\.[\\\\/]")
                        get_filename_component(output_filename ${arg} NAME)

                    # Otherwise use the full relative path as output filename
                    else()
                        set(output_filename ${arg})
                    endif()
                endif()

                # Sanity checks
                if(${output_filename} MATCHES "(\\.\\.[\\\\/]|@)" OR IS_ABSOLUTE ${output_filename})
                    message(SEND_ERROR "Names of files added to corrade_add_test() can't contain .., @ or be absolute")
                endif()

                # Convert input to absolute, concatenate the files back and
                # add to the list
                get_filename_component(input_filename ${input_filename} ABSOLUTE)
                list(APPEND files ${input_filename}@${output_filename})
                list(APPEND absolute_files ${absolute_input_filename})
            else()
                list(APPEND sources ${arg})
            endif()
        endif()
    endforeach()

    if(CORRADE_TESTSUITE_TARGET_XCTEST)
        add_library(${test_name} SHARED ${sources})
        set_target_properties(${test_name} PROPERTIES FRAMEWORK TRUE)
        target_link_libraries(${test_name} PRIVATE ${libraries} Corrade::TestSuite)

        set(test_runner_file ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.mm)
        configure_file(${CORRADE_TESTSUITE_XCTEST_RUNNER}
                       ${test_runner_file})
        xctest_add_bundle(${test_name}Runner ${test_name} ${test_runner_file})
        if(CORRADE_TARGET_IOS)
            # The EFFECTIVE_PLATFORM_NAME variable is not expanded when using
            # TARGET_* generator expressions on iOS, we need to hardcode it
            # manually. See http://public.kitware.com/pipermail/cmake/2016-March/063049.html
            add_test(NAME ${test_name} COMMAND ${XCTest_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>${_CORRADE_EFFECTIVE_PLATFORM_NAME}/${test_name}Runner.xctest)
        else()
            xctest_add_test(${test_name} ${test_name}Runner)
        endif()
    else()
        add_executable(${test_name} ${sources})
        target_link_libraries(${test_name} PRIVATE ${libraries} Corrade::TestSuite)

        # Run tests using Node.js on Emscripten
        if(CORRADE_TARGET_EMSCRIPTEN)
            # Emscripten needs to have exceptions enabled for TestSuite to work
            # properly
            set_property(TARGET ${test_name} APPEND_STRING PROPERTY LINK_FLAGS "-s DISABLE_EXCEPTION_CATCHING=0")
            find_package(NodeJs REQUIRED)
            add_test(NAME ${test_name} COMMAND NodeJs::NodeJs --stack-trace-limit=0 $<TARGET_FILE:${test_name}>)

            # Embed all files
            foreach(file ${files})
                string(REGEX REPLACE ${_corrade_file_pair_match} "${_corrade_file_pair_replace}" file_pair ${file})
                list(GET file_pair 0 input_filename)
                list(GET file_pair 1 output_filename)
                emscripten_embed_file(${test_name} ${input_filename} "/${output_filename}")
            endforeach()

        # Run tests using ADB on Android
        elseif(CORRADE_TARGET_ANDROID)
            # The executables need to be PIE
            target_compile_options(${test_name} PRIVATE "-fPIE")
            set_property(TARGET ${test_name} APPEND_STRING PROPERTY LINK_FLAGS "-fPIE -pie")
            # All files will be copied to the target when the test is run
            add_test(NAME ${test_name} COMMAND ${CORRADE_TESTSUITE_ADB_RUNNER} $<TARGET_FILE_DIR:${test_name}> $<TARGET_FILE_NAME:${test_name}> ${files})

        # Run tests natively elsewhere
        else()
            add_test(${test_name} ${test_name})
        endif()

        # iOS-specific
        if(CORRADE_TARGET_IOS)
            set_target_properties(${test_name} PROPERTIES
                MACOSX_BUNDLE ON
                MACOSX_BUNDLE_GUI_IDENTIFIER ${CORRADE_TESTSUITE_BUNDLE_IDENTIFIER_PREFIX}.${test_name}
                XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "YES")
        endif()
    endif()

    # Add the file to list of required files for given test case
    set_tests_properties(${test_name} PROPERTIES REQUIRED_FILES "${absolute_files}")
endfunction()

function(corrade_add_resource name configurationFile)
    # Parse dependencies from the file
    set(dependencies )
    set(filenameRegex "^[ \t]*filename[ \t]*=[ \t]*\"?([^\"]+)\"?[ \t]*$")
    get_filename_component(configurationFilePath ${configurationFile} PATH)

    # CMake < 3.1 can't handle UTF-8 in file(STRINGS)
    if(CMAKE_VERSION VERSION_LESS 3.1)
        file(STRINGS "${configurationFile}" files REGEX ${filenameRegex})
    else()
        file(STRINGS "${configurationFile}" files REGEX ${filenameRegex} ENCODING UTF-8)
    endif()
    foreach(file ${files})
        string(REGEX REPLACE ${filenameRegex} "\\1" filename "${file}")
        if(NOT IS_ABSOLUTE "${filename}" AND configurationFilePath)
            set(filename "${configurationFilePath}/${filename}")
        endif()
        list(APPEND dependencies "${filename}")
    endforeach()

    # Force IDEs display also the resource files in project view
    add_custom_target(${name}-dependencies SOURCES ${dependencies})

    # Output file name
    set(out "${CMAKE_CURRENT_BINARY_DIR}/resource_${name}.cpp")
    set(outDepends "${CMAKE_CURRENT_BINARY_DIR}/resource_${name}.depends")

    # Use configure_file() to trick CMake to re-run and update the dependency
    # list when the resource list file changes (otherwise it parses the file
    # only during the explicit configure step and never again, thus additions/
    # deletions are not recognized automatically)
    configure_file(${configurationFile} ${outDepends} COPYONLY)

    # Run command
    add_custom_command(
        OUTPUT "${out}"
        COMMAND Corrade::rc ${name} "${configurationFile}" "${out}"
        DEPENDS Corrade::rc ${outDepends} ${dependencies} ${name}-dependencies
        COMMENT "Compiling data resource file ${out}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    # Save output filename
    set(${name} "${out}" PARENT_SCOPE)
endfunction()

function(corrade_add_plugin plugin_name debug_install_dirs release_install_dirs metadata_file)
    if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_WINDOWS_RT OR CORRADE_TARGET_IOS)
        message(SEND_ERROR "corrade_add_plugin(): dynamic plugins are not available on this platform, use corrade_add_static_plugin() instead")
    endif()

    # Populate {debug,release}_{binary,library,conf}_install_dir variables
    if(NOT debug_install_dirs STREQUAL CMAKE_CURRENT_BINARY_DIR)
        list(LENGTH debug_install_dirs debug_install_dir_count)
        list(LENGTH release_install_dirs release_install_dir_count)
        if(NOT debug_install_dir_count EQUAL release_install_dir_count)
            message(FATAL_ERROR "corrade_add_plugin(): either none or both install dirs must contain binary location")
        elseif(debug_install_dir_count EQUAL 1)
            set(debug_binary_install_dir ${debug_install_dirs})
            set(debug_library_install_dir ${debug_install_dirs})
            set(release_binary_install_dir ${release_install_dirs})
            set(release_library_install_dir ${release_install_dirs})
        elseif(debug_install_dir_count EQUAL 2)
            list(GET debug_install_dirs 0 debug_binary_install_dir)
            list(GET debug_install_dirs 1 debug_library_install_dir)
            list(GET release_install_dirs 0 release_binary_install_dir)
            list(GET release_install_dirs 1 release_library_install_dir)
        else()
            message(FATAL_ERROR "corrade_add_plugin(): install dirs must contain either just library location or both binary and library location")
        endif()

        if(CORRADE_TARGET_WINDOWS)
            set(debug_conf_install_dir ${debug_binary_install_dir})
            set(release_conf_install_dir ${release_binary_install_dir})
        else()
            set(debug_conf_install_dir ${debug_library_install_dir})
            set(release_conf_install_dir ${release_library_install_dir})
        endif()
    endif()

    # Create dynamic library and bring all needed options along
    if(CORRADE_TARGET_WINDOWS)
        add_library(${plugin_name} SHARED ${ARGN})
    else()
        add_library(${plugin_name} MODULE ${ARGN})
    endif()
    set_target_properties(${plugin_name} PROPERTIES CORRADE_CXX_STANDARD 11)
    target_compile_definitions(${plugin_name} PRIVATE "CORRADE_DYNAMIC_PLUGIN")
    target_include_directories(${plugin_name} PUBLIC $<TARGET_PROPERTY:Corrade::PluginManager,INTERFACE_INCLUDE_DIRECTORIES>)

    # Plugins don't have any prefix (e.g. 'lib' on Linux)
    set_target_properties(${plugin_name} PROPERTIES PREFIX "")

    # Enable incremental linking on the Mac macOS
    if(CORRADE_TARGET_APPLE)
        set_target_properties(${plugin_name} PROPERTIES
            LINK_FLAGS "-undefined dynamic_lookup")
    endif()

    # Copy metadata next to the binary for testing purposes or install it both
    # somewhere
    if(debug_install_dirs STREQUAL CMAKE_CURRENT_BINARY_DIR)
        add_custom_command(
            OUTPUT ${plugin_name}.conf
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${metadata_file} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${plugin_name}.conf
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${metadata_file})
        add_custom_target(${plugin_name}-metadata ALL
            DEPENDS ${plugin_name}.conf
            # Force IDEs display also the metadata file in project view
            SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${metadata_file})
    else()
        # Force IDEs display also the metadata file in project view
        add_custom_target(${plugin_name}-metadata SOURCES ${metadata_file})

        # CONFIGURATIONS must be first in order to not be ignored when having
        # multiple destinations.
        # https://gitlab.kitware.com/cmake/cmake/issues/16361
        install(TARGETS ${plugin_name}
            CONFIGURATIONS Debug
            RUNTIME DESTINATION ${debug_binary_install_dir}
            LIBRARY DESTINATION ${debug_library_install_dir}
            ARCHIVE DESTINATION ${debug_library_install_dir})
        install(TARGETS ${plugin_name}
            CONFIGURATIONS "" None Release RelWithDebInfo MinSizeRel
            RUNTIME DESTINATION ${release_binary_install_dir}
            LIBRARY DESTINATION ${release_library_install_dir}
            ARCHIVE DESTINATION ${release_library_install_dir})
        install(FILES ${metadata_file} DESTINATION ${debug_conf_install_dir}
            RENAME "${plugin_name}.conf"
            CONFIGURATIONS Debug)
        install(FILES ${metadata_file} DESTINATION ${release_conf_install_dir}
            RENAME "${plugin_name}.conf"
            CONFIGURATIONS "" None Release RelWithDebInfo MinSizeRel)
    endif()
endfunction()

function(corrade_add_static_plugin plugin_name install_dirs metadata_file)
    # Populate library_install_dir variable
    list(LENGTH install_dirs install_dir_count)
    if(install_dir_count EQUAL 1)
        set(library_install_dir ${install_dirs})
    elseif(install_dir_count EQUAL 2)
        list(GET install_dirs 1 library_install_dir)
    else()
        message(FATAL_ERROR "corrade_add_static_plugin(): install dir must contain either just library location or both library and binary location")
    endif()

    # Compile resources
    set(resource_file "${CMAKE_CURRENT_BINARY_DIR}/resources_${plugin_name}.conf")
    file(WRITE "${resource_file}" "group=CorradeStaticPlugin_${plugin_name}\n[file]\nfilename=\"${CMAKE_CURRENT_SOURCE_DIR}/${metadata_file}\"\nalias=${plugin_name}.conf")
    corrade_add_resource(${plugin_name} "${resource_file}")

    # Create static library and bring all needed options along
    add_library(${plugin_name} STATIC ${ARGN} ${${plugin_name}})
    set_target_properties(${plugin_name} PROPERTIES CORRADE_CXX_STANDARD 11)
    target_compile_definitions(${plugin_name} PRIVATE "CORRADE_STATIC_PLUGIN")
    target_include_directories(${plugin_name} PUBLIC $<TARGET_PROPERTY:Corrade::PluginManager,INTERFACE_INCLUDE_DIRECTORIES>)

    set_target_properties(${plugin_name} PROPERTIES DEBUG_POSTFIX "-d")

    # Install, if not into the same place
    if(NOT install_dirs STREQUAL CMAKE_CURRENT_BINARY_DIR)
        install(TARGETS ${plugin_name} DESTINATION ${library_install_dir})
    endif()
endfunction()

if(CORRADE_TARGET_WINDOWS)
function(corrade_find_dlls_for_libs result)
    set(dlls )
    foreach(lib ${ARGN})
        get_filename_component(lib_dir ${lib} DIRECTORY)
        get_filename_component(lib_name ${lib} NAME_WE)
        find_file(CORRADE_DLL_FOR_${lib_name} ${lib_name}.dll HINTS ${lib_dir}/../bin)
        list(APPEND dlls ${CORRADE_DLL_FOR_${lib_name}})
    endforeach()
    set(${result} ${dlls} PARENT_SCOPE)
endfunction()
endif()

set(_CORRADE_USE_INCLUDED TRUE)
