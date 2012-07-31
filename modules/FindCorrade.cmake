# - Find Corrade
#
# Basic usage:
#
#  find_package(Corrade [REQUIRED])
#
# This module tries to find Corrade library and then defines:
#
#  CORRADE_FOUND                    - True if Corrade library is found
#
#  CORRADE_INCLUDE_DIR              - Include dir for Corrade
#  CORRADE_LIBRARIES                - All Corrade libraries
#  CORRADE_UTILITY_LIBRARY          - Corrade Utility library
#  CORRADE_PLUGINMANAGER_LIBRARY    - Corrade Plugin manager library
#  CORRADE_RC_EXECUTABLE            - Corrade resource compiler executable
#
# Additionally these variables are defined for internal usage:
#
#  CORRADE_BINARY_INSTALL_DIR       - Binary installation directory
#  CORRADE_LIBRARY_INSTALL_DIR      - Library installation directory
#  CORRADE_CMAKE_MODULE_INSTALL_DIR - Installation dir for CMake modules
#  CORRADE_INCLUDE_INSTALL_DIR      - Include installation directory for Corrade headers
#

# Libraries
find_library(CORRADE_UTILITY_LIBRARY CorradeUtility)
find_library(CORRADE_PLUGINMANAGER_LIBRARY CorradePluginManager)

# RC executable
find_program(CORRADE_RC_EXECUTABLE corrade-rc)

# Paths
find_path(CORRADE_INCLUDE_DIR
    NAMES PluginManager Utility
    PATH_SUFFIXES Corrade)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Corrade DEFAULT_MSG
    CORRADE_INCLUDE_DIR
    CORRADE_UTILITY_LIBRARY
    CORRADE_PLUGINMANAGER_LIBRARY
    CORRADE_RC_EXECUTABLE)

if(CORRADE_FOUND)
    include(CorradeMacros)

    include(CorradeLibSuffix)
    set_parent_scope(CORRADE_BINARY_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin)
    set_parent_scope(CORRADE_LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
    set_parent_scope(CORRADE_CMAKE_MODULE_INSTALL_DIR ${CMAKE_ROOT}/Modules)
    set_parent_scope(CORRADE_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/Corrade)

    set_parent_scope(CORRADE_LIBRARIES ${CORRADE_UTILITY_LIBRARY} ${CORRADE_PLUGINMANAGER_LIBRARY})
endif()
