set(ULL_UNIT_TESTS ull_tests)

include(CTest)

file(GLOB TARGET_SRC "./tests.cpp" )

add_executable(${ULL_UNIT_TESTS} ${TARGET_SRC})

add_test(${ULL_UNIT_TESTS} ${ULL_UNIT_TESTS})