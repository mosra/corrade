
# (the blank line is here so CMake doesn't generate documentation from it)

#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>
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

# Quoted variables should not be dereferenced (affects the "MSVC" string)
if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

# Check compiler version
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Don't allow to use compilers older than what compatibility mode allows
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.1")
        message(FATAL_ERROR "Corrade cannot be used with GCC < 4.8.1. Sorry.")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
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
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.30")
        if(NOT CORRADE_MSVC2019_COMPATIBILITY)
            message(FATAL_ERROR "To use Corrade with MSVC 2019, build it with MSVC2019_COMPATIBILITY enabled")
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
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR (CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?Clang" AND NOT CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC") OR CORRADE_TARGET_EMSCRIPTEN)
    set(CORRADE_PEDANTIC_COMPILER_OPTIONS
        "-Wall" "-Wextra"
        "$<$<STREQUAL:$<TARGET_PROPERTY:LINKER_LANGUAGE>,CXX>:-Wold-style-cast>"
        "-Winit-self"
        "-Werror=return-type"
        "-Wmissing-declarations"
        "-pedantic"
        # Needs to have both, otherwise Clang's linker on macOS complains that
        # "direct access in function [...] to global weak symbol [...] means the
        # weak symbol cannot be overridden at runtime. This was likely caused
        # by different translation units being compiled with different
        # visibility settings." See also various google results for the above
        # message.
        "-fvisibility=hidden" "-fvisibility-inlines-hidden")

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

        list(APPEND CORRADE_PEDANTIC_TEST_COMPILER_OPTIONS
            # Unlike GCC, -Wunused-function (which is enabled through -Wall)
            # doesn't fire for member functions, it's controlled separately
            "-Wunused-member-function"
            # This is implicitly enabled by the above and causes lots of
            # warnings for e.g. move constructors, so disabling
            "-Wno-unneeded-member-function")
    endif()

# MSVC-specific compiler flags
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
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

# In order to avoid clashes with builtin CMake features, we won't add the
# standard flag in case the CXX_STANDARD property is present. Additionally,
# since CMake 3.15, the cxx_std_14 compile feature doesn't result in any flag
# being added to compiler command-line if C++14 is a default on given compiler
# (such as GCC 6 and up). That unfortunately means ours default flag (-std=c++11)
# gets set, making it look like the COMPILE_FEATURES didn't work at all. To
# circumvent that, the CORRADE_CXX_STANDARD isn't set if anything from
# COMPILE_FEATURES is present either.
set(_CORRADE_CXX_STANDARD_ONLY_IF_NOT_ALREADY_SET
    "$<STREQUAL:$<TARGET_PROPERTY:LINKER_LANGUAGE>,CXX>,$<NOT:$<BOOL:$<TARGET_PROPERTY:CXX_STANDARD>>>,$<NOT:$<BOOL:$<TARGET_PROPERTY:COMPILE_FEATURES>>>")

# Enable C++11/14/17/2a on GCC/Clang if CORRADE_CXX_STANDARD is set. In
# addition to the above it also does nothing in case the user put "-std=" in
# CMAKE_CXX_FLAGS. It doesn't cover adding flags using target_compile_options(),
# though.
if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR (CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?Clang" AND NOT CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC") OR CORRADE_TARGET_EMSCRIPTEN) AND NOT CMAKE_CXX_FLAGS MATCHES "-std=")
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9) OR ((CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?Clang" OR CORRADE_TARGET_EMSCRIPTEN) AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.5))
        set(_CORRADE_CXX14_STANDARD_FLAG "-std=c++14")
    else()
        set(_CORRADE_CXX14_STANDARD_FLAG "-std=c++1y")
    endif()

    set_property(DIRECTORY APPEND PROPERTY COMPILE_OPTIONS
        "$<$<AND:${_CORRADE_CXX_STANDARD_ONLY_IF_NOT_ALREADY_SET},$<STREQUAL:$<TARGET_PROPERTY:CORRADE_CXX_STANDARD>,11>>:-std=c++11>"
        "$<$<AND:${_CORRADE_CXX_STANDARD_ONLY_IF_NOT_ALREADY_SET},$<STREQUAL:$<TARGET_PROPERTY:CORRADE_CXX_STANDARD>,14>>:${_CORRADE_CXX14_STANDARD_FLAG}>"
        # TODO: change to C++17 when compiler support is widespread enough
        "$<$<AND:${_CORRADE_CXX_STANDARD_ONLY_IF_NOT_ALREADY_SET},$<STREQUAL:$<TARGET_PROPERTY:CORRADE_CXX_STANDARD>,17>>:-std=c++1z>"
        "$<$<AND:${_CORRADE_CXX_STANDARD_ONLY_IF_NOT_ALREADY_SET},$<STREQUAL:$<TARGET_PROPERTY:CORRADE_CXX_STANDARD>,20>>:-std=c++2a>")
endif()

# Enable C++14/17/2a on MSVC if CORRADE_CXX_STANDARD is set. C++11 is present
# implicitly, 14, 17 and 20 has to be enabled through a flag. Does nothing in
# case the user specified CXX_STANDARD property or put "/std:" or "-std:" in
# CMAKE_CXX_FLAGS. It doesn't cover adding flags using target_compile_options(),
# though.
if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC") AND NOT CMAKE_CXX_FLAGS MATCHES "[-/]std:")
    set_property(DIRECTORY APPEND PROPERTY COMPILE_OPTIONS
        "$<$<AND:${_CORRADE_CXX_STANDARD_ONLY_IF_NOT_ALREADY_SET},$<STREQUAL:$<TARGET_PROPERTY:CORRADE_CXX_STANDARD>,14>>:/std:c++14>"
        "$<$<AND:${_CORRADE_CXX_STANDARD_ONLY_IF_NOT_ALREADY_SET},$<STREQUAL:$<TARGET_PROPERTY:CORRADE_CXX_STANDARD>,17>>:/std:c++17>"
        # TODO: change to c++20? when such flag appears
        # https://docs.microsoft.com/en-us/cpp/build/reference/std-specify-language-standard-version?view=vs-2019
        "$<$<AND:${_CORRADE_CXX_STANDARD_ONLY_IF_NOT_ALREADY_SET},$<STREQUAL:$<TARGET_PROPERTY:CORRADE_CXX_STANDARD>,20>>:/std:c++latest>")
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
        elseif(arg STREQUAL ARGUMENTS)
            set(_DOING_LIBRARIES OFF)
            set(_DOING_FILES OFF)
            set(_DOING_ARGUMENTS ON)
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
                list(APPEND absolute_files ${input_filename})
            elseif(_DOING_ARGUMENTS)
                list(APPEND arguments ${arg})
            else()
                list(APPEND sources ${arg})
            endif()
        endif()
    endforeach()

    if(CORRADE_TESTSUITE_TARGET_XCTEST)
        add_library(${test_name} SHARED ${sources})
        set_target_properties(${test_name} PROPERTIES FRAMEWORK TRUE)
        # This is never Windows, so no need to bother with Corrade::Main
        target_link_libraries(${test_name} PRIVATE ${libraries} Corrade::TestSuite)

        set(test_runner_file ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.mm)
        configure_file(${CORRADE_TESTSUITE_XCTEST_RUNNER}
                       ${test_runner_file})
        xctest_add_bundle(${test_name}Runner ${test_name} ${test_runner_file})
        if(CORRADE_TARGET_IOS)
            # The EFFECTIVE_PLATFORM_NAME variable is not expanded when using
            # TARGET_* generator expressions on iOS, we need to hardcode it
            # manually. See http://public.kitware.com/pipermail/cmake/2016-March/063049.html
            # In case we redirect the runtime output directory, use that (and
            # assume there's no TARGET_* generator expression). This will of
            # course break when someone sets the LIBRARY_OUTPUT_DIRECTORY
            # property of the target, but that didn't work before either.
            if(CMAKE_LIBRARY_OUTPUT_DIRECTORY)
                add_test(NAME ${test_name} COMMAND ${XCTest_EXECUTABLE} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${test_name}Runner.xctest)
            else()
                add_test(NAME ${test_name} COMMAND ${XCTest_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>${_CORRADE_EFFECTIVE_PLATFORM_NAME}/${test_name}Runner.xctest)
            endif()
        else()
            xctest_add_test(${test_name} ${test_name}Runner)
        endif()
        if(arguments)
            message(WARNING "corrade_add_test() ARGUMENTS are not supported when CORRADE_TESTSUITE_TARGET_XCTEST is enabled")
        endif()
    else()
        add_executable(${test_name} ${sources})
        target_link_libraries(${test_name} PRIVATE ${libraries} Corrade::TestSuite Corrade::Main)

        # Run tests using Node.js on Emscripten
        if(CORRADE_TARGET_EMSCRIPTEN)
            # Emscripten needs to have exceptions enabled for TestSuite to work
            # properly. See TestSuite CMakeLists for further information.
            set_property(TARGET ${test_name} APPEND_STRING PROPERTY COMPILE_FLAGS " -s DISABLE_EXCEPTION_CATCHING=0")
            set_property(TARGET ${test_name} APPEND_STRING PROPERTY LINK_FLAGS " -s DISABLE_EXCEPTION_CATCHING=0")
            find_package(NodeJs REQUIRED)
            add_test(NAME ${test_name} COMMAND NodeJs::NodeJs --stack-trace-limit=0 $<TARGET_FILE:${test_name}> ${arguments})

            # Embed all files
            foreach(file ${files})
                string(REGEX REPLACE ${_corrade_file_pair_match} "${_corrade_file_pair_replace}" file_pair ${file})
                list(GET file_pair 0 input_filename)
                list(GET file_pair 1 output_filename)
                emscripten_embed_file(${test_name} ${input_filename} "/${output_filename}")
            endforeach()

            # Generate the runner file, first replacing ${test_name} with
            # configure_file() and then copying that into the final location.
            # Two steps because file(GENERATE) can't replace variables while
            # configure_file() can't have generator expressions in the path.
            configure_file(${CORRADE_TESTSUITE_EMSCRIPTEN_RUNNER}
                           ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.html)
            file(GENERATE OUTPUT $<TARGET_FILE_DIR:${test_name}>/${test_name}.html
                INPUT ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.html)

        # Run tests using ADB on Android
        elseif(CORRADE_TARGET_ANDROID)
            # The executables need to be PIE
            target_compile_options(${test_name} PRIVATE "-fPIE")
            set_property(TARGET ${test_name} APPEND_STRING PROPERTY LINK_FLAGS "-fPIE -pie")
            # All files will be copied to the target when the test is run. The
            # arguments are passed together with the filename, at the moment it
            # will fail for arguments with spaces
            string(REPLACE ";" " " arguments_str "${arguments}")
            add_test(NAME ${test_name} COMMAND ${CORRADE_TESTSUITE_ADB_RUNNER} $<TARGET_FILE_DIR:${test_name}> "$<TARGET_FILE_NAME:${test_name}> ${arguments_str}" ${files})

        # Run tests natively elsewhere
        else()
            add_test(NAME ${test_name} COMMAND ${test_name} ${arguments})
        endif()

        # iOS-specific
        if(CORRADE_TARGET_IOS)
            set_target_properties(${test_name} PROPERTIES
                MACOSX_BUNDLE ON
                MACOSX_BUNDLE_GUI_IDENTIFIER ${CORRADE_TESTSUITE_BUNDLE_IDENTIFIER_PREFIX}.${test_name}
                XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "YES")
        endif()
    endif()

    set_property(TARGET ${test_name} APPEND PROPERTY COMPILE_OPTIONS "$<$<BOOL:$<TARGET_PROPERTY:CORRADE_USE_PEDANTIC_FLAGS>>:${CORRADE_PEDANTIC_TEST_COMPILER_OPTIONS}>")

    # Add the file to list of required files for given test case
    set_tests_properties(${test_name} PROPERTIES REQUIRED_FILES "${absolute_files}")
endfunction()

function(corrade_add_resource name configurationFile)
    # Parse dependencies from the file
    set(dependencies )
    set(filenameRegex "^[ \t]*filename[ \t]*=[ \t]*\"?([^\"]+)\"?[ \t]*$")
    get_filename_component(configurationFilePath ${configurationFile} PATH)

    file(STRINGS "${configurationFile}" files REGEX ${filenameRegex} ENCODING UTF-8)
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

    # Create dynamic library and bring all needed options along. On Windows a
    # DLL cannot have undefined references, so we need to link against all its
    # dependencies *at compile time*, as opposed to runtime like in all sane
    # systems. But when using add_library(MODULE), CMake disallows linking
    # MODULEs to MODULEs (when dealing with inter-plugin dependencies, for
    # example) -- probably because, on Windows, creating a MODULE doesn't
    # create the corresponding import lib for it. So we work around that by
    # using SHARED on Windows.
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

    # Force IDEs display also the resource files in project view
    add_custom_target(${plugin_name}-metadata SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${metadata_file})

    # Copy metadata next to the binary so tests and CMake subprojects can use
    # it as well
    add_custom_command(TARGET ${plugin_name} POST_BUILD
        # This would be nice to Ninja, but BYPRODUCTS don't support generator
        # expressions right now (last checked: CMake 3.16)
        #BYPRODUCTS $<TARGET_FILE_DIR:${plugin_name}>/${plugin_name}.conf
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${metadata_file} $<TARGET_FILE_DIR:${plugin_name}>/${plugin_name}.conf
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${metadata_file} ${name}-metadata)

    # Install it somewhere, unless that's explicitly not wanted
    if(NOT debug_install_dirs STREQUAL CMAKE_CURRENT_BINARY_DIR)
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
