cmake_minimum_required(VERSION 3.0.0)
project(uLLTests VERSION 0.1.0)

include(CTest)
enable_testing()

file(GLOB TARGET_SRC "./tests.cpp" )

add_executable(tests ${TARGET_SRC})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

