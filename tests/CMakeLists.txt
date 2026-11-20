configure_file(test_doc.json test_doc.json)
configure_file(test_index.json test_index.json)
configure_file(test_index2.json test_index2.json)

include(GoogleTest)

# Offline tests: no Couchbase cluster required.
add_executable(search_index_definition_tests search_index_definition_tests.cpp)
target_link_libraries(search_index_definition_tests PRIVATE GTest::gtest_main search_index_definition)
gtest_discover_tests(search_index_definition_tests WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_executable(u_tests tests.cpp)
target_link_libraries(
    u_tests
    PRIVATE
    GTest::gtest_main
    cxx_quickstart_lib
)
target_include_directories(u_tests PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/include"
                          "${PROJECT_SOURCE_DIR}"
                          )



include(GoogleTest)
gtest_discover_tests(u_tests)