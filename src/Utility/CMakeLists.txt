set(Map2X_Utility_SRCS
    Directory.cpp
)

add_library(Map2XUtility STATIC ${Map2X_Utility_SRCS})

if(BUILD_TESTS)
    add_subdirectory(Test)
endif()
