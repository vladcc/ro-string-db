#include "input.hpp"

#include <stdexcept>
#include <algorithm>

void input::open_or_throw(const char * fname, std::ifstream& ifs)
{
	ifs.clear();
	ifs.open(fname);
	if (!ifs)
	{
		std::string err("input::read_lines(): couldn't open file '");
		err += fname;
		err += "'";
		throw std::runtime_error(err);
	}
}

uint input::count_lines_check_fld_number(const char * fname,
	char delim,
	uint field_number
)
{
	std::ifstream in;
	open_or_throw(fname, in);
	
	uint lines = 0;
	std::string str;
	while(std::getline(in, str))
	{
		++lines;
		auto count = std::count(str.begin(), str.end(), delim)+1;
		if (count != field_number)
		{
			std::string err("input::count_lines_check_fld_number(): ");
			err += "number of fields ";
			err += std::to_string(count);
			err += " on line ";
			err += std::to_string(lines);
			err += " in file '";
			err += fname;
			err += "' different than the specified ";
			err += std::to_string(field_number);
			err += "; ";
			err += "line: '";
			err += str;
			err += "'; ";
			err += "delimiter given: '";
			err += delim;
			err += "'";
			throw std::runtime_error(err);
		}
	}
	in.close();
	
	return lines;
}

namespace
{
	void _throw_out_of_bouds(const char * prefix, uint field)
	{
		std::string err(prefix);
		err += " field number ";
		err += std::to_string(field);
		err += " out of bounds";
		throw std::runtime_error(err);
	}
}

void input::filter_fields_keep(const std::vector<const char *>& in,
	const std::set<uint>& fields_to_keep, // zero based
	std::vector<const char *>& out
)
{
	out.clear();
	uint all_fields = in.size();
	for (auto index : fields_to_keep)
	{
		if (index < all_fields)
			out.push_back(in[index]);
		else
			_throw_out_of_bouds("input::filter_fields_keep():", index);
	}
}

void input::filter_fields_remove(const std::vector<const char *>& in,
	const std::set<uint>& fields_to_remove, // zero based
	std::vector<const char *>& out
)
{
	out.clear();
	uint all_fileds = in.size();
	uint largest = *fields_to_remove.rbegin();
	
	if (largest < all_fileds)
	{
		for (uint i = 0; i < all_fileds; ++i)
		{
			if (!fields_to_remove.count(i))
				out.push_back(in[i]);
		}
	}
	else
		_throw_out_of_bouds("input::filter_fields_remove():", largest);
}

uint input::split_string(std::string& str,
	char delim,
	std::vector<const char *>& out_split
)
{
	out_split.clear();
	uint fields = 1;
	char * pstr = const_cast<char *>(str.c_str());
	
	out_split.push_back(pstr);
	for (uint i = 0, len = str.length(); i < len; ++i)
	{	
		if (pstr[i] == delim)
		{
			pstr[i] = '\0';
			++fields;
			out_split.push_back(pstr+i+1);
		}
	}
	
	return fields;
}
