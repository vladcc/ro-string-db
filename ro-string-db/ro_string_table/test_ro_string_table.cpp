#include "../test/test.h"
#include "ro_string_table.hpp"

#include <vector>
#include <string>
#include <iostream>

static bool test_ro_string_table(void);

static ftest tests[] = {
	test_ro_string_table,
};

static bool didnt_throw = false;

#define print(str) std::cout << (str) << std::endl;

static bool test_ro_string_table(void)
{
	int all_lines = 6;
	int data_lines = all_lines-1; // first line comes from field_info
	
	bool is_unique = true;
	ro_string_table::field_info ufld_id("id", is_unique);
	ro_string_table::field_info ufld_fruit("fruit", is_unique);
	ro_string_table::field_info fld_type("type");
	ro_string_table::field_info fld_price("price");
	
	std::vector<ro_string_table::field_info>
		fields{ufld_id, ufld_fruit, fld_type, fld_price};
	
	std::vector<const char *> ids{"1", "2", "3", "4", "5"};
	std::vector<const char *>
		fruits{"pineapple", "apple", "peach", "mango", "pear"};
	std::vector<const char *>
		type{"fancy", "normal", "normal", "fancy", "normal"};
	std::vector<const char *> price{"12.25", "5.32", "4.22", "10.50", "6.00"};
	
	{ // test iteration
		ro_string_table str_tbl(all_lines, fields);
		for (int i = 0; i < data_lines; ++i)
		{
			str_tbl.append(ids[i]);
			str_tbl.append(fruits[i]);
			str_tbl.append(type[i]);
			str_tbl.append(price[i]);
		}
		str_tbl.seal();
		
		std::vector<const char *> l0 = {"id", "fruit", "type", "price"};
		std::vector<const char *> l1 = {"1", "pineapple", "fancy", "12.25"};
		std::vector<const char *> l2 = {"2", "apple", "normal", "5.32"};
		std::vector<const char *> l3 = {"3", "peach", "normal", "4.22"};
		std::vector<const char *> l4 = {"4", "mango", "fancy", "10.50"};
		std::vector<const char *> l5 = {"5", "pear", "normal", "6.00"};
		std::vector<std::vector<const char *>> tbl = {l0, l1, l2, l3, l4, l5};
		
		uint rows = str_tbl.get_num_rows();
		uint cols = str_tbl.get_num_cols();
		
		check(rows == 6);
		check(cols == 4);
		check(std::string(str_tbl.get_str_at(0, 0)) == std::string("id"));
		check(std::string(str_tbl.get_str_at(rows-1, cols-1))
			== std::string("6.00")
		);
		
		for (uint i = 0; i < rows; ++i)
		{
			for (uint j = 0; j < cols; ++j)
			{			
				check(std::string(str_tbl.get_str_at(i, j))
					== std::string(tbl[i][j])
				);
			}
		}
	}
	
	{ // throw on bad unique
		{
			ro_string_table::field_info fld_type("type", is_unique);
			std::vector<ro_string_table::field_info>
				fields{ufld_id, ufld_fruit, fld_type, fld_price};
			
			ro_string_table str_tbl(all_lines, fields);
			for (int i = 0; i < data_lines; ++i)
			{
				str_tbl.append(ids[i]);
				str_tbl.append(fruits[i]);
				str_tbl.append(type[i]);
				str_tbl.append(price[i]);
			}
			
			try {str_tbl.seal(); check(didnt_throw);}
			catch(std::runtime_error& e)
			{
				std::string expected("single_field_data::check_unique(): string 'fancy' appears more than once in field 'type' marked as unique");
				check(expected == e.what());
			}
		}
		
		{
			std::vector<const char *> ids{"1", "", "3", "", "5"};
			std::vector<ro_string_table::field_info>
			fields{ufld_id, ufld_fruit, fld_type, fld_price};
			
			ro_string_table str_tbl(all_lines, fields);
			for (int i = 0; i < data_lines; ++i)
			{
				str_tbl.append(ids[i]);
				str_tbl.append(fruits[i]);
				str_tbl.append(type[i]);
				str_tbl.append(price[i]);
			}
			
			try {str_tbl.seal(); check(didnt_throw);}
			catch(std::runtime_error& e)
			{
				std::string expected("single_field_data::check_unique(): string '' appears more than once in field 'id' marked as unique");
				check(expected == e.what());
			}
		}
	}
	
	{ // throw more lines than specified
		ro_string_table str_tbl(all_lines, fields);
		for (int i = 0; i < data_lines; ++i)
		{
			str_tbl.append(ids[i]);
			str_tbl.append(fruits[i]);
			str_tbl.append(type[i]);
			str_tbl.append(price[i]);
		}
		
		try {str_tbl.append(ids[0]); check(didnt_throw);}
		catch(std::runtime_error& e)
		{
			std::string expected("ro_string_table: more lines added than the specified 6; the names of the requested fields mismatch the names of the fields in the file?");
			check(expected == e.what());
		}
	}
	
	{ // throw on append after seal
		ro_string_table str_tbl(all_lines, fields);
		for (int i = 0; i < data_lines; ++i)
		{
			str_tbl.append(ids[i]);
			str_tbl.append(fruits[i]);
			str_tbl.append(type[i]);
			str_tbl.append(price[i]);
		}
		str_tbl.seal();
		
		try {str_tbl.append(ids[0]); check(didnt_throw);}
		catch(std::runtime_error& e)
		{
			std::string expected("ro_string_table: append() called after seal()");
			check(expected == e.what());
		}
	}
	
	{ // throw lookup before seal
		ro_string_table str_tbl(all_lines, fields);
		for (int i = 0; i < data_lines; ++i)
		{
			str_tbl.append(ids[i]);
			str_tbl.append(fruits[i]);
			str_tbl.append(type[i]);
			str_tbl.append(price[i]);
		}
		
		ro_string_table::field_pair src("id", "3");
		std::vector<ro_string_table::field_pair> dest{
			ro_string_table::field_pair("fruit")
		};
		
		try {str_tbl.lookup_unique(src, dest); check(didnt_throw);}
		catch(std::runtime_error& e)
		{
			std::string expected("ro_string_table: lookup before seal()");
			check(expected == e.what());
		}
		
		std::vector<ro_string_table::eq_range_result> eqr{
			ro_string_table::eq_range_result("type")
		};
		
		try {str_tbl.lookup_equal_range(src, eqr); check(didnt_throw);}
		catch(std::runtime_error& e)
		{
			std::string expected("ro_string_table: lookup before seal()");
			check(expected == e.what());
		}
	}
	
	{ // throw field not unique
		ro_string_table str_tbl(all_lines, fields);
		for (int i = 0; i < data_lines; ++i)
		{
			str_tbl.append(ids[i]);
			str_tbl.append(fruits[i]);
			str_tbl.append(type[i]);
			str_tbl.append(price[i]);
		}
		str_tbl.seal();
		
		ro_string_table::field_pair src("type", "normal");
		std::vector<ro_string_table::field_pair> dest{
			ro_string_table::field_pair("fruit")
		};
		
		try {str_tbl.lookup_unique(src, dest); check(didnt_throw);}
		catch(std::runtime_error& e)
		{
			std::string expected("ro_string_table: unique lookup of non-unique field 'type'");
			check(expected == e.what());
		}
	}
	
	{ // throw no such field
		ro_string_table str_tbl(all_lines, fields);
		for (int i = 0; i < data_lines; ++i)
		{
			str_tbl.append(ids[i]);
			str_tbl.append(fruits[i]);
			str_tbl.append(type[i]);
			str_tbl.append(price[i]);
		}
		str_tbl.seal();
		
		{
			ro_string_table::field_pair src("i-do-not-exist", "normal");
			std::vector<ro_string_table::field_pair> dest{
				ro_string_table::field_pair("fruit")
			};
			
			try {str_tbl.lookup_unique(src, dest); check(didnt_throw);}
			catch(std::runtime_error& e)
			{
				std::string expected("ro_string_table: lookup fail: no such field 'i-do-not-exist'");
				check(expected == e.what());
			}
		}
		
		{
			ro_string_table::field_pair src("id", "3");
			std::vector<ro_string_table::field_pair> dest{
				ro_string_table::field_pair("banana")
			};
			try {str_tbl.lookup_unique(src, dest); check(didnt_throw);}
			catch(std::runtime_error& e)
			{
				std::string expected("ro_string_table: lookup fail: no such field 'banana'");
				check(expected == e.what());
			}
		}
		
		{
			ro_string_table::field_pair src("i-do-not-exist", "normal");
			std::vector<ro_string_table::eq_range_result> eqr{
				ro_string_table::eq_range_result("type")
			};
			
			try {str_tbl.lookup_equal_range(src, eqr); check(didnt_throw);}
			catch(std::runtime_error& e)
			{
				std::string expected("ro_string_table: lookup fail: no such field 'i-do-not-exist'");
				check(expected == e.what());
			}
		}
		
		{
			ro_string_table::field_pair src("type", "normal");
			std::vector<ro_string_table::eq_range_result> eqr{
				ro_string_table::eq_range_result("banana")
			};
			
			try {str_tbl.lookup_equal_range(src, eqr); check(didnt_throw);}
			catch(std::runtime_error& e)
			{
				std::string expected("ro_string_table: lookup fail: no such field 'banana'");
				check(expected == e.what());
			}
		}
	}
	
	{ // lookup_unique
		ro_string_table str_tbl(all_lines, fields);
		for (int i = 0; i < data_lines; ++i)
		{
			str_tbl.append(ids[i]);
			str_tbl.append(fruits[i]);
			str_tbl.append(type[i]);
			str_tbl.append(price[i]);
		}
		str_tbl.seal();
		
		ro_string_table::field_pair src("id", "10");
		std::vector<ro_string_table::field_pair> dest{
			ro_string_table::field_pair("fruit")
		};
		
		check(!str_tbl.lookup_unique(src, dest));
		check(!dest[0].field_value);
		
		src.field_value = "3";
		check(str_tbl.lookup_unique(src, dest));
		check(dest.size() == 1);
		check(std::string(dest[0].field_value) == std::string("peach"));
		
		dest.push_back(ro_string_table::field_pair("price"));
		src.field_value = "1";
		check(str_tbl.lookup_unique(src, dest));
		check(dest.size() == 2);
		check(std::string(dest[0].field_value) == std::string("pineapple"));
		check(std::string(dest[1].field_value) == std::string("12.25"));
		
		src.field_value = "5";
		check(str_tbl.lookup_unique(src, dest));
		check(dest.size() == 2);
		check(std::string(dest[0].field_value) == std::string("pear"));
		check(std::string(dest[1].field_value) == std::string("6.00"));
	}
	
	{ // lookup_equal_range
		ro_string_table str_tbl(all_lines, fields);
		for (int i = 0; i < data_lines; ++i)
		{
			str_tbl.append(ids[i]);
			str_tbl.append(fruits[i]);
			str_tbl.append(type[i]);
			str_tbl.append(price[i]);
		}
		str_tbl.seal();
				
		ro_string_table::field_pair src("type", "!normal");
		std::vector<ro_string_table::eq_range_result> eqr{
			ro_string_table::eq_range_result("fruit")
		};
		
		check(!str_tbl.lookup_equal_range(src, eqr));
		check(eqr[0].values.size() == 0);
		
		src.field_value = "normal";
		check(str_tbl.lookup_equal_range(src, eqr));
		check(eqr.size() == 1);
		check(eqr[0].values.size() == 3);
		check(std::string(eqr[0].values[0]) == std::string("apple"));
		check(std::string(eqr[0].values[1]) == std::string("peach"));
		check(std::string(eqr[0].values[2]) == std::string("pear"));
		
		eqr.push_back(ro_string_table::eq_range_result("id"));
		check(str_tbl.lookup_equal_range(src, eqr));
		check(eqr.size() == 2);
		check(eqr[0].values.size() == 3);
		check(eqr[1].values.size() == 3);
		check(std::string(eqr[0].values[0]) == std::string("apple"));
		check(std::string(eqr[0].values[1]) == std::string("peach"));
		check(std::string(eqr[0].values[2]) == std::string("pear"));
		check(std::string(eqr[1].values[0]) == std::string("2"));
		check(std::string(eqr[1].values[1]) == std::string("3"));
		check(std::string(eqr[1].values[2]) == std::string("5"));
		
		src.field_name = "id";
		src.field_value = "3";
		check(str_tbl.lookup_equal_range(src, eqr));
		check(eqr.size() == 2);
		check(eqr[0].values.size() == 1);
		check(eqr[1].values.size() == 1);
		check(std::string(eqr[0].values[0]) == std::string("peach"));
		check(std::string(eqr[1].values[0]) == std::string("3"));	
	}
	
	return true;
}

static int passed, failed;
void run_test_ro_string_table(void)
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

int test_ro_string_table_passed(void)
{return passed;}

int test_ro_string_table_failed(void)
{return failed;}
