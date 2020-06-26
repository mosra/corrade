#!/usr/bin/env python
# -*- coding: utf-8 -*-

from conans import ConanFile, CMake, tools
from conans.errors import ConanException
import os
import shutil


def sort_libs(correct_order, libs, lib_suffix='', reverse_result=False):
    # Add suffix for correct string matching
    correct_order[:] = [s.__add__(lib_suffix) for s in correct_order]

    result = []
    for expectedLib in correct_order:
        for lib in libs:
            if expectedLib == lib:
                result.append(lib)

    if reverse_result:
        # Linking happens in reversed order
        result.reverse()

    return result


class CorradeConan(ConanFile):
    name = "corrade"
    version = "2020.06"
    description = "Corrade is a multiplatform utility library written \
                    in C++11/C++14. It's used as a base for the Magnum \
                    graphics engine, among other things."
    # topics can get used for searches, GitHub topics, Bintray tags etc. Add here keywords about the library
    topics = ("conan", "corrad", "magnum", "filesystem", "console", "environment", "os")
    url = "https://github.com/mosra/corrade"
    homepage = "https://magnum.graphics/corrade"
    author = "helmesjo <helmesjo@gmail.com>"
    license = "MIT"  # Indicates license type of the packaged library; please use SPDX Identifiers https://spdx.org/licenses/
    exports = ["COPYING"]
    exports_sources = ["CMakeLists.txt", "src/*", "package/conan/*", "modules/*"]
    generators = "cmake"
    short_paths = True  # Some folders go out of the 260 chars path length scope (windows)

    # Options may need to change depending on the packaged library.
    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "build_deprecated": [True, False],
        "with_interconnect": [True, False],
        "with_pluginmanager": [True, False],
        "with_rc": [True, False],
        "with_testsuite": [True, False],
        "with_utility": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "build_deprecated": True,
        "with_interconnect": True,
        "with_pluginmanager": True,
        "with_rc": True,
        "with_testsuite": True,
        "with_utility": True,
    }

    _build_subfolder = "build_subfolder"

    def config_options(self):
        if self.settings.os == 'Windows':
            del self.options.fPIC

    def configure(self):
        if self.settings.compiler == 'Visual Studio' and int(self.settings.compiler.version.value) < 14:
            raise ConanException("{} requires Visual Studio version 14 or greater".format(self.name))

    def source(self):
        # Wrap the original CMake file to call conan_basic_setup
        shutil.move("CMakeLists.txt", "CMakeListsOriginal.txt")
        shutil.move(os.path.join("package", "conan", "CMakeLists.txt"), "CMakeLists.txt")

    def _configure_cmake(self):
        cmake = CMake(self)

        def add_cmake_option(option, value):
            var_name = "{}".format(option).upper()
            value_str = "{}".format(value)
            var_value = "ON" if value_str == 'True' else "OFF" if value_str == 'False' else value_str
            cmake.definitions[var_name] = var_value

        for option, value in self.options.items():
            add_cmake_option(option, value)

        # Corrade uses suffix on the resulting 'lib'-folder when running cmake.install()
        # Set it explicitly to empty, else Corrade might set it implicitly (eg. to "64")
        add_cmake_option("LIB_SUFFIX", "")

        add_cmake_option("BUILD_STATIC", not self.options.shared)

        if self.settings.compiler == 'Visual Studio':
            add_cmake_option("MSVC2015_COMPATIBILITY", int(self.settings.compiler.version.value) == 14)
            add_cmake_option("MSVC2017_COMPATIBILITY", int(self.settings.compiler.version.value) == 15)
            add_cmake_option("MSVC2019_COMPATIBILITY", int(self.settings.compiler.version.value) == 16)

        cmake.configure(build_folder=self._build_subfolder)

        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        self.copy("COPYING", dst="licenses", src=".")
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        # See dependency order here: https://doc.magnum.graphics/magnum/custom-buildsystems.html
        allLibs = [
            #1
            "CorradeUtility",
            "CorradeContainers",
            #2
            "CorradeInterconnect",
            "CorradePluginManager",
            "CorradeTestSuite",
        ]

        # Sort all built libs according to above, and reverse result for correct link order
        suffix = '-d' if self.settings.build_type == "Debug" else ''
        builtLibs = tools.collect_libs(self)
        self.cpp_info.libs = sort_libs(correct_order=allLibs, libs=builtLibs, lib_suffix=suffix, reverse_result=True)
