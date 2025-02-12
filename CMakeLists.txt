cmake_minimum_required(VERSION 3.19)

project(CxxQuickStart CXX)
find_package(fmt REQUIRED)
include(cmake/CPM.cmake)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
configure_file(hotel_search_index.json hotel_search_index.json)
configure_file(hotel_search_index.json tests/hotel_search_index.json)
configure_file(tests/test_doc.json test_doc.json)
configure_file(tests/test_index.json test_index.json)
configure_file(tests/test_index2.json test_index2.json)

CPMAddPackage(
  NAME
  couchbase_cxx_client
  GIT_TAG
  1.0.5
  VERSION
  1.0.5
  GITHUB_REPOSITORY
  "couchbase/couchbase-cxx-client"
  OPTIONS
  "COUCHBASE_CXX_CLIENT_INSTALL ON"
  "COUCHBASE_CXX_CLIENT_BUILD_STATIC OFF"
  "COUCHBASE_CXX_CLIENT_BUILD_SHARED ON"
)


include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(cxx_quickstart_lib cxx_client.cpp)
target_link_libraries(cxx_quickstart_lib couchbase_cxx_client::couchbase_cxx_client taocpp::json)
target_include_directories(cxx_quickstart_lib PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/include"
                          )


add_executable(cxx_quickstart_ex main.cpp)
target_link_libraries(cxx_quickstart_ex cxx_quickstart_lib)
target_include_directories(cxx_quickstart_ex PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/include"
                          )

add_subdirectory(tests)