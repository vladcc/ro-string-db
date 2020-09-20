#include "../test/test.h"
#include "matrix.ipp"

#include <vector>

template <typename T>
class test_mx : public matrix<T>
{
	public:
		typedef unsigned int uint;
		inline test_mx(uint rows, uint cols) :
			 matrix<T>(rows, cols)
		{}
		
		inline std::vector<T>& expose_memory()
		{return matrix<T>::expose_memory();}
};

static bool test_matrix(void);

static ftest tests[] = {
	test_matrix,
};

static bool test_matrix(void)
{
	const int cRows = 2;
	const int cCols = 3;
	test_mx<int> mx(cRows, cCols);
	
	std::vector<int>& vect = mx.expose_memory();
	check(vect.size() == cRows*cCols);
	
	check(mx.get_rows() == cRows);
	check(mx.get_cols() == cCols);
	
	int num = 0;
	for (int i = 0; i < cRows; ++i)
	{
		for (int j = 0; j < cCols; ++j)
			mx.place(i, j, num++);
	}
	
	num = 0;
	for (int i = 0; i < cRows; ++i)
	{
		for (int j = 0; j < cCols; ++j)
			check(mx.get(i, j) == num++);
	}
	
	num = 0;
	check(vect[0] == num++);
	check(vect[1] == num++);
	check(vect[2] == num++);
	check(vect[3] == num++);
	check(vect[4] == num++);
	check(vect[5] == num++);
	
	return true;
}

static int passed, failed;
void run_test_matrix(void)
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

int test_matrix_passed(void)
{return passed;}

int test_matrix_failed(void)
{return failed;}
