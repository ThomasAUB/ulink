cmake_minimum_required(VERSION 3.13)        # CMake version check
project(uLL VERSION 0.0.1 LANGUAGES CXX)    # Create project "simple_example"
set(CMAKE_CXX_STANDARD 17)                  # Enable c++17 standard



if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    # Prepare doctest for other targets to use
    # find_package(doctest REQUIRED)

    include(CTest)

    add_executable(ullTests tests/tests.cpp)
    # target_link_libraries(ullTests PRIVATE doctest::doctest)

    # add_test(NAME ullTests COMMAND $<TARGET_FILE:tests/tests.cpp>)

endif()


if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
endif()


# add_test(NAME ullTests COMMAND tests.cpp)

# Add main.cpp file of the project root directory as a source file
# set(SOURCE_FILES main.cpp)



# Add executable target with source files listed in SOURCE_FILES variable
# add_executable(ull ${SOURCE_FILES})