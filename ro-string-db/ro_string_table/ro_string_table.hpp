#ifndef RO_STRING_TABLE_HPP
#define RO_STRING_TABLE_HPP

#include "matrix.ipp"
#include "string_pool.hpp"
#include "sort_vector.ipp"
#include "generic_compar.ipp"

#include <vector>
#include <string>

class ro_string_table
{
	public:
	typedef unsigned int uint;
	typedef unsigned char byte;

	struct field_pair {
		field_pair(const char * field_name, const char * field_value = nullptr)
			: field_name(field_name), field_value(field_value) {}
		const char * field_name;
		const char * field_value;
	};
	/* Used as a lookup source and as the result of a unique lookup. */
	
	struct eq_range_result {
		eq_range_result(const char * name) : field_name(name) {}
		std::vector<const char *> values;
		const char * field_name;
	};
	/* Equal range may return an array of values for each field name. */
	
    struct field_info
    {
        field_info(std::string name, bool is_unique = false) :
			name(name),
			is_unique(is_unique)
		{}
		
        std::string name;
        bool is_unique;
    };
	/*
	   And array of field_info defines which fields from the csv will be read
	   in memory. This array has to be a subset of all field names and have the
	   same relative order. Fields marked as unique are checked for duplicate
	   strings when seal() is called. An exception is thrown when a duplicate is
	   found.
	*/
	
	ro_string_table(uint lines,
        const std::vector<field_info>& fields,
        size_t pool_init_size = 0
    );
    /*
       ro_string_table has to know the number of lines in the input file in
       order to efficiently allocate its internal structures. This does not
       apply to the string pool, since less fields can be read than exist in the
       csv. However, if the user opts to pre-calculate the size of the pool,
       they can set pool_init_size in order to reserve space. Upon seal(),
       shrink_to_fit() is called on each structure anyway, however, the standard
       does not guarantee the shirk request will be honored.
    */
    
    inline void append(const std::string& str)
	{append(str.c_str());}
	
	void append(const char * str);
	/* 
	   Appends str to the internal string pool. An exception is thrown if the
	   number of strings exceeds lines * fields.size(), or the table has been
	   sealed.
	*/
	
	void seal();
	/*
	   Marks the table as sealed. This causes the internal structures to get
	   sorted, so a logarithmic lookup is possible. An attempt to release
	   excessive memory, if any allocated, is made. If an append is attempted
	   after a call to seal(), an exception is thrown.
	*/
	
	bool lookup_unique(const field_pair& source,
		std::vector<field_pair>& in_out_targets
	);
	/*
	   Looks up the field with the value specified by source. If it's not found,
	   false is returned. Otherwise true is returned and in_out_targets is
	   filled. The field_name in each field_pair in in_out_target must point
	   to a valid field name. Upon a successful return, each field_value in
	   in_out_targets points to the value of its field_name found on the same
	   line as source.
	   
	   lookup_unique() throws if any of the field_name fields in both source and
	   in_out_targets points to a name not found in the fields structure given
	   to the ro_string_table constructor, if the field named by source has not
	   been marked as unique, or if seal() has not been called prior to the
	   lookup.
	*/
	
	bool lookup_equal_range(const field_pair& source,
		std::vector<eq_range_result>& in_out_targets
	);
	/*
	   Like lookup_unique(), but source.field_value can exist more than once.
	   In that case, multiple values for each field_name in in_out_targets is
	   returned. Throws like lookup_unique(), with the exception that it does
	   not check source.field_name for uniqueness. Lookup takes twice as long,
	   since a lower and an upper bound have to be found.
	*/

	inline uint get_num_rows() {return _data_map.get_rows();}
	inline uint get_num_cols() {return _data_map.get_cols();}
	inline const char * get_str_at(uint row, uint col)
	{return _pool.get(_data_map.get(row, col));}
	/*
	   get_num_rows(), get_num_cols(), and get_str_at() allow for linear 
	   iteration of the whole csv as it exist in memory. row represents a line
	   number in the csv, col represents the field found at field number col
	   at line number row.
	*/

	void dbg_dump();
	/* Spits out internals as text. */

	private:
	struct num_field_info
    {
        num_field_info(uint line_num, uint index) :
			original_line_number(line_num),
			index_of_string(index)
		{}
		
        uint original_line_number;
        uint index_of_string;
    };
	/*
	   Each field string from the csv is internally represented by a
	   num_field_info. index_of_string points to the beginning of the respective
	   string inside the string pool, and original_line_number is used to
	   associate different fields to each other upon lookup.
	*/
	
	class single_field_data
    {
		/*
		   This class represents the data for a complete field from a csv
		   as if sliced vertically. It's a sorted vector of num_field_info along
		   with some other information which provides access to the string pool.
		*/
        public:
        typedef num_field_info nfi;
        
        struct context_lookup
        {
			context_lookup(const string_pool * str_pool = nullptr,
				const char * str = nullptr
			) :
				str_pool(str_pool),
				str(str)
			{}
			
			const string_pool * str_pool;
			const char * str;
		};
        /*
           Since strings in the sorted vector are represented by num_field_info,
           we need to know about the string pool in order to get the actual
           string. The value that we are looking for, however, is represented
           by an ordinary char *, so context_lookup allows us to transparently
           compare num_field_info to C strings.
        */
        
        single_field_data(
            int field_num,
            num_field_info name_id,
            const string_pool& str_pool,
            bool is_unique = false,
            uint init_vect_reserve = 0
        );
        
        inline void append_info(const nfi& num_fi)
        {_field_data.append(num_fi);}

        inline void seal()
        {
			_field_data.seal();
			_check_unique();
		}

        inline nfi get(int index) const
        {return _field_data.get(index);}

        inline bool lookup(const nfi ** out,
			gen_comp_less_ctx_lower_bound<nfi, context_lookup>& less_ctx
		)
        {
			nfi dummy(-1, -1);
			return _field_data.lookup(dummy, out, less_ctx);
		}
		
		inline bool equal_range(std::pair<size_t, size_t>& out,
			sort_vector<nfi, context_lookup>::equal_range_ctx_compars& cmps
		)
		{
			nfi dummy(-1, -1);
			return _field_data.equal_range(dummy, out, cmps);
		}
		
        inline int field_number() const
        {return _field_num;}

        inline void reserve(size_t how_many)
        {_field_data.reserve(how_many);}

		inline const char * get_name() const
		{return _str_pool->get(_field_name_id.index_of_string);}

		inline bool is_unique() const
		{return _is_unique;}

		void dbg_dump() const;

        private:
        void _check_unique();
        
        sort_vector<nfi, context_lookup> _field_data;
        num_field_info _field_name_id;
        const string_pool * _str_pool; // can't use default assignment if &
        int _field_num;
        bool _is_unique;
    };
	
	void _set_fields(const std::vector<field_info>& fields);
	uint _append_to_table(const char * str);
	bool _lookup_field(const char * name, const single_field_data ** out);
	bool _lookup_field_val(const ro_string_table::single_field_data& field,
		const char * val,
		const num_field_info ** out
	);
	
	void _dbg_dump_pool() const;
	void _throw_no_such_field(const char * field_name);
	void _throw_field_not_unique(const char * field_name);
	void _throw_not_sealed();
	
	typedef int (*string_context_lookup) (
		const ro_string_table::num_field_info& lhs,
		const ro_string_table::num_field_info& dummy_rhs,
		ro_string_table::single_field_data::context_lookup ctx
	);
	
	sort_vector<single_field_data, const char*> _fields;
	matrix<uint> _data_map;
	string_pool _pool;
	string_context_lookup _str_ctx_lup;
	bool _is_sealed;
	bool _are_fields_set;
	uint _num_lines;
	uint _num_fields;
	uint _current_line;
	uint _current_field;
};
#endif
