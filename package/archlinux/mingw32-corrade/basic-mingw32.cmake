#
# Toolchain for crosscompiling basic application for Windows on ArchLinux.
#
# Minimal dependencies:
#
#     mingw32-gcc
#

# Target system name
SET(CMAKE_SYSTEM_NAME   Windows)

# Compilers and utilities
SET(CMAKE_C_COMPILER    i486-mingw32-gcc)
SET(CMAKE_CXX_COMPILER  i486-mingw32-g++)
SET(CMAKE_STRIP         i486-mingw32-strip)
SET(CMAKE_RC_COMPILER   i486-mingw32-windres)

# Croscompiler path
SET(CMAKE_FIND_ROOT_PATH /usr/i486-mingw32)

# Find executables in root path, libraries and includes are in crosscompiler
# path
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# FindQt4.cmake asks native QMake for variables, which surely cannot work when
# crosscompiling.
set(MINGW_PREFIX        /usr/i486-mingw32)

# Directories where to search for DLLs
set(DLL_SEARCH_PATH
    ${MINGW_PREFIX}/lib
    ${MINGW_PREFIX}/bin
)
