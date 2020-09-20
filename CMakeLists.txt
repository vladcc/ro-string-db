cmake_minimum_required(VERSION 3.16.3)
project(ro-string-db VERSION 1.0)

set(ROOTD "ro-string-db")

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -Wfatal-errors")

include_directories(
	${ROOTD}/input
	${ROOTD}/matrix
	${ROOTD}/query_driver
	${ROOTD}/ro_string_db
	${ROOTD}/ro_string_table
	${ROOTD}/sort_vector
	${ROOTD}/string_pool
)

set(ALL_PROD_CPP
	${ROOTD}/input/input.cpp
	${ROOTD}/matrix/matrix.ipp
	${ROOTD}/ro_string_db/ro_string_db.cpp
	${ROOTD}/ro_string_table/ro_string_table.cpp
	${ROOTD}/sort_vector/sort_vector.ipp
	${ROOTD}/string_pool/string_pool.hpp
)

set(LIB_STATIC "ro_string_db_static")
add_library(
	${LIB_STATIC} STATIC
	${ALL_PROD_CPP}
)

set(LIB_SHARED "ro_string_db_shared")
add_library(
	${LIB_SHARED} SHARED
	${ALL_PROD_CPP}
)

set(QUERY_DRIVER "query-driver")
add_executable(
	${QUERY_DRIVER}
	${ROOTD}/query_driver/parse_opts.c
	${ROOTD}/query_driver/self_stat.c
	${ROOTD}/query_driver/query_driver.cpp
)
target_link_libraries(
	${QUERY_DRIVER} PRIVATE
	${LIB_STATIC}
)

set(ALL_TESTS "all-tests")
set(ALL_TEST_CPP
	${ROOTD}/ro_string_table/test_ro_string_table.cpp
	${ROOTD}/matrix/test_matrix.cpp
	${ROOTD}/string_pool/test_string_pool.cpp
	${ROOTD}/ro_string_db/test_ro_string_db.cpp
	${ROOTD}/input/test_input.cpp
	${ROOTD}/sort_vector/test_sort_vector.cpp
)

add_executable(
	${ALL_TESTS}
	${ALL_TEST_CPP}
	${ALL_PROD_CPP}
	${ROOTD}/test/test_all.cpp
)
