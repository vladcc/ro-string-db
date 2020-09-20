#include "test_ro_string_table.hpp"
#include "test_matrix.hpp"
#include "test_string_pool.hpp"
#include "test_ro_string_db.hpp"
#include "test_input.hpp"
#include "test_sort_vector.hpp"

#include <cstdio>

typedef struct test_funs {
    void (*run_tests)(void);
    int (*get_passed)(void);
    int (*get_failed)(void);
} test_funs;

static test_funs all_tests[] = {
	{run_test_ro_string_table, test_ro_string_table_passed, test_ro_string_table_failed},
	{run_test_matrix, test_matrix_passed, test_matrix_failed},
	{run_test_string_pool, test_string_pool_passed, test_string_pool_failed},
	{run_test_ro_string_db, test_ro_string_db_passed, test_ro_string_db_failed},
	{run_test_input, test_input_passed, test_input_failed},
	{run_test_sort_vector, test_sort_vector_passed, test_sort_vector_failed},
};

int main()
{
	int passed = 0, failed = 0, end = sizeof(all_tests)/sizeof(*all_tests);

    for (int i = 0; i < end; ++i)
    {
        all_tests[i].run_tests();
        passed += all_tests[i].get_passed();
        failed += all_tests[i].get_failed();
    }

    putchar('\n');
    printf("Files tested: %d\n", end);
    printf("All passed  : %d\n", passed);
    printf("All failed  : %d\n", failed);
    return failed;
}
