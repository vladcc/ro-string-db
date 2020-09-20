#include "ro_string_table.hpp"
#include <stdexcept>
#include <cstring>
#include <string>
#include <cstring>

// dbg
#include <iostream>

#define throw_str(str) "ro_string_table: " str

// class ro_string_table
ro_string_table::ro_string_table(uint lines,
        const std::vector<field_info>& fields,
        size_t pool_init_size
) :
	_fields(
		gen_comp_less<ro_string_table::single_field_data, const char*>(
			[](const ro_string_table::single_field_data& lhs,
				const ro_string_table::single_field_data& rhs,
				const char * ignored
			)
			{
				return strcmp(lhs.get_name(), rhs.get_name());
			}
		)
	),
	_data_map(lines, fields.size()),
	_pool(pool_init_size),
	_str_ctx_lup(
		[](const ro_string_table::num_field_info& lhs,
			const ro_string_table::num_field_info& dummy_rhs,
			ro_string_table::single_field_data::context_lookup ctx
		)
		{
			return strcmp(ctx.str_pool->get(lhs.index_of_string), ctx.str);
		}
	),
	_is_sealed(false),
	_are_fields_set(false),
	_current_line(0),
	_current_field(0)
{
	_num_lines = _data_map.get_rows();
	_num_fields = _data_map.get_cols();
	
	_fields.reserve(fields.size());
	_set_fields(fields);
}

void ro_string_table::_set_fields(const std::vector<field_info>& fields)
{
	if (!_are_fields_set)
	{
		for (uint i = 0, end = fields.size(); i < end; ++i)
		{
			auto& field = fields[i];
			uint place_in_pool = _append_to_table(field.name.c_str());
			
			ro_string_table::num_field_info tmp(0, place_in_pool);
			_fields.append(
				single_field_data(i, tmp, _pool, field.is_unique, _num_lines)
			);
		}
		_are_fields_set = true;
	}
	else
		throw std::runtime_error(throw_str("can't set fields twice"));
}

void ro_string_table::append(const char * str)
{
	if (!_is_sealed)
	{
		int line_number = _current_line;
		int field = _current_field;
		int place_in_pool = _append_to_table(str);
		
		ro_string_table::num_field_info numfi(line_number, place_in_pool);
		const auto& noconst = _fields.get(field);
		const_cast<ro_string_table::single_field_data&>
			(noconst).append_info(numfi);
	}
	else
		throw std::runtime_error(throw_str("append() called after seal()"));
}

uint ro_string_table::_append_to_table(const char * str)
{
	if (_current_line < _num_lines)
	{
		uint place_in_pool = _pool.append(str);
		_data_map.place(_current_line, _current_field, place_in_pool);

		++_current_field;
		if (_current_field == _num_fields)
		{
			++_current_line;
			_current_field = 0;
		}
		
		return place_in_pool;
	}
	else
	{
		std::string err(throw_str("more lines added than the specified "));
		err += std::to_string(_num_lines);
		err += "; the names of the requested fields mismatch the names of the ";
		err += "fields in the file?";
		throw std::runtime_error(err);
	}
}

void ro_string_table::seal()
{
	for (int i = 0, end = _fields.size(); i < end; ++i)
	{
		const auto& noconst = _fields.get(i);
		const_cast<ro_string_table::single_field_data&>(noconst).seal();
	}
	_fields.seal();
	_pool.shrink_to_fit();
	_is_sealed = true;
}

bool ro_string_table::_lookup_field(const char * name,
	const ro_string_table::single_field_data ** out
)
{
	gen_comp_less_ctx_lower_bound<ro_string_table::single_field_data,
		const char*>
	str_less(
		[](const ro_string_table::single_field_data& lhs,
			const ro_string_table::single_field_data& dummy_rhs,
			const char * real_rhs
		)
		{
			return strcmp(lhs.get_name(), real_rhs);
		},
		name
	);
	
	const ro_string_table::single_field_data& dummy = _fields.get(0);
	return _fields.lookup(dummy, out, str_less);
}

bool ro_string_table::_lookup_field_val(
	const ro_string_table::single_field_data& field,
	const char * val,
	const num_field_info ** out
)
{
	gen_comp_less_ctx_lower_bound<ro_string_table::num_field_info,
		ro_string_table::single_field_data::context_lookup>
		less_val_ctx(_str_ctx_lup,
			ro_string_table::single_field_data::context_lookup(&_pool, val)
		);
	
	auto& noconst = const_cast<ro_string_table::single_field_data&>(field);
	return noconst.lookup(out, less_val_ctx);
}

bool ro_string_table::lookup_unique(const field_pair& source,
	std::vector<field_pair>& in_out_targets
)
{
	bool ret = false;
	
	if (_is_sealed)
	{	
		const ro_string_table::single_field_data * out_sfd_ = nullptr;
		const ro_string_table::single_field_data ** out_sfd = &out_sfd_;
		
		if (_lookup_field(source.field_name, out_sfd))
		{
			const ro_string_table::single_field_data& source_field = **out_sfd;
			if (source_field.is_unique())
			{
				const ro_string_table::num_field_info * out_nfi_ = nullptr;
				const ro_string_table::num_field_info ** out_nfi = &out_nfi_;
				if (_lookup_field_val(source_field,
						source.field_value,
						out_nfi
					))
				{
					for (field_pair& pair : in_out_targets)
					{						
						if (_lookup_field(pair.field_name, out_sfd))
						{
							uint value_row = (*out_nfi)->original_line_number;
							uint value_col = (*out_sfd)->field_number();	
							pair.field_value = 
								_pool.get(_data_map.get(value_row, value_col));
						}
						else
							_throw_no_such_field(pair.field_name);
					}
					ret = true;
				}
			}
			else
				_throw_field_not_unique(source.field_name);
		}
		else
			_throw_no_such_field(source.field_name);
	}
	else
		_throw_not_sealed();
		
	return ret;
}

bool ro_string_table::lookup_equal_range(const field_pair& source,
	std::vector<eq_range_result>& in_out_targets
)
{
	bool ret = false;
	
	if (_is_sealed)
	{
		const ro_string_table::single_field_data * out_sfd_ = nullptr;
		const ro_string_table::single_field_data ** out_sfd = &out_sfd_;
		if (_lookup_field(source.field_name, out_sfd))
		{
			ro_string_table::single_field_data&
				source_field =
					const_cast<ro_string_table::single_field_data&>(**out_sfd);
				
			gen_comp_less_ctx_lower_bound<ro_string_table::num_field_info,
				ro_string_table::single_field_data::context_lookup>
				less_lwr_ctx(_str_ctx_lup);
			
			gen_comp_less_ctx_upper_bound<ro_string_table::num_field_info,
				ro_string_table::single_field_data::context_lookup>
				less_upr_ctx(_str_ctx_lup);
			
			sort_vector<ro_string_table::num_field_info,
				ro_string_table::single_field_data::context_lookup>
			::equal_range_ctx_compars cmprs(less_lwr_ctx, less_upr_ctx,
				ro_string_table::single_field_data
				::context_lookup(&_pool, source.field_value)
			);
			
			std::pair<size_t, size_t> range;
			if (source_field.equal_range(range, cmprs))
			{
				for (eq_range_result& elem : in_out_targets)
				{
					const char * res_fld_name = elem.field_name;
					std::vector<const char *>& res_vect = elem.values;
					
					res_vect.clear();
					if (_lookup_field(res_fld_name, out_sfd))
					{
						uint value_col = (*out_sfd)->field_number();
						for (size_t i = range.first; i < range.second; ++i)
						{
							uint value_row =
								source_field.get(i).original_line_number;
							res_vect.push_back(
								_pool.get(_data_map.get(value_row, value_col))
							);
						}
					}
					else
						_throw_no_such_field(res_fld_name);
				}
				ret = true;
			}
		}
		else
			_throw_no_such_field(source.field_name);
	}
	else
		_throw_not_sealed();
		
	return ret;
}

void ro_string_table::_throw_no_such_field(const char * field_name)
{
	std::string err(throw_str("lookup fail: no such field '"));
	err += field_name;
	err += "'";
	throw std::runtime_error(err);
}

void ro_string_table::_throw_field_not_unique(const char * field_name)
{
	std::string err(throw_str("unique lookup of non-unique field '"));
	err += field_name;
	err += "'";
	throw std::runtime_error(err);
}

void ro_string_table::_throw_not_sealed()
{
	throw std::runtime_error(throw_str("lookup before seal()"));
}

void ro_string_table::_dbg_dump_pool() const
{
	int len = 0;
	for (size_t i = 0; i < _pool.size(); ++i)
	{
		const char * pch = _pool.get(i);
		if (*pch)
		{
			std::cout << *pch;
			++len;
		}
		else
		{
			std::cout << "|" << static_cast<int>(i-len) << "|";
			len = 0;
		}
	}
	std::cout << std::endl;
}

void ro_string_table::dbg_dump()
{
	std::cout << "@@@ FIELDS @@@" << std::endl;
	for (size_t i = 0, end = _fields.size(); i < end; ++i)
	{
		const auto& noconst = _fields.get(i);
		const_cast<ro_string_table::single_field_data&>(noconst).dbg_dump();
	}
	std::cout << std::endl;
	
	std::cout << "@@@ POOL @@@" << std::endl;
	_dbg_dump_pool();
	
	std::cout << std::endl;
	
	std::cout << "@@@ TABLE @@@" << std::endl;
	for (uint i = 0; i < _num_lines; ++i)
	{
		for (uint j = 0; j < _num_fields; ++j)
			std::cout << get_str_at(i, j) << " ";
		std::cout << std::endl;
	}
}

// class ro_string_table::single_field_data
ro_string_table::single_field_data::single_field_data(
	int field_num,
	num_field_info name_id,
	const string_pool& str_pool,
	bool is_unique,
	uint init_vect_reserve
) :
	_field_data(
		gen_comp_less<ro_string_table::num_field_info,
			ro_string_table::single_field_data::context_lookup>(
			[](const ro_string_table::num_field_info& lhs,
				const ro_string_table::num_field_info& rhs,
				ro_string_table::single_field_data::context_lookup ctx
			)
			{
				const char * str_lhs = ctx.str_pool->get(lhs.index_of_string);
				const char * str_rhs = ctx.str_pool->get(rhs.index_of_string);
				return strcmp(str_lhs, str_rhs);
			},
			ro_string_table::single_field_data::context_lookup(&str_pool,
				nullptr
			)
		)
	),
	_field_name_id(name_id),
	_str_pool(&str_pool),
	_field_num(field_num),
	_is_unique(is_unique)
{
	_field_data.reserve(init_vect_reserve);
}

void ro_string_table::single_field_data::_check_unique()
{
	const char * stra = nullptr, * strb = nullptr;
	if (_is_unique)
	{
		for (size_t i = 1; i < _field_data.size(); ++i)
		{
			single_field_data::nfi a = _field_data.get(i-1);
			single_field_data::nfi b = _field_data.get(i);
			stra = _str_pool->get(a.index_of_string);
			strb = _str_pool->get(b.index_of_string);
			
			if (0 == strcmp(stra, strb))
				goto _throw;
		}
	}
	
	return;
	
_throw:
	std::string err("single_field_data::check_unique(): ");
	err += "string '";
	err += stra;
	err += "' appears more than once in field '";
	err += get_name();
	err += "' marked as unique";
	throw std::runtime_error(err);
}

void ro_string_table::single_field_data::dbg_dump() const
{
	std::cout << get_name() << ":";
	for (int i = 0, end = _field_data.size(); i < end; ++i)
	{
		auto tmp = _field_data.get(i);
		std::cout << tmp.original_line_number
			<< " " << tmp.index_of_string
			<< " " << _str_pool->get(tmp.index_of_string)
			<< " ";
	}
	std::cout << std::endl;
}
