#include "../test/test.h"
#include "ro_string_db.hpp"

#include <set>
#include <string>
#include <vector>
#include <iostream>

static bool test_ro_string_db_statics(void);
static bool test_ro_string_db(void);

static ftest tests[] = {
	test_ro_string_db_statics,
	test_ro_string_db,
};

static bool didnt_throw = false;

#define print(str) std::cout << str << std::endl;

#define DIR "../test_data"
#define EMPTY_FILE DIR "/empty_file.txt"
#define FRUIT_FILE DIR "/fruit.csv"
#define FRUIT_FILE_QUOTES DIR "/fruit_quotes.csv"
#define FRUIT_FILE_BAD DIR "/fruit_bad.csv"

static bool test_ro_string_db_statics(void)
{
	class stat_test : public ro_string_db
	{
		public:
		static void _field_info_to_str_vect(std::vector<field_info>& fi,
			std::vector<std::string>& out
		) {return ro_string_db::_field_info_to_str_vect(fi, out);}
		
		static void _get_vect_as_cs_string(std::vector<std::string>& vec,
			std::string& out
		) {return ro_string_db::_get_vect_as_cs_string(vec, out);}
		
		static bool _is_vect_a_set(std::vector<std::string>& vect,
			std::string& out_offender
		) {return ro_string_db::_is_vect_a_set(vect, out_offender);}
		
		static bool _is_va_subset_of_vb(std::vector<std::string>& va,
			std::vector<std::string>& vb,
			std::string& out_offender
		) {return ro_string_db::_is_va_subset_of_vb(va, vb, out_offender);}
		
		static bool _is_va_set_ordered_in_vb_set(
			std::vector<std::string>& va,
			std::vector<std::string>& vb,
			std::string& out_offender
		)
		{
			return ro_string_db::
				_is_va_set_ordered_in_vb_set(va, vb, out_offender);
		}
	};
	
	std::vector<std::string> l0 = {"id", "fruit", "type", "price"};
	std::string single_line, str_l0("id,fruit,type,price");
	
	{ // first_line_to_field_names()
		std::vector<std::string> from_file;
		ro_string_db::first_line_to_field_names(FRUIT_FILE,
			';',
			nullptr,
			from_file
		);
		
		check(from_file == l0);
		
		ro_string_db::first_line_to_field_names(FRUIT_FILE_QUOTES,
			';',
			nullptr,
			from_file
		);
		
		check(from_file != l0);
		
		ro_string_db::first_line_to_field_names(FRUIT_FILE_QUOTES,
			';',
			[](std::string& field)
			{
				field.erase(std::remove(field.begin(), field.end(), '\''),
					field.end()
				);
			},
			from_file
		);
		
		check(from_file == l0);
	}
	
	{ // _get_vect_as_cs_string()
		stat_test::_get_vect_as_cs_string(l0, single_line);
		check(single_line == str_l0);
	}
	
	{ // _field_info_to_str_vect()
		bool is_unique = true;
		ro_string_db::field_info ufld_id("id", is_unique);
		ro_string_db::field_info ufld_fruit("fruit", is_unique);
		ro_string_db::field_info fld_type("type");
		ro_string_db::field_info fld_price("price");
		
		std::vector<ro_string_db::field_info>
			fields{ufld_id, ufld_fruit, fld_type, fld_price};
		std::vector<std::string> vstr_out{"mark"};
		
		stat_test::_field_info_to_str_vect(fields, vstr_out);
		check(vstr_out.size() == fields.size());
		check(vstr_out[0] == fields[0].name);
		check(vstr_out[1] == fields[1].name);
		check(vstr_out[2] == fields[2].name);
		check(vstr_out[3] == fields[3].name);
		
		single_line = "not correct";
		stat_test::_get_vect_as_cs_string(l0, single_line);
		check(single_line == str_l0);
	}
	
	{ // _is_vect_a_set()
		std::string offender;
		std::vector<std::string> vect_set{"a", "b", "c", "d"};
		
		check(stat_test::_is_vect_a_set(vect_set, offender));
		check(offender.length() == 0);
		
		std::vector<std::string> not_set{"a", "b", "a", "d"};
		
		check(!stat_test::_is_vect_a_set(not_set, offender));
		check(offender == not_set[0]);
	}
	
	{ // _is_va_subset_of_vb()
		std::string offender;
		std::vector<std::string> big_set{"a", "x", "b", "y", "c", "z", "d"};
		std::vector<std::string> small_set{"x", "y", "z"};
		
		check(stat_test::_is_va_subset_of_vb(small_set, big_set, offender));
		check(offender.length() == 0);
		
		check(!stat_test::_is_va_subset_of_vb(big_set, small_set, offender));
		check(offender == big_set[0]);
		
		std::vector<std::string> small_set2{"x", "1", "y", "z"};
		check(!stat_test::_is_va_subset_of_vb(small_set2, big_set, offender));
		check(offender == small_set2[1]);
		
		std::vector<std::string> small_set3{"x", "y", "z", "3"};
		check(!stat_test::_is_va_subset_of_vb(small_set3, big_set, offender));
		check(offender == small_set3[3]);
	}
	
	{ // _is_va_set_ordered_in_vb_set()
		std::string off;
		std::vector<std::string> big{"0", "1", "2", "3", "4"};
		std::vector<std::string> small{"0", "2", "3"};
		
		check(stat_test::_is_va_set_ordered_in_vb_set(small, big, off));
		check(off.length() == 0);
		
		std::vector<std::string> small2{"2", "3", "1", "4", "zazz"};
		check(!stat_test::_is_va_set_ordered_in_vb_set(small2, big, off));
		check(off == small2[2]);
		
		std::vector<std::string> small3{"1", "10"};
		check(!stat_test::_is_va_set_ordered_in_vb_set(small3, big, off));
		check(off == small3[1]);
		
		std::vector<std::string> small4{"10", "1"};
		check(!stat_test::_is_va_set_ordered_in_vb_set(small4, big, off));
		check(off == small4[0]);
	}
	
	return true;
}

static bool test_ro_string_db(void)
{
	bool is_unique = true;
	ro_string_db::field_info ufld_id("id", is_unique);
	ro_string_db::field_info ufld_fruit("fruit", is_unique);
	ro_string_db::field_info fld_type("type");
	ro_string_db::field_info fld_price("price");
	
	std::vector<const char *> l0 = {"id", "fruit", "type", "price"};
	std::vector<std::string> fld_names(l0.begin(), l0.end());
		
	std::vector<ro_string_db::byte> buff;

	{ // test empty file
		std::vector<ro_string_db::field_info>
			fields{ufld_id, ufld_fruit, fld_type, fld_price};
		
		ro_string_db::init_info init(EMPTY_FILE, ';', fld_names, fields);
		
		try {ro_string_db str_db(init); check(didnt_throw);}
		catch(std::runtime_error& e)
		{
			std::string expected("ro_string_db: file '" EMPTY_FILE "' is empty");
			check(expected == e.what());
		}
	}
	
	{ // test bad field number on a line
		std::vector<ro_string_db::field_info>
			fields{ufld_id, ufld_fruit, fld_type, fld_price};
		
		ro_string_db::init_info init(FRUIT_FILE_BAD, ';', fld_names, fields);
		
		try {ro_string_db str_db(init); check(didnt_throw);}
		catch(std::runtime_error& e)
		{
			std::string expected("input::count_lines_check_fld_number(): number of fields 3 on line 5 in file '" DIR "/fruit_bad.csv' different than the specified 4; line: '4;mango;10.50'; delimiter given: ';'");
			check(expected == e.what());
		}
	}

	std::vector<const char *> l1 = {"1", "pineapple", "fancy", "12.25"};
	std::vector<const char *> l2 = {"2", "apple", "normal", "5.32"};
	std::vector<const char *> l3 = {"3", "peach", "normal", "4.22"};
	std::vector<const char *> l4 = {"4", "mango", "fancy", "10.50"};
	std::vector<const char *> l5 = {"5", "pear", "normal", "6.00"};
	std::vector<std::vector<const char *>> tbl = {l0, l1, l2, l3, l4, l5};
	
	std::vector<const char *> l0k = {"fruit", "type"};
	std::vector<const char *> l1k = {"pineapple", "fancy"};
	std::vector<const char *> l2k = {"apple", "normal"};
	std::vector<const char *> l3k = {"peach", "normal"};
	std::vector<const char *> l4k = {"mango", "fancy"};
	std::vector<const char *> l5k = {"pear", "normal"};
	std::vector<std::vector<const char *>> kept = {
		l0k, l1k, l2k, l3k, l4k, l5k
	};
	
	std::vector<const char *> l0r = {"id", "price"};
	std::vector<const char *> l1r = {"1", "12.25"};
	std::vector<const char *> l2r = {"2", "5.32"};
	std::vector<const char *> l3r = {"3", "4.22"};
	std::vector<const char *> l4r = {"4", "10.50"};
	std::vector<const char *> l5r = {"5", "6.00"};
	std::vector<std::vector<const char *>> removed = {
		l0r, l1r, l2r, l3r, l4r, l5r
	};
	
	{ // trivial case
		std::vector<ro_string_db::field_info>
		fields{ufld_id, ufld_fruit, fld_type, fld_price};
		
		ro_string_db::init_info init(FRUIT_FILE, ';', fld_names, fields);
		
		ro_string_db str_db(init);
		
		uint rows = str_db.get_num_rows();
		uint cols = str_db.get_num_cols();
		
		check(rows == 6);
		check(cols == 4);
		check(std::string(str_db.get_str_at(0, 0)) == std::string("id"));
		check(std::string(str_db.get_str_at(rows-1, cols-1))
			== std::string("6.00")
		);
		
		for (uint i = 0; i < rows; ++i)
		{
			for (uint j = 0; j < cols; ++j)
			{			
				check(std::string(str_db.get_str_at(i, j))
					== std::string(tbl[i][j])
				);
			}
		}
	}
	
	{ // field list exceptions
		std::vector<ro_string_db::field_info>
				fields_ok{ufld_id, ufld_fruit, fld_type, fld_price};
		{
			std::vector<ro_string_db::field_info>
				fields{ufld_id, ufld_fruit, ufld_fruit, fld_price};
			
			ro_string_db::init_info init(FRUIT_FILE, ';', fld_names, fields);
			
			try {ro_string_db str_db(init); check(didnt_throw);}
			catch (std::runtime_error& e)
			{check(e.what() == std::string("ro_string_db: requested fields list 'id,fruit,fruit,price' is not a set; 'fruit' repeats"));}
		}

		{
			std::vector<std::string> l0_ = {"id", "type", "type", "price"};
			ro_string_db::init_info init(FRUIT_FILE, ';', l0_, fields_ok);
			
			try {ro_string_db str_db(init); check(didnt_throw);}
			catch (std::runtime_error& e)
			{check(e.what() == std::string("ro_string_db: field list 'id,type,type,price' for file '" DIR "/fruit.csv' is not a set; 'type' repeats"));}
		}
		
		{
			ro_string_db::field_info ufld_zazz("zazz", is_unique);
			std::vector<ro_string_db::field_info>
				fields{ufld_id, ufld_fruit, ufld_zazz, fld_price};
				
			ro_string_db::init_info init(FRUIT_FILE, ';', fld_names, fields);
			
			try {ro_string_db str_db(init); check(didnt_throw);}
			catch (std::runtime_error& e)
			{check(e.what() == std::string("ro_string_db: requested fields list 'id,fruit,zazz,price' not a subset of declared fields list 'id,fruit,type,price' for file '" DIR "/fruit.csv'; 'zazz' not found in the declared list"));}
		}
		
		{
			std::vector<ro_string_db::field_info>
				fields{ufld_id, fld_type, ufld_fruit, fld_price};
				
			ro_string_db::init_info init(FRUIT_FILE, ';', fld_names, fields);
			
			try {ro_string_db str_db(init); check(didnt_throw);}
			catch (std::runtime_error& e)
			{check(e.what() == std::string("ro_string_db: requested fields list 'id,type,fruit,price' order not relative to the order of the declared fields list 'id,fruit,type,price' for file '" DIR "/fruit.csv'; 'fruit' is misplaced"));}
		}
	}
	
	{ // process fields as they are read
		std::vector<const char *> l0_ = {
			"'id'", "'fruit'", "'type'", "'price'"
		};
		std::vector<std::string> fld_names(l0_.begin(), l0_.end());
		
		ro_string_db::field_info ufld_id("'id'", is_unique);
		ro_string_db::field_info ufld_fruit("'fruit'", is_unique);
		ro_string_db::field_info fld_type("'type'");
		ro_string_db::field_info fld_price("'price'");
		
		std::vector<ro_string_db::field_info>
			fields{ufld_id, ufld_fruit, fld_type, fld_price};
		
		ro_string_db::init_info init(FRUIT_FILE_QUOTES,
			';',
			fld_names,
			fields,
			[](std::string& field)
			{
				field.erase(std::remove(field.begin(), field.end(), '\''),
					field.end()
				);
			}
		);
		
		ro_string_db str_db(init);
		
		uint rows = str_db.get_num_rows();
		uint cols = str_db.get_num_cols();
		
		check(rows == 6);
		check(cols == 4);
			
		check(std::string(str_db.get_str_at(0, 0)) == std::string("id"));
		check(std::string(str_db.get_str_at(rows-1, cols-1))
			== std::string("6.00")
		);
		
		for (uint i = 0; i < rows; ++i)
		{
			for (uint j = 0; j < cols; ++j)
			{			
				check(std::string(str_db.get_str_at(i, j))
					== std::string(tbl[i][j])
				);
			}
		}
	}
	
	{ // process fields as they are read; no quotes in given field names
		std::vector<const char *> l0_ = {
			"id", "fruit", "type", "price"
		};
		std::vector<std::string> fld_names(l0_.begin(), l0_.end());
		
		ro_string_db::field_info ufld_id("id", is_unique);
		ro_string_db::field_info ufld_fruit("fruit", is_unique);
		ro_string_db::field_info fld_type("type");
		ro_string_db::field_info fld_price("price");
		
		std::vector<ro_string_db::field_info>
			fields{ufld_id, ufld_fruit, fld_type, fld_price};
		
		ro_string_db::init_info init(FRUIT_FILE_QUOTES,
			';',
			fld_names,
			fields,
			[](std::string& field)
			{
				field.erase(std::remove(field.begin(), field.end(), '\''),
					field.end()
				);
			}
		);
		
		ro_string_db str_db(init);
		
		uint rows = str_db.get_num_rows();
		uint cols = str_db.get_num_cols();
		
		check(rows == 6);
		check(cols == 4);
			
		check(std::string(str_db.get_str_at(0, 0)) == std::string("id"));
		check(std::string(str_db.get_str_at(rows-1, cols-1))
			== std::string("6.00")
		);
		
		for (uint i = 0; i < rows; ++i)
		{
			for (uint j = 0; j < cols; ++j)
			{			
				check(std::string(str_db.get_str_at(i, j))
					== std::string(tbl[i][j])
				);
			}
		}
	}
	
	{ // request only two fields
		
		std::vector<ro_string_db::field_info>
		fields{ufld_fruit, fld_price};
		
		ro_string_db::init_info init(FRUIT_FILE, ';', fld_names, fields);
		
		ro_string_db str_db(init);
		
		uint rows = str_db.get_num_rows();
		uint cols = str_db.get_num_cols();
		
		check(rows == 6);
		check(cols == 2);
		check(std::string(str_db.get_str_at(0, 0)) == std::string("fruit"));
		check(std::string(str_db.get_str_at(rows-1, cols-1))
			== std::string("6.00")
		);
		
		std::vector<std::vector<std::string>> tbl{
			{"fruit", "price"},
			{"pineapple", "12.25"},
			{"apple", "5.32"},
			{"peach", "4.22"},
			{"mango", "10.50"},
			{"pear","6.00"},
		};
		
		for (uint i = 0; i < rows; ++i)
		{
			for (uint j = 0; j < cols; ++j)
			{			
				check(std::string(str_db.get_str_at(i, j))
					== std::string(tbl[i][j])
				);
			}
		}
	}
	
	{ // request only two fields, process fields
		
		std::vector<ro_string_db::field_info>
		fields{ufld_fruit, fld_price};
		
		ro_string_db::init_info init(FRUIT_FILE_QUOTES,
			';',
			fld_names,
			fields,
			[](std::string& field)
			{
				field.erase(std::remove(field.begin(), field.end(), '\''),
					field.end()
				);
			}
		);
		
		ro_string_db str_db(init);
		
		uint rows = str_db.get_num_rows();
		uint cols = str_db.get_num_cols();
		
		check(rows == 6);
		check(cols == 2);
		check(std::string(str_db.get_str_at(0, 0)) == std::string("fruit"));
		check(std::string(str_db.get_str_at(rows-1, cols-1))
			== std::string("6.00")
		);
		
		std::vector<std::vector<std::string>> tbl{
			{"fruit", "price"},
			{"pineapple", "12.25"},
			{"apple", "5.32"},
			{"peach", "4.22"},
			{"mango", "10.50"},
			{"pear","6.00"},
		};
		
		for (uint i = 0; i < rows; ++i)
		{
			for (uint j = 0; j < cols; ++j)
			{			
				check(std::string(str_db.get_str_at(i, j))
					== std::string(tbl[i][j])
				);
			}
		}
	}
	
	{ // more lines than expected because the names of the fields are wrong
		std::vector<const char *> l0_ = {
			"id", "_fruit", "type", "price"
		};
		std::vector<std::string> fld_names(l0_.begin(), l0_.end());
		
		ro_string_db::field_info ufld_id("id", is_unique);
		ro_string_db::field_info ufld_fruit("_fruit", is_unique);
		ro_string_db::field_info fld_type("type");
		ro_string_db::field_info fld_price("price");
		
		std::vector<ro_string_db::field_info>
		fields{ufld_id, ufld_fruit, fld_type, fld_price};
		
		ro_string_db::init_info init(FRUIT_FILE, ';', fld_names, fields);
		
		try {ro_string_db str_db(init); check(didnt_throw);}
		catch (std::runtime_error& e)
		{check(e.what() == std::string("ro_string_table: more lines added than the specified 6; the names of the requested fields mismatch the names of the fields in the file?"));}
	}
	
	{ // same but with less requested
		std::vector<const char *> l0_ = {
			"id", "_fruit", "type", "price"
		};
		std::vector<std::string> fld_names(l0_.begin(), l0_.end());
		
		ro_string_db::field_info ufld_id("id", is_unique);
		ro_string_db::field_info ufld_fruit("_fruit", is_unique);
		
		std::vector<ro_string_db::field_info>
		fields{ufld_id, ufld_fruit, fld_type, fld_price};
		
		ro_string_db::init_info init(FRUIT_FILE, ';', fld_names, fields);
		
		try {ro_string_db str_db(init); check(didnt_throw);}
		catch (std::runtime_error& e)
		{check(e.what() == std::string("ro_string_table: more lines added than the specified 6; the names of the requested fields mismatch the names of the fields in the file?"));}
	}
	
	return true;
}

static int passed, failed;
void run_test_ro_string_db(void)
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

int test_ro_string_db_passed(void)
{return passed;}

int test_ro_string_db_failed(void)
{return failed;}
