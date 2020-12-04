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

corrade_add_test(ContainersArrayTest ArrayTest.cpp)
corrade_add_test(ContainersArrayTupleTest ArrayTupleTest.cpp LIBRARIES CorradeUtilityTestLib)
corrade_add_test(ContainersArrayViewTest ArrayViewTest.cpp)
corrade_add_test(ContainersArrayViewStlTest ArrayViewStlTest.cpp)
corrade_add_test(ContainersEnumSetTest EnumSetTest.cpp)

corrade_add_test(ContainersGrowableArrayTest GrowableArrayTest.cpp)
if(CORRADE_TARGET_EMSCRIPTEN)
    # We need to append to avoid overwriting -s DISABLE_EXCEPTION_CATCHING=0
    # TODO: somehow, for std::vector the available memory is enough?
    set_property(TARGET ContainersGrowableArrayTest APPEND_STRING PROPERTY LINK_FLAGS " -s ALLOW_MEMORY_GROWTH=1")
endif()
corrade_add_test(ContainersGrowableArraySani___FailTest GrowableArraySanitizerFailTest.cpp)
# Not matching with the -f, as MSVC might use either /f or -f. Source:
# https://devblogs.microsoft.com/cppblog/addresssanitizer-asan-for-windows-with-msvc/
if(CMAKE_CXX_FLAGS MATCHES "fsanitize=address")
    set_tests_properties(ContainersGrowableArraySani___FailTest PROPERTIES
        PASS_REGULAR_EXPRESSION "AddressSanitizer: container-overflow")
endif()

corrade_add_test(ContainersLinkedListTest LinkedListTest.cpp)
corrade_add_test(ContainersOptionalTest OptionalTest.cpp)
corrade_add_test(ContainersPointerTest PointerTest.cpp)
corrade_add_test(ContainersPointerStlTest PointerStlTest.cpp)
corrade_add_test(ContainersRawForwardListTest RawForwardListTest.cpp)
corrade_add_test(ContainersReferenceTest ReferenceTest.cpp)
corrade_add_test(ContainersReferenceStlTest ReferenceStlTest.cpp)
corrade_add_test(ContainersScopeGuardTest ScopeGuardTest.cpp)
corrade_add_test(ContainersStaticArrayTest StaticArrayTest.cpp)
corrade_add_test(ContainersStaticArrayViewTest StaticArrayViewTest.cpp)
corrade_add_test(ContainersStaticArrayViewStlTest StaticArrayViewStlTest.cpp)
corrade_add_test(ContainersStridedArrayViewTest StridedArrayViewTest.cpp)
corrade_add_test(ContainersStringTest StringTest.cpp LIBRARIES CorradeUtilityTestLib)
corrade_add_test(ContainersStringStlTest StringStlTest.cpp)
corrade_add_test(ContainersStringViewTest StringViewTest.cpp LIBRARIES CorradeUtilityTestLib)
corrade_add_test(ContainersStringViewStlTest StringViewStlTest.cpp)
corrade_add_test(ContainersTagsTest TagsTest.cpp)

set_property(TARGET
    ContainersLinkedListTest
    ContainersArrayTest
    ContainersArrayViewTest
    ContainersArrayViewStlTest
    ContainersGrowableArrayTest
    ContainersOptionalTest
    ContainersPointerTest
    ContainersStaticArrayViewTest
    ContainersStridedArrayViewTest
    ContainersStringTest
    ContainersStringViewTest
    APPEND PROPERTY COMPILE_DEFINITIONS "CORRADE_GRACEFUL_ASSERT")

set_target_properties(
    ContainersArrayTest
    ContainersArrayViewTest
    ContainersEnumSetTest
    ContainersLinkedListTest
    ContainersPointerTest
    ContainersPointerStlTest
    ContainersRawForwardListTest
    ContainersReferenceTest
    ContainersReferenceStlTest
    ContainersScopeGuardTest
    ContainersStaticArrayTest
    ContainersStaticArrayViewTest
    ContainersStridedArrayViewTest
    ContainersStringTest
    ContainersStringStlTest
    ContainersStringViewTest
    ContainersStringViewStlTest
    ContainersTagsTest
    PROPERTIES FOLDER "Corrade/Containers/Test")

# Build these only if there's no explicit -std= passed in the flags
if(NOT CMAKE_CXX_FLAGS MATCHES "-std=")
    # Copied verbatim from src/Corrade/Test/CMakeLists.txt, please keep in sync
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0") OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0") OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.3") OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.10"))
        corrade_add_test(ContainersOptionalStlTest OptionalStlTest.cpp)
        set_target_properties(ContainersOptionalStlTest PROPERTIES
            CORRADE_CXX_STANDARD 17
            FOLDER "Corrade/Containers/Test")
    endif()

    # Copied verbatim from src/Corrade/Test/CMakeLists.txt, please keep in sync
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0") OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.0") OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10.0") OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.15"))
        corrade_add_test(ContainersArrayStlSpanTest ArrayStlSpanTest.cpp)
        corrade_add_test(ContainersArrayViewStlSpanTest ArrayViewStlSpanTest.cpp)
        corrade_add_test(ContainersStaticArrayStlSpanTest StaticArrayStlSpanTest.cpp)
        corrade_add_test(ContainersStaticArrayViewStlSpanTest StaticArrayViewStlSpanTest.cpp)
        set_target_properties(
            ContainersArrayStlSpanTest
            ContainersArrayViewStlSpanTest
            ContainersStaticArrayStlSpanTest
            ContainersStaticArrayViewStlSpanTest
            PROPERTIES
            CORRADE_CXX_STANDARD 20
            FOLDER "Corrade/Containers/Test")
    endif()
endif()
