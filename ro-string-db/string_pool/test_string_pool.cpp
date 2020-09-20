#include "../test/test.h"
#include "string_pool.hpp"

#include <string>
#include <vector>

static bool test_string_pool(void);

static ftest tests[] = {
	test_string_pool,
};

static bool test_string_pool(void)
{
	string_pool spool;
	std::string foo("foo");
	std::string bar("barr");
	std::string baz("baz");
	
	check(spool.size() == 0);
	check(spool.append(foo) == 0);
	check(spool.size() == 4);
	check(*spool.get(0) == 'f');
	check(*spool.get(1) == 'o');
	check(*spool.get(2) == 'o');
	check(*spool.get(3) == '\0');
	
	check(spool.append(bar) == 4);
	check(*spool.get(4) == 'b');
	check(*spool.get(5) == 'a');
	check(*spool.get(6) == 'r');
	check(*spool.get(7) == 'r');
	check(*spool.get(8) == '\0');
	
	check(spool.append(baz) == 9);
	check(*spool.get(9) == 'b');
	check(*spool.get(10) == 'a');
	check(*spool.get(11) == 'z');
	check(*spool.get(12) == '\0');
	
	return true;
}

static int passed, failed;
void run_test_string_pool(void)
{
    int i, end = sizeof(tests)/sizeof(*tests);

    passed = 0;
    for (i = 0; i < end; ++i)
        if (tests[i]())
            ++passed;

    if (passed != end)
        putchar('\n');

    failed = end - passed;
    report(passed, failed);
    return;
}

int test_string_pool_passed(void)
{return passed;}

int test_string_pool_failed(void)
{return failed;}
