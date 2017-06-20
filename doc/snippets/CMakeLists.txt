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

# TODO: causes spurious linker errors on Travis iOS build, so I'm disabling it
if(WITH_TESTSUITE AND NOT CORRADE_TARGET_IOS)
    find_package(Threads REQUIRED)

    # TestSuite documentation snippets. I need them executable so I can
    # copy&paste the output to the documentation. Also mot using
    # corrade_add_test() because these tests shouldn't be run as part of CTest
    # -- some of them fail on purpose
    add_executable(testsuite-basic testsuite-basic.cpp)
    add_executable(testsuite-templated testsuite-templated.cpp)
    add_executable(testsuite-repeated testsuite-repeated.cpp)
    add_executable(testsuite-benchmark testsuite-benchmark.cpp)
    add_executable(testsuite-benchmark-custom testsuite-benchmark-custom.cpp)
    set_target_properties(
        testsuite-basic
        testsuite-templated
        testsuite-repeated
        testsuite-benchmark
        testsuite-benchmark-custom
        PROPERTIES FOLDER "Corrade/doc/snippets")

    target_link_libraries(testsuite-basic CorradeTestSuite)
    target_link_libraries(testsuite-templated CorradeTestSuite)
    target_link_libraries(testsuite-repeated CorradeTestSuite ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(testsuite-benchmark CorradeTestSuite)
    target_link_libraries(testsuite-benchmark-custom CorradeTestSuite)

    if(NOT CORRADE_TARGET_ANDROID)
        add_executable(testsuite-instanced testsuite-instanced.cpp)
        target_link_libraries(testsuite-instanced CorradeTestSuite)
        set_target_properties(testsuite-instanced PROPERTIES FOLDER "Corrade/doc/snippets")
    endif()
endif()
