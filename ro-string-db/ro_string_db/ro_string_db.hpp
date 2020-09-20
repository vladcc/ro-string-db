/*
   ro_string_db.hpp -- associative csv representation with minimal memory
   overhead and logarithmic lookup time.
   v1.0
   
   Author: Vladimir Dinev
   vld.dinev@gmail.com
   2020-09-20
*/

#ifndef RO_STRING_DB_HPP
#define RO_STRING_DB_HPP

#include "input.hpp"
#include "ro_string_table.hpp"

#include <set>
#include <vector>
#include <string>
#include <memory>

class ro_string_db
{
	public:
	typedef unsigned int uint;
	typedef ro_string_table::field_pair field_pair;
	typedef ro_string_table::eq_range_result eq_range_result;
	typedef ro_string_table::field_info field_info;
	typedef ro_string_table::byte byte;
	typedef void (*on_field_split)(std::string& field);
	
	struct init_info
	{
		init_info(const char * csv_file_name,
			char delim,
			std::vector<std::string>& all_csv_field_names,
			std::vector<field_info>& fields_to_keep,
			on_field_split on_field = nullptr
		) :
			all_csv_field_names(all_csv_field_names),
			fields_to_keep(fields_to_keep),
			csv_file_name(csv_file_name),
			on_field(on_field),
			delim(delim)
		{}
		
		std::vector<std::string>& all_csv_field_names;
		std::vector<field_info>& fields_to_keep;
		const char * csv_file_name;
		on_field_split on_field;
		char delim;
	};
	/*
	   init_info is everything the ro_string_db needs to create itself.
	   
	   all_csv_field_names is a vector with the names of the fields in
	   csv_file_name in the same order as in the file. This must exist and can
	   be obtained from the csv file by calling first_line_to_field_names(),
	   provided the first line of the csv file is the field name list.
	   
	   fields_to_keep is the list of fields which will be committed to memory.
	   If a field name is not in fields_to_keep, it's skipped.
		
	   on_field is a user function which is called, if given, for each string
	   read from the file before that string is placed in the string pool. This
	   allows, for example, to get rid of quotes, or do other kind of
	   processing. It is also called for each string in both all_csv_field_names
	   and fields_to_keep in order to normalize their representation.
	   
	   all_csv_field_names and fields_to_keep must not have repeating elements.
	   fields_to_keep must be a subset of all_csv_field_names, and must have
	   the same relative order. If any of these conditions are not met, an
	   exception is thrown.
	*/
	
	ro_string_db(init_info& init);
	
	inline bool lookup_unique(const field_pair& source,
		const char * target_name,
		field_pair ** out_value
	)
	{
		field_pair& unq = _single_unq[0];
		unq.field_name = target_name;
		unq.field_value = nullptr;
		lookup_unique(source, _single_unq);
		*out_value = &unq;
		return unq.field_value;
	}
	/* A convenience function for looking up a single target field. */
	
	inline bool lookup_unique(const field_pair& source,
		std::vector<field_pair>& in_out_targets
	)
	{return _str_tbl->lookup_unique(source, in_out_targets);}
	/* See lookup_unique() in ro_string_table. */
	
	inline bool lookup_equal_range(const field_pair& source,
		const char * target_name,
		eq_range_result ** out_values
	)
	{
		eq_range_result& eqr = _single_eqr[0];
		eqr.field_name = target_name;
		eqr.values.clear();
		lookup_equal_range(source, _single_eqr);
		*out_values = &eqr;
		return eqr.values.size();
	}
	/* Another convenience function for looking up a single target field. */
	
	inline bool lookup_equal_range(const field_pair& source,
		std::vector<eq_range_result>& in_out_targets
	) 
	{return _str_tbl->lookup_equal_range(source, in_out_targets);}
	/* See lookup_equal_range() in ro_string_table. */
	
	inline uint get_num_rows() {return _str_tbl->get_num_rows();}
	inline uint get_num_cols() {return _str_tbl->get_num_cols();}
	inline const char * get_str_at(uint row, uint col)
	{return _str_tbl->get_str_at(row, col);}
	
	inline void dbg_dump_tbl()
	{_str_tbl->dbg_dump();}
	
	
	static void first_line_to_field_names(const char * csv_file_name,
		char delim,
		on_field_split on_split,
		std::vector<std::string>& out
	);
	/*
	   Reads the first line from file_name and splits it in out. It's a
	   convenient way to obtain a list of the csv field names, given the first
	   line contains the names.
	*/
	
	protected:
	/* Protected so they can be made public in a test subclass. */
	static void _field_info_to_str_vect(const std::vector<field_info>& fi,
		std::vector<std::string>& out
	);
	static void _get_vect_as_cs_string(const std::vector<std::string>& vect,
		std::string& out
	);
	static bool _is_vect_a_set(const std::vector<std::string>& vec,
		std::string& out_offender
	);
	static bool _is_va_subset_of_vb(const std::vector<std::string>& va,
		const std::vector<std::string>& vb,
		std::string& out_offender
	);
	static bool _is_va_set_ordered_in_vb_set(const std::vector<std::string>& va,
		const std::vector<std::string>& vb,
		std::string& out_offender
	);
	
	
	private:
	void _field_checks(init_info& info);
	bool _should_skip_first_line(init_info& info); 
	void _fields_to_keep(init_info& info, std::set<uint>& out);
	void _init_str_tbl(init_info& info);
	
	static void _throw_empty_file(const char * fname);
	
	std::unique_ptr<ro_string_table> _str_tbl;
	std::vector<field_pair> _single_unq;
	std::vector<eq_range_result> _single_eqr;
};

#endif
