#include "../test/test.h"
#include "sort_vector.ipp"

static bool test_sort_vector_lookup(void);
static bool test_sort_vector_equal_range(void);

static ftest tests[] = {
	test_sort_vector_lookup,
	test_sort_vector_equal_range,
};

static bool didnt_throw = false;

struct int_in_a_struct
{
	int_in_a_struct(int n) : i(n) {}
	int i;
};

static bool test_sort_vector_equal_range(void)
{	
	gen_comp_less<int_in_a_struct, int> normal_less(
		[](const int_in_a_struct& lhs,
			const int_in_a_struct& rhs,
			int context
		)
		{
			int a = lhs.i;
			int b = rhs.i;
			return ((a > b) - (a < b));
		}
	);
	
	auto generic_ctx_3way_cmp = [](const int_in_a_struct& lhs,
		const int_in_a_struct& rhs,
		int context
	)
	{
		int a = lhs.i;
		int b = context;
		return ((a > b) - (a < b));
	};
	
	int my_int = 6;
	gen_comp_less_ctx_lower_bound<int_in_a_struct, int> ctx_lookup(
		generic_ctx_3way_cmp,
		my_int
	);
	
	gen_comp_less_ctx_lower_bound<int_in_a_struct, int> ctx_lower(
		generic_ctx_3way_cmp
	);
	
	gen_comp_less_ctx_upper_bound<int_in_a_struct, int> ctx_upper(
		generic_ctx_3way_cmp
	);
	
	my_int = 5;
	sort_vector<int_in_a_struct, int>::equal_range_ctx_compars eq_range(
		ctx_lower,
		ctx_upper,
		my_int
	);
	
	sort_vector<int_in_a_struct, int> sort_vect(normal_less);
	
	int place_inside[] = {5, 5, 5, 1, 6, 9, 9};
	for (size_t i = 0; i < sizeof(place_inside)/sizeof(*place_inside); ++i)
		sort_vect.append(int_in_a_struct(place_inside[i]));
	
	int_in_a_struct my_int_in_a_struct(7);
	const int_in_a_struct * result_ = nullptr;
	const int_in_a_struct ** result = &result_;
	
	std::pair<size_t, size_t> pres;
	
	try {sort_vect.lookup(my_int_in_a_struct, result); check(didnt_throw);}
	catch(std::runtime_error& e)
	{
		std::string expected("sort_vector: lookup on unsorted data");
		check(expected == e.what());
	}
	
	try
	{
		sort_vect.equal_range(my_int_in_a_struct, pres, eq_range);
		check(didnt_throw);
	}
	catch(std::runtime_error& e)
	{
		std::string expected("sort_vector: equal_range on unsorted data");
		check(expected == e.what());
	}
	
	sort_vect.seal();
	
	int sorted[] = {1, 5, 5, 5, 6, 9, 9};
	for (size_t i = 0; i < sizeof(sorted)/sizeof(*sorted); ++i)
		check(sort_vect.get(i).i == sorted[i]);
	
	check(!sort_vect.lookup(my_int_in_a_struct, result));
	
	my_int_in_a_struct.i = 9;
	check(sort_vect.lookup(my_int_in_a_struct, result));
	check((*result)->i == 9);
	
	my_int_in_a_struct.i = 5;
	check(sort_vect.lookup(my_int_in_a_struct, result, ctx_lookup));
	check((*result)->i == 6);
	
	
	my_int_in_a_struct.i = 6; // <-- ignored, because context is 5
	check(sort_vect.equal_range(my_int_in_a_struct, pres, eq_range));
	check(pres.first == 1);
	check(pres.second == 4);
	for (size_t i = pres.first; i < pres.second; ++i)
		check(sort_vect.get(i).i == 5);
	
	check(sort_vect.equal_range(my_int_in_a_struct, pres, eq_range));
	check(pres.first == 1);
	check(pres.second == 4);
	for (size_t i = pres.first; i < pres.second; ++i)
		check(sort_vect.get(i).i == 5);
		
	eq_range.change_context(1);
	check(sort_vect.equal_range(my_int_in_a_struct, pres, eq_range));
	check(pres.first == 0);
	check(pres.second == 1);
	for (size_t i = pres.first; i < pres.second; ++i)
		check(sort_vect.get(i).i == 1);
		
	eq_range.change_context(6);
	check(sort_vect.equal_range(my_int_in_a_struct, pres, eq_range));
	check(pres.first == 4);
	check(pres.second == 5);
	for (size_t i = pres.first; i < pres.second; ++i)
		check(sort_vect.get(i).i == 6);
	
	eq_range.change_context(-1000);
	check(!sort_vect.equal_range(my_int_in_a_struct, pres, eq_range));
	check(pres.first == 0);
	check(pres.second == 0);
	
	eq_range.change_context(9);
	check(sort_vect.equal_range(my_int_in_a_struct, pres, eq_range));
	check(pres.first == 5);
	check(pres.second == 7);
	for (size_t i = pres.first; i < pres.second; ++i)
		check(sort_vect.get(i).i == 9);
	
	eq_range.change_context(1000);
	check(!sort_vect.equal_range(my_int_in_a_struct, pres, eq_range));
	check(pres.first == 7);
	check(pres.second == 7);
	return true;
}

static bool add_10(sort_vector<int_in_a_struct, int>& vect)
{
	for (int i = 10; i > 0; --i)
		vect.append(int_in_a_struct(i));
	
	int n = 10;
	for (size_t i = 0; i < vect.size(); ++i)
		check(vect.get(i).i == (n--));
		
	return true;
}

static bool test_sort_vector_lookup(void)
{
	gen_comp_less<int_in_a_struct, int> normal_less(
		[](const int_in_a_struct& lhs,
			const int_in_a_struct& rhs,
			int context
		)
		{
			int a = lhs.i;
			int b = rhs.i;
			return ((a > b) - (a < b));
		}
	);
	
	auto generic_ctx_3way_cmp = [](const int_in_a_struct& lhs,
		const int_in_a_struct& rhs,
		int context
	)
	{
		int a = lhs.i;
		int b = context;
		return ((a > b) - (a < b));
	};
	
	int my_int = 6;
	gen_comp_less_ctx_lower_bound<int_in_a_struct, int> ctx_lookup(
		generic_ctx_3way_cmp,
		my_int
	);
	
	sort_vector<int_in_a_struct, int> sort_vect(normal_less);
	add_10(sort_vect);
	
	int_in_a_struct my_int_in_a_struct(7);
	const int_in_a_struct * result_ = nullptr;
	const int_in_a_struct ** result = &result_;
	
	try {sort_vect.lookup(my_int_in_a_struct, result); check(didnt_throw);}
	catch(std::runtime_error& e) {}
	
	try
	{
		sort_vect.lookup(my_int_in_a_struct, result, ctx_lookup);
		check(didnt_throw);
	}
	catch(std::runtime_error& e) {}
	
	sort_vect.seal();
	for (size_t i = 0; i < sort_vect.size(); ++i)
		check(sort_vect.get(i).i == static_cast<int>((i+1)));
	
	check(sort_vect.lookup(my_int_in_a_struct, result));
	check((*result)->i == 7);
	
	check(sort_vect.lookup(my_int_in_a_struct, result, ctx_lookup));
	check((*result)->i == 6);
	
	my_int_in_a_struct.i = 10000;
	check(!sort_vect.lookup(my_int_in_a_struct, result));
	
	ctx_lookup.set_context(3);
	check(sort_vect.lookup(my_int_in_a_struct, result, ctx_lookup));
	check((*result)->i == 3);

	ctx_lookup.set_context(-9000);
	check(!sort_vect.lookup(my_int_in_a_struct, result, ctx_lookup));
	
	my_int_in_a_struct.i = -5;
	sort_vect.append(my_int_in_a_struct);
	for (size_t i = 0; i < sort_vect.size()-1; ++i)
		check(sort_vect.get(i).i == static_cast<int>((i+1)));
	check(sort_vect.get(sort_vect.size()-1).i == -5);
	
	try {sort_vect.lookup(my_int_in_a_struct, result); check(didnt_throw);}
	catch(std::runtime_error& e) {}
	
	sort_vect.seal();
	check(sort_vect.get(0).i == -5);
	check(sort_vect.lookup(my_int_in_a_struct, result));
	check((*result)->i == -5);
	
	return true;
}

static int passed, failed;
void run_test_sort_vector(void)
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

int test_sort_vector_passed(void)
{return passed;}

int test_sort_vector_failed(void)
{return failed;}
