#ifndef INPUT_HPP
#define INPUT_HPP

#include <set>
#include <string>
#include <vector>
#include <fstream>

namespace input
{
	typedef unsigned int uint;
	
	void open_or_throw(const char * fname, std::ifstream& ifs);
	/* Associates fname with ifs. Throws if fname can't be opened. */
	
	uint count_lines_check_fld_number(const char * fname,
		char delim,
		uint field_number
	);
	/*
	   Returns the number of lines in fname. Throws if a line has a number of
	   fields delimited by delim different than field_number.
	*/
	
	void filter_fields_keep(const std::vector<const char *>& in,
		const std::set<uint>& fields_to_keep, // zero based
		std::vector<const char *>& out
	);
	/*
	   Places only those elements from in in out which are found at the indexes
	   specified by fields_to_keep. Throws if any index in fields_to_keep is
	   out of the bounds of in.
	*/
	
	void filter_fields_remove(const std::vector<const char *>& in,
		const std::set<uint>& fields_to_remove, // zero based
		std::vector<const char *>& out
	);	
	/*
	   Places only those elements from in in out which reside at indexes *not*
	   specified by fields_to_remove. Throws if any index in fields_to_remove is
	   out of the bounds of in. 
	*/
	
	uint split_string(std::string& str,
		char delim,
		std::vector<const char *>& out_split
	);
	/*
	   Replaces each occurrence of delim in str with '\0' and places a pointer
	   to the beginning of the newly formed C string in out_split.
	*/
}
#endif
