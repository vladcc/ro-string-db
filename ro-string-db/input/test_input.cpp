#include "../test/test.h"
#include "input.hpp"

#include <set>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

#define print(str) std::cout << (str) << std::endl

#define DIR "../test_data"
#define SYMMETRIC_CSV DIR "/data_symmetric.csv"
#define MISSING_FIELDS_CSV DIR "/data_jagged.csv"
#define BAD_FILE_NAME DIR "/i-dont-exist.txt"
#define EMPTY_FILE DIR "/empty_file.txt"

static bool test_open_or_throw();
static bool test_count_lines_check_fld_number();
static bool test_filter_fields_keep();
static bool test_filter_fields_remove();
static bool test_split_string();

static ftest tests[] = {
	test_open_or_throw,
	test_count_lines_check_fld_number,
	test_filter_fields_keep,
	test_filter_fields_remove,
	test_split_string,
};

static bool didnt_throw = false; // for a readable fail message

static bool test_filter_fields_remove()
{
	std::vector<const char *> vect{"zero", "one", "two", "three", "four"};
	std::vector<const char *> out;
	check(out.size() == 0);
	
	{
		std::set<uint> set{1, 3};
		input::filter_fields_remove(vect, set, out);
		check(out.size() == 3);
		check(std::string(out[0]) == std::string("zero"));
		check(std::string(out[1]) == std::string("two"));
		check(std::string(out[2]) == std::string("four"));
	}
	
	{
		std::set<uint> set{0, 4};
		input::filter_fields_remove(vect, set, out);
		check(out.size() == 3);
		check(std::string(out[0]) == std::string("one"));
		check(std::string(out[1]) == std::string("two"));
		check(std::string(out[2]) == std::string("three"));
	}
	
	{
		std::set<uint> set{1, 2, 3};
		input::filter_fields_remove(vect, set, out);
		check(out.size() == 2);
		check(std::string(out[0]) == std::string("zero"));
		check(std::string(out[1]) == std::string("four"));
	}
	
	{
		std::set<uint> set{0, 100};
		try
		{input::filter_fields_remove(vect, set, out); check(didnt_throw);}
		catch (std::runtime_error& e)
		{check(e.what() == std::string("input::filter_fields_remove(): field number 100 out of bounds"));}
	}
		
	return true;
}

static bool test_filter_fields_keep()
{
	std::vector<const char *> vect{"zero", "one", "two", "three", "four"};
	std::vector<const char *> out;
	check(out.size() == 0);
	
	{
		std::set<uint> set{2, 1, 3, 0, 4};
		input::filter_fields_keep(vect, set, out);
		check(out.size() == 5);
		check(std::string(out[0]) == std::string("zero"));
		check(std::string(out[1]) == std::string("one"));
		check(std::string(out[2]) == std::string("two"));
		check(std::string(out[3]) == std::string("three"));
		check(std::string(out[4]) == std::string("four"));
	}
	
	{
		std::set<uint> set{1, 3};
		input::filter_fields_keep(vect, set, out);
		check(out.size() == 2);
		check(std::string(out[0]) == std::string("one"));
		check(std::string(out[1]) == std::string("three"));
	}
	
	{
		std::set<uint> set{0, 4};
		input::filter_fields_keep(vect, set, out);
		check(out.size() == 2);
		check(std::string(out[0]) == std::string("zero"));
		check(std::string(out[1]) == std::string("four"));
	}
	
	{
		std::set<uint> set{1, 2, 3};
		input::filter_fields_keep(vect, set, out);
		check(out.size() == 3);
		check(std::string(out[0]) == std::string("one"));
		check(std::string(out[1]) == std::string("two"));
		check(std::string(out[2]) == std::string("three"));
	}
	
	{
		std::set<uint> set{0, 100};
		try
		{input::filter_fields_keep(vect, set, out); check(didnt_throw);}
		catch (std::runtime_error& e)
		{check(e.what() == std::string("input::filter_fields_keep(): field number 100 out of bounds"));}
	}
	
	return true;
}

static bool test_open_or_throw()
{
	std::ifstream in;
	
	try
	{input::open_or_throw(BAD_FILE_NAME, in); check(didnt_throw);}
	catch (std::runtime_error& e)
	{check(e.what() == std::string("input::read_lines(): couldn't open file '" BAD_FILE_NAME "'"));}
	
	input::open_or_throw(SYMMETRIC_CSV, in);
	check(in.is_open());
	in.close();
	
	return true;
}

static bool test_count_lines_check_fld_number()
{
	check(input::count_lines_check_fld_number(EMPTY_FILE, ';', 4) == 0);
	
	try
	{
		input::count_lines_check_fld_number(SYMMETRIC_CSV, ';', 3);
		check(didnt_throw);
	}
	catch (std::runtime_error& e)
	{
		check(e.what() == std::string("input::count_lines_check_fld_number(): number of fields 4 on line 1 in file '" DIR "/data_symmetric.csv' different than the specified 3; line: 'id;fruit;available;price'; delimiter given: ';'"));
	}
	
	try
	{
		input::count_lines_check_fld_number(SYMMETRIC_CSV, ';', 100);
		check(didnt_throw);
	}
	catch (std::runtime_error& e)
	{
		check(e.what() == std::string("input::count_lines_check_fld_number(): number of fields 4 on line 1 in file '" DIR "/data_symmetric.csv' different than the specified 100; line: 'id;fruit;available;price'; delimiter given: ';'"));
	}
	
	try
	{
		input::count_lines_check_fld_number(SYMMETRIC_CSV, ';', 0);
		check(didnt_throw);
	}
	catch (std::runtime_error& e)
	{
		check(e.what() == std::string("input::count_lines_check_fld_number(): number of fields 4 on line 1 in file '" DIR "/data_symmetric.csv' different than the specified 0; line: 'id;fruit;available;price'; delimiter given: ';'"));
	}
	
	check(input::count_lines_check_fld_number(SYMMETRIC_CSV, ';', 4) == 12);
	
	try
	{
		input::count_lines_check_fld_number(MISSING_FIELDS_CSV, ';', 4);
		check(didnt_throw);
	}
	catch (std::runtime_error& e)
	{
		check(e.what() == std::string("input::count_lines_check_fld_number(): number of fields 2 on line 3 in file '" DIR "/data_jagged.csv' different than the specified 4; line: 'yes;3.50'; delimiter given: ';'"));
	}
	
	return true;
}

static bool test_split_string()
{
	std::vector<const char *> split;
	std::string str("field1;field2;field3");
	
	check(input::split_string(str, '|', split) == 1);
	check(split.size() == 1);
	check(std::string(split[0]) == str);
	
	std::string cpy(str);
	check(input::split_string(cpy, ';', split) == 3);
	check(split.size() == 3);
	check(std::string(split[0]) == "field1");
	check(std::string(split[1]) == "field2");
	check(std::string(split[2]) == "field3");
	
	cpy = str;
	cpy += ";";
	check(input::split_string(cpy, ';', split) == 4);
	check(split.size() == 4);
	check(std::string(split[0]) == "field1");
	check(std::string(split[1]) == "field2");
	check(std::string(split[2]) == "field3");
	check(std::string(split[3]) == "");
	
	cpy = ";";
	cpy += str;
	check(input::split_string(cpy, ';', split) == 4);
	check(split.size() == 4);
	check(std::string(split[0]) == "");
	check(std::string(split[1]) == "field1");
	check(std::string(split[2]) == "field2");
	check(std::string(split[3]) == "field3");
	
	cpy = ";;;;";
	check(input::split_string(cpy, ';', split) == 5);
	check(split.size() == 5);
	check(std::string(split[0]) == "");
	check(std::string(split[1]) == "");
	check(std::string(split[2]) == "");
	check(std::string(split[3]) == "");
	check(std::string(split[4]) == "");

	return true;
}

static int passed, failed;
void run_test_input(void)
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

int test_input_passed(void)
{return passed;}

int test_input_failed(void)
{return failed;}
