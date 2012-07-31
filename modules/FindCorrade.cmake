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
# If Corrade library is found, modules CorradeMacros and CorradeLibSuffix are
# included.
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

if(NOT CORRADE_FOUND)
    return()
endif()

include(CorradeMacros)
include(CorradeLibSuffix)
set(CORRADE_LIBRARIES ${CORRADE_UTILITY_LIBRARY} ${CORRADE_PLUGINMANAGER_LIBRARY})
