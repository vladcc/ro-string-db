#include "ro_string_db.hpp"

#include <fstream>
#include <stdexcept>

#define throw_str(str) "ro_string_db: " str

ro_string_db::ro_string_db(init_info& init)
{
	_single_unq.push_back(field_pair(""));
	_single_eqr.push_back(eq_range_result(""));
	
	on_field_split callback = init.on_field;
	if (callback) // normalize
	{
		for (auto& str : init.all_csv_field_names)
			callback(str);
		for (auto& inf : init.fields_to_keep)
			callback(inf.name);
	}
	
	_init_str_tbl(init);
}

void ro_string_db::first_line_to_field_names(const char * csv_file_name,
	char delim,
	on_field_split on_split,
	std::vector<std::string>& out
)
{
	out.clear();
	
	std::ifstream in;
	input::open_or_throw(csv_file_name, in);
	
	std::string line;
	std::getline(in, line);
	in.close();
	
	std::vector<const char *> split;
	input::split_string(line, delim, split);
	
	for (auto& pstr : split)
		out.push_back(pstr);
	
	if (on_split)
	{
		for (auto& str : out)
			on_split(str);
	}
}

bool ro_string_db::_should_skip_first_line(init_info& init)
{
	const char * fname = init.csv_file_name;
	char delim = init.delim;
	on_field_split callback = init.on_field;
	std::vector<std::string>& fld_names = init.all_csv_field_names;
	std::vector<std::string> from_file;
	first_line_to_field_names(fname, delim, callback, from_file);
	
	return (fld_names == from_file);
}

void ro_string_db::_fields_to_keep(init_info& init, std::set<uint>& out)
{
	_field_checks(init);
	
	out.clear();
	std::vector<std::string>& fld_names = init.all_csv_field_names;
	std::vector<field_info>& fields_to_keep = init.fields_to_keep;
	
	for (auto& info : fields_to_keep)
	{
		const std::string& keep = info.name;
		for (uint j = 0, flds_size = fld_names.size(); j < flds_size; ++j)
		{
			if (keep == fld_names[j])
				out.insert(j);
		}
	}
}

void ro_string_db::_init_str_tbl(init_info& init)
{
	std::vector<field_info>& tbl_fields = init.fields_to_keep;
	const char * fname = init.csv_file_name;
	char delim = init.delim;
	uint fields_num = init.all_csv_field_names.size();
	on_field_split callback = init.on_field;
	
	// check fields requested vs fields given, pick only the requested if ok
	std::set<uint> keep;
	_fields_to_keep(init, keep);
	
	// make sure the file has the declared number of fields on each line
	uint lines_num =
		input::count_lines_check_fld_number(fname, delim, fields_num);
	
	if (!lines_num)
		_throw_empty_file(fname);
	
	// know if the first line of the file is a header list of the field names
	bool skip_first = _should_skip_first_line(init);
	
	// open file, skip header, place in table
	std::ifstream in;
	input::open_or_throw(fname, in);
	
	std::string line;
	if (skip_first)
		std::getline(in, line);
	
	_str_tbl.reset(new ro_string_table(lines_num, tbl_fields));
	
	std::string field;
	const char * str_append;
	std::vector<byte> buff;
	std::vector<const char *> psplit, pkeep;
	while (std::getline(in, line))
	{
		input::split_string(line, delim, psplit);
		input::filter_fields_keep(psplit, keep, pkeep);
		
		for (uint i = 0, end = pkeep.size(); i < end; ++i)
		{
			str_append = pkeep[i];
			if (callback)
			{
				field = str_append;
				callback(field);
				str_append = field.c_str();
			}
			_str_tbl->append(str_append);
		}
	}
	_str_tbl->seal();
	in.close();
}

void ro_string_db::_field_checks(init_info& init)
{
	const char * fname = init.csv_file_name;
	std::vector<std::string>& fld_names = init.all_csv_field_names;
	std::vector<field_info>& fields_to_keep = init.fields_to_keep;
	
	std::vector<std::string> str_to_keep;
	ro_string_db::_field_info_to_str_vect(fields_to_keep, str_to_keep);
	
	std::string bad, line;
	
	if (!ro_string_db::_is_vect_a_set(fld_names, bad))
	{
		std::string err(throw_str("field list '"));
		_get_vect_as_cs_string(fld_names, line);
		err += line;
		err += "' for file '";
		err += fname;
		err += "' is not a set; '";
		err += bad;
		err += "' repeats";
		throw std::runtime_error(err);
	}
	
	if (!ro_string_db::_is_vect_a_set(str_to_keep, bad))
	{
		std::string err(throw_str("requested fields list '"));
		_get_vect_as_cs_string(str_to_keep, line);
		err += line;
		err += "' is not a set; '";
		err += bad;
		err += "' repeats";
		throw std::runtime_error(err);
	}
	
	if (!ro_string_db::_is_va_subset_of_vb(str_to_keep,
		fld_names,
		bad
	))
	{
		std::string err(throw_str("requested fields list '"));
		_get_vect_as_cs_string(str_to_keep, line);
		err += line;
		err += "' not a subset of declared fields list '";
		_get_vect_as_cs_string(fld_names, line);
		err += line;
		err += "' for file '";
		err += fname;
		err += "'; '";
		err += bad;
		err += "' not found in the declared list";
		throw std::runtime_error(err);
	}
	
	if (!ro_string_db::_is_va_set_ordered_in_vb_set(str_to_keep,
		fld_names,
		bad
	))
	{
		std::string err(throw_str("requested fields list '"));
		_get_vect_as_cs_string(str_to_keep, line);
		err += line;
		err += "' order not relative to the order of the declared fields list";
		err += " '";
		_get_vect_as_cs_string(fld_names, line);
		err += line;
		err += "' for file '";
		err += fname;
		err += "'; '";
		err += bad;
		err += "' is misplaced";
		throw std::runtime_error(err);
	}
}

void ro_string_db::_throw_empty_file(const char * fname)
{
	std::string err(throw_str("file '"));
	err += fname;
	err += "' is empty";
	throw std::runtime_error(err);
}

void ro_string_db::_field_info_to_str_vect(
	const std::vector<field_info> &fi,
	std::vector<std::string>& out
)
{
	out.clear();
	for (auto& i : fi)
		out.push_back(i.name);
}

void ro_string_db::_get_vect_as_cs_string(
	const std::vector<std::string>& vect,
	std::string& out
)
{
	out.clear();
	for (uint i = 0, end = vect.size(); i < end; ++i)
	{
		out += vect[i];
		
		if (i < end-1)
			out += ',';
	}
}

bool ro_string_db::_is_va_subset_of_vb(const std::vector<std::string>& va,
	const std::vector<std::string>& vb,
	std::string& out_offender
)
{
	bool found = false;
	
	for (uint i = 0, va_end = va.size(); i < va_end; ++i)
	{
		found = false;
		for (auto& str : vb)
		{
			if (va[i] == str)
				found = true;
		}
		
		if (!found)
		{
			out_offender = va[i];
			break;
		}
	}
	
	return found;
}

bool ro_string_db::_is_vect_a_set(const std::vector<std::string>& vec,
	std::string& out_offender
)
{
	bool ret = true;
	std::set<std::string> bucket;
	
	for (auto& str : vec)
	{
		if (bucket.count(str))
		{
			out_offender = str;
			ret = false;
			break;
		}
		else
			bucket.insert(str);
	}
	
	return ret;
}

bool ro_string_db::_is_va_set_ordered_in_vb_set(
	const std::vector<std::string>& va,
	const std::vector<std::string>& vb,
	std::string& out_offender
)
{
	bool ret = true;
	
	int last_at = -1;
	for (uint i = 0, va_end = va.size(); i < va_end; ++i)
	{
		int new_at = -1;
		for (uint j = 0, vb_end = vb.size(); j < vb_end; ++j)
		{
			if (va[i] == vb[j])
			{
				new_at = j;
				break;
			}
		}
		
		if (new_at > last_at)
			last_at = new_at;
		else
		{
			out_offender = va[i];
			ret = false;
			break;
		}
	}
	
	return ret;
}
