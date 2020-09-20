extern "C" {
#include "self_stat.h"
#include "parse_opts.h"
}

#include "ro_string_db.hpp"
#include "input.hpp"

#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <cstdio>

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <set>

#include <unistd.h>

static const char program_name[] = "query_driver";
static const char program_ver[] = "1.0";

static void equit_(const char * msg, ...);
#define equit(str, ...) equit_("%s: error: " str, program_name, __VA_ARGS__)

static void get_csv_fields(const char * opt, char * opt_arg, void * ctx);
static void get_field_info(const char * opt, char * opt_arg, void * ctx);
static void get_source(const char * opt, char * opt_arg, void * ctx);
static void get_targets(const char * opt, char * opt_arg, void * ctx);
static void read_query_file(const char * opt, char * opt_arg, void * ctx);
static void help_message();

#define print_try()\
fprintf(stderr, "Try '%s --%s' for more information\n",\
program_name, help_opt_long)

enum {UNIQ, EQR};

struct program_options {
	program_options() :
		in_file(nullptr),
		delimiter('\0'),
		dump_in_file(false)
	{}
	
	std::vector<ro_string_db::field_info> finfo;
	std::vector<std::string> csv_fields;
	std::vector<std::string> query_file_text;
	std::vector<ro_string_db::field_pair> sources;
	std::vector<std::vector<const char *>> targets;
	std::vector<int> lookups;
	const char * in_file;
	char delimiter;
	bool dump_in_file;
};

// --input-file|-i
static const char input_file_opt_short = 'i';
static const char input_file_opt_long[] = "input-file";
static void handle_input_file(const char * opt, char * opt_arg, void * ctx)
{
	program_options * opts = (program_options *)(ctx);
	opts->in_file = opt_arg;
}

static void help_input_file(const char * short_name, const char * long_name)
{
printf("%s|%s <input-csv-file-name>\n", short_name, long_name);
}

// --query-file|-q
static const char query_file_opt_short = 'q';
static const char query_file_opt_long[] = "query-file";
static void handle_query_file(const char * opt, char * opt_arg, void * ctx)
{
	read_query_file(opt, opt_arg, ctx);
}

static void help_query_file(const char * short_name, const char * long_name)
{
printf("%s|%s <query-file-name> - this is optional\n",short_name, long_name);
}

// --delimiter|-d
static const char delimiter_opt_short = 'd';
static const char delimiter_opt_long[] = "delimiter";
static void handle_delimiter(const char * opt, char * opt_arg, void * ctx)
{
	program_options * opts = (program_options *)(ctx);
	opts->delimiter = *opt_arg;
}

static void help_delimiter(const char * short_name, const char * long_name)
{
printf("%s|%s <csv-delimiter-character>\n", short_name, long_name);
}

// --field-info|-f
static const char field_info_opt_short = 'f';
static const char field_info_opt_long[] = "field-info";
static void handle_field_info(const char * opt, char * opt_arg, void * ctx)
{
	get_field_info(opt, opt_arg, ctx);
}

static void help_field_info(const char * short_name, const char * long_name)
{
printf("%s|%s <field-information>\n", short_name, long_name);
puts("<field-information> describes which fields from the csv file will be");
puts("read in memory and if a particular field contains only unique strings");
puts("and therefore can be looked up with binary search and will produce");
puts("a maximum of one results. This is in contrast with fields which can");
puts("contain repeating strings and are therefore looked up with equal range");
puts("and can return more than one result. <field-information> is a comma");
puts("separated list like so: <field-1>=<1/0>,<field-2>=<1/0>.. etc.");
puts("");
}

// --csv-fields|-c
static const char csv_fields_opt_short = 'c';
static const char csv_fields_opt_long[] = "csv-fields";
static void handle_csv_fields(const char * opt, char * opt_arg, void * ctx)
{
	get_csv_fields(opt, opt_arg, ctx);
}

static void help_csv_fields(const char * short_name, const char * long_name)
{
printf("%s|%s <comma-separated-field-names>\n", short_name, long_name);
puts("This is optional. If it exists it describes the field names of the");
puts("csv. If it doesn't, the names are taken from the first line of the");
puts("file.");
puts("");
}

// --source|-s
static const char source_opt_short = 's';
static const char source_opt_long[] = "source";
static void handle_source(const char * opt, char * opt_arg, void * ctx)
{
	get_source(opt, opt_arg, ctx);
}

static void help_source(const char * short_name, const char * long_name)
{
printf("%s|%s <field-name>=<value> - a field value pair to look for\n",
short_name, long_name);
}

// --targets|-t
static const char targets_opt_short = 't';
static const char targets_opt_long[] = "targets";
static void handle_targets(const char * opt, char * opt_arg, void * ctx)
{
	get_targets(opt, opt_arg, ctx);
}

static void help_targets(const char * short_name, const char * long_name)
{
printf("%s|%s <target-field-names>\n", short_name, long_name);
puts("<target-field-names> is a comma separated list of the field names the");
puts("values to which the lookup will retrieve. The fields must exist.");
puts("");
}

// --lookup-unique|-u
static const char lookup_unique_opt_short = 'u';
static const char lookup_unique_opt_long[] = "lookup-unique";
static void handle_lookup_unique(const char * opt, char * opt_arg, void * ctx)
{
	program_options * opts = (program_options *)(ctx);
	opts->lookups.push_back(UNIQ);
}

static void help_lookup_unique(const char * short_name, const char * long_name)
{
printf("%s|%s - specifies that the source field contains only unique strings\n",
short_name, long_name);
}

// --lookup-equal-range|-e
static const char lookup_equal_range_opt_short = 'e';
static const char lookup_equal_range_opt_long[] = "lookup-equal-range";
static void handle_lookup_equal_range(const char * opt, char * opt_arg, void * ctx)
{
	program_options * opts = (program_options *)(ctx);
	opts->lookups.push_back(EQR);
}

static void help_lookup_equal_range(const char * short_name, const char * long_name)
{
printf("%s|%s - specifies that the source field may contain non-unique "
"strings\n", short_name, long_name);
}

// --dump|-D
static const char dump_opt_short = 'D';
static const char dump_opt_long[] = "dump";
static void handle_dump(const char * opt, char * opt_arg, void * ctx)
{
	program_options * opts = (program_options *)(ctx);
	opts->dump_in_file = true;
}

static void help_dump(const char * short_name, const char * long_name)
{
printf("%s|%s - dumps the csv as it exist in memory\n", short_name, long_name);
}

// --help|-h
static const char help_opt_short = 'h';
static const char help_opt_long[] = "help";
static void handle_help(const char * opt, char * opt_arg, void * ctx)
{
	help_message();
	opts_print_help((opts_table *)ctx);
	exit(EXIT_SUCCESS);
}

static void help_help(const char * short_name, const char * long_name)
{
printf("%s|%s - this message\n", short_name, long_name);
}

// --version|-v
static const char version_opt_short = 'v';
static const char version_opt_long[] = "version";
static void handle_version(const char * opt, char * opt_arg, void * ctx)
{
		printf("%s v%s\n", program_name, program_ver);
		exit(EXIT_SUCCESS);
}

static void help_version(const char * short_name, const char * long_name)
{
printf("%s|%s - version info\n", short_name, long_name);
}

// on_unbound_arg
static void on_unbound_arg(const char * arg, void * ctx)
{
	equit("argument without option: %s\n", arg);
}

// on_error
static void on_error(opts_err_code err_code, const char * err_opt, void * ctx)
{
	FILE * where = stdout; // preserve output order
	fprintf(where, "%s: error: ", program_name);
	switch (err_code)
	{
		case OPTS_UNKOWN_OPT_ERR:
			fprintf(where, "option '%s' unknown\n", err_opt);
		break;
		case OPTS_ARG_REQ_ERR:
			fprintf(where, "option '%s' requires an argument\n", err_opt);
		break;
		case OPTS_NO_ARG_REQ_ERR:
			fprintf(where, "option '%s' does not take arguments\n", err_opt);
		break;
		default:
		break;
	}
	
	exit(EXIT_FAILURE);
}

static void equit_(const char * msg, ...)
{
	va_list args;
	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end (args);
	fprintf(stderr, "%s", "\n");
	print_try();
	exit(EXIT_FAILURE);
}

static void get_field_info(const char * opt, char * opt_arg, void * ctx)
{
	char * sub_arg, ** parg = &opt_arg;
	while ((sub_arg = opts_get_sub_arg(parg, ',')))
	{
		char ** pparg = &sub_arg;
		char * name = opts_get_sub_arg(pparg, '=');
		char * unique = opts_get_sub_arg(pparg, '=');
		
		if (!unique)
		{
			equit("%s %s", "bad field info syntax;",
				"should be <field1>=<1/0>[,<field2>=<1/0>,...]"); 
		}
		else
		{
			int is_unique;
			if (sscanf(unique, "%d", &is_unique) != 1)
				equit("option '%s': '%s' bad number", opt, unique);
			else
			{
				if (is_unique != 0 && is_unique != 1)
				{
					equit("option '%s': '%s' has to be either 1 or 0",
						opt,
						unique
					);
				}
				else
				{
					program_options * opts = (program_options *)(ctx);
					opts->finfo.push_back(
						ro_string_db::field_info(name, is_unique)
					);
				}
			}
		}
	}
}

static void get_csv_fields(const char * opt, char * opt_arg, void * ctx)
{
	program_options * opts = (program_options *)(ctx);
	std::vector<std::string>& csvf = opts->csv_fields;
	
	char * sub_arg, ** parg = &opt_arg;
	while ((sub_arg = opts_get_sub_arg(parg, ',')))
		csvf.push_back(sub_arg);
}

static void get_source(const char * opt, char * opt_arg, void * ctx)
{
	char * name, ** parg = &opt_arg;
	if ((name = opts_get_sub_arg(parg, '=')))
	{
		char * val = opts_get_sub_arg(parg, '=');
		if (!val)
		{
			equit("%s %s", "bad source field syntax;",
				"should be <field>=<name>"
			);
		}
		else
		{
			program_options * opts = (program_options *)(ctx);
			opts->sources.push_back(ro_string_db::field_pair(name, val));
		}
	}
}

static void get_targets(const char * opt, char * opt_arg, void * ctx)
{
	program_options * opts = (program_options *)(ctx);
	opts->targets.push_back(std::vector<const char *>());
	std::vector<const char *>& tgts = opts->targets.back();
	
	char * sub_arg, ** parg = &opt_arg;
	while ((sub_arg = opts_get_sub_arg(parg, ',')))
		tgts.push_back(sub_arg);
}

static void read_query_file(const char * opt, char * opt_arg, void * ctx)
{	
	program_options * opts = (program_options *)(ctx);
	std::vector<std::string>& text = opts->query_file_text;
	
	try
	{
		std::ifstream in;
		input::open_or_throw(opt_arg, in);
		
		std::string line;
		while (std::getline(in, line))
			text.push_back(line);
	}
	catch (std::exception& e)
	{equit("%s", e.what());}
	
	for (auto& line : text)
	{
		if (line[0] != '#')
		{
			char * pline = const_cast<char *>(line.c_str());
			char * sub_arg, ** parg = &pline;
			if ((sub_arg = opts_get_sub_arg(parg, ';')))
			{
				get_source(opt, sub_arg, ctx);
				if ((sub_arg = opts_get_sub_arg(parg, ';')))
				{
					get_targets(opt, sub_arg, ctx);
					if ((sub_arg = opts_get_sub_arg(parg, ';')))
						opts->lookups.push_back((*sub_arg == 'u') ? UNIQ : EQR);
				}	
			}
		}
	}
}

static void help_message()
{
printf("--- %s v%s ---\n", program_name, program_ver);
puts("A driver for ro_string_db. You can test lookups. Provides timing and");
puts("memory info. For a lookup from the command line you need to provide:");
puts("1. an input file in csv format");
puts("2. the delimiter character");
puts("3. information about which fields to keep and if they are unique");
puts("4. a source field value pair for the lookup");
puts("5. one or more target fields to retrieve the values of");
puts("6. the type of the lookup - unique fields are looked up with binary");
puts("search, non-unique are looked up with equal range");
puts("");
puts("Example:");
puts("If the file looks like this:");
puts("");
puts("id;fruit;type;price");
puts("1;pineapple;fancy;12.25");
puts("2;apple;normal;5.32");
puts("3;peach;normal;4.22");
puts("4;mango;fancy;10.50");
puts("5;pear;normal;6.00");
puts("");
printf("%s -%c <input-file> -%c ';' -%c id=1,fruit=1,type=0 -%c id=2 -%c "
"fruit,type -%c\n", program_name, input_file_opt_short, delimiter_opt_short,
field_info_opt_short, source_opt_short, targets_opt_short,
lookup_unique_opt_short);
puts("Will read the input file, discard the 'price' field so it doesn't take");
puts("any memory, lookup the line on which id=2 and return the 'fruit' and");
puts("'type' fields for that line - namely 'apple' and 'normal'. Queries");
puts("can also be placed in a file and executed one after the other in a");
puts("single process. The file has the syntax:");
puts("<source>;<targets>;<lookup-type>");
puts("");
puts("Example:");
puts("If the query file looks like:");
puts("");
puts("id=2;fruit,type;u");
puts("");
printf("%s -%c <input-file> -%c ';' -%c id=1,fruit=1,type=0 -%c <query-file>\n",
program_name, input_file_opt_short, delimiter_opt_short, field_info_opt_short,
query_file_opt_short);
puts("Will do the same as above. Lines beginning with '#' are ignored.");
puts("");
puts("Options:");
}

void remove_quotes(std::string& field)
{
	field.erase(
		std::remove_if(field.begin(),
			field.end(),
			[](char ch)
			{
				return (ch == '\'' || ch == '"');
			}
		),
		field.end()
	);
}

void handle_optios(int argc, char * argv[], program_options& opts)
{
	opts_table the_tbl;
	opts_entry all_entries[] = {
		{
			.names = {
				.long_name = input_file_opt_long,
				.short_name = input_file_opt_short
			},
			.handler = {
				.handler = handle_input_file,
				.context = (void *)(&opts),
			},
			.print_help = help_input_file,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = query_file_opt_long,
				.short_name = query_file_opt_short
			},
			.handler = {
				.handler = handle_query_file,
				.context = (void *)(&opts),
			},
			.print_help = help_query_file,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = delimiter_opt_long,
				.short_name = delimiter_opt_short
			},
			.handler = {
				.handler = handle_delimiter,
				.context = (void *)(&opts),
			},
			.print_help = help_delimiter,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = field_info_opt_long,
				.short_name = field_info_opt_short
			},
			.handler = {
				.handler = handle_field_info,
				.context = (void *)(&opts),
			},
			.print_help = help_field_info,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = csv_fields_opt_long,
				.short_name = csv_fields_opt_short
			},
			.handler = {
				.handler = handle_csv_fields,
				.context = (void *)(&opts),
			},
			.print_help = help_csv_fields,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = source_opt_long,
				.short_name = source_opt_short
			},
			.handler = {
				.handler = handle_source,
				.context = (void *)(&opts),
			},
			.print_help = help_source,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = targets_opt_long,
				.short_name = targets_opt_short
			},
			.handler = {
				.handler = handle_targets,
				.context = (void *)(&opts),
			},
			.print_help = help_targets,
			.takes_arg = true,
		},
		{
			.names = {
				.long_name = lookup_unique_opt_long,
				.short_name = lookup_unique_opt_short
			},
			.handler = {
				.handler = handle_lookup_unique,
				.context = (void *)(&opts),
			},
			.print_help = help_lookup_unique,
			.takes_arg = false,
		},
		{
			.names = {
				.long_name = lookup_equal_range_opt_long,
				.short_name = lookup_equal_range_opt_short
			},
			.handler = {
				.handler = handle_lookup_equal_range,
				.context = (void *)(&opts),
			},
			.print_help = help_lookup_equal_range,
			.takes_arg = false,
		},
		{
			.names = {
				.long_name = dump_opt_long,
				.short_name = dump_opt_short
			},
			.handler = {
				.handler = handle_dump,
				.context = (void *)(&opts),
			},
			.print_help = help_dump,
			.takes_arg = false,
		},
		{
			.names = {
				.long_name = help_opt_long,
				.short_name = help_opt_short
			},
			.handler = {
				.handler = handle_help,
				.context = (void *)(&the_tbl),
			},
			.print_help = help_help,
			.takes_arg = false,
		},
		{
			.names = {
				.long_name = version_opt_long,
				.short_name = version_opt_short
			},
			.handler = {
				.handler = handle_version,
				.context = (void *)(&opts),
			},
			.print_help = help_version,
			.takes_arg = false,
		},
	};

	the_tbl.tbl = all_entries;
	the_tbl.length = sizeof(all_entries)/sizeof(*all_entries);

	opts_parse_data parse_data = {
		.the_tbl = &the_tbl,
		.on_unbound = {
			.handler = on_unbound_arg,
			.context = (void *)(&opts),
		},
		.on_error = {
			.handler = on_error,
			.context = (void *)(&opts),
		}
	};

	opts_parse(argc-1, argv+1, &parse_data);
}

ro_string_db * make_db(program_options &opts)
{
	const char * fname = opts.in_file;
	char delim = opts.delimiter;
	std::vector<ro_string_db::field_info>& fields = opts.finfo;
	std::vector<std::string>& csvf = opts.csv_fields;
	
	if (csvf.empty())
	{
		ro_string_db::first_line_to_field_names(fname,
			delim,
			remove_quotes,
			csvf
		);
	}
	
	ro_string_db::init_info init(fname, delim, csvf, fields, remove_quotes);
	
	return (new ro_string_db(init));
}

size_t rss_in_kb()
{
	sst_proc_statm ps;
	if (!sst_read_self_statm(&ps))
		equit("%s", "stt_read_self_statm() failed");
	return (ps.resident*(sysconf(_SC_PAGESIZE)/1024));
}

size_t print_rss(const char * opt)
{
	std::cout << "RSS ";
	if (opt)
		std::cout << opt << ' ';
	
	size_t rss = rss_in_kb();
	std::cout << rss << " kb" << std::endl;
	
	return rss;
}

struct pg_prv {
	size_t cl;
	size_t dr;
};

void parse_for_private(char * line, void * context)
{
	static const char pr_cl[] = "Private_Clean:";
	static const char pr_dr[] = "Private_Dirty:";
	static const char fmt[] = "%127s %zu";
	
	size_t cl = 0, dr = 0;
	pg_prv * out = (pg_prv *)context;
	
	char buff[128] = {0};
	if (strstr(line, pr_cl))
		sscanf(line, fmt, buff, &cl);
		
	if (strstr(line, pr_dr))
		sscanf(line, fmt, buff, &dr);
		
	out->cl += cl;
	out->dr += dr;
}

size_t private_cl_dr_in_kb(const char * opt)
{
	pg_prv pgs;
	memset((void *)&pgs, 0, sizeof(pgs));
	sst_read_self_smaps(parse_for_private, (void *)&pgs);
	std::cout << "Private clean + dirty ";
	if (opt)
		std::cout << opt << " : ";
	
	size_t all = pgs.cl + pgs.dr;
	std::cout << all << " kb\n";
		
	return all;
}

void process(program_options& opts)
{
	size_t ppgs1 = private_cl_dr_in_kb("before string_db");
	size_t rss1 = print_rss("before string_db");
	
	std::unique_ptr<ro_string_db> _str_db(make_db(opts));
	
	size_t ppgs2 = private_cl_dr_in_kb("after string_db");
	size_t rss2 = print_rss("after string_db");
	std::cout << "RSS delta " << rss2 - rss1 << " kb" << std::endl;
	std::cout << "Private delta: " << ppgs2 - ppgs1 << " kb\n" << std::endl;
	
	ro_string_db& str_db = *_str_db;
	
	if (opts.dump_in_file)
	{
		for (uint i = 0, end = str_db.get_num_rows(); i < end; ++i)
		{
			for (uint j = 0, end = str_db.get_num_cols(); j < end; ++j)
			{
				std::cout << str_db.get_str_at(i, j);
				
				if (j < end-1)
					std::cout << opts.delimiter;
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
	
	std::vector<ro_string_db::field_pair>& sources = opts.sources;
	std::vector<std::vector<const char *>>& targets = opts.targets;
	std::vector<int>& lookups = opts.lookups;
	
	if (sources.empty())
		equit("%s", "no sources for lookup");
	if (targets.empty())
		equit("%s", "no targets for lookup");
	if (lookups.empty())
		equit("%s", "no lookup specified");
			
	for (uint i = 0, end = lookups.size(); i < end; ++i)
	{
		int lookup = lookups[i];
		if (lookup == UNIQ)
		{
			ro_string_db::field_pair& src = sources[i];
			std::vector<const char *>& tgts = targets[i];
			std::vector<ro_string_db::field_pair> fpar_in_out_tgts;
			for (uint k = 0, end = tgts.size(); k < end; ++k)
				fpar_in_out_tgts.push_back(ro_string_db::field_pair(tgts[k]));
			
			auto start = std::chrono::steady_clock::now();
			
			str_db.lookup_unique(src, fpar_in_out_tgts);
			
			auto end = std::chrono::steady_clock::now();
			auto nanos =
				std::chrono::duration_cast
					<std::chrono::nanoseconds>(end - start);
			
			auto micros =
				std::chrono::duration_cast
					<std::chrono::microseconds>(end - start);
			
			auto mills =
				std::chrono::duration_cast
					<std::chrono::milliseconds>(end - start);
			
			
			std::cout << "lookup type   : unique" << std::endl;
			std::cout << "lookup source : "
				<< src.field_name << " = " << src.field_value << std::endl;
			std::cout << "lookup results: " << std::endl;
			for (auto& tgt : fpar_in_out_tgts)
			{
				std::cout << tgt.field_name << " = "
					<< ((tgt.field_value) ? tgt.field_value : "not found")
					<< std::endl;
			}
			std::cout << "lookup time   : " << nanos.count()
				<< " nanos" << std::endl;
			std::cout << "lookup time   : " << micros.count()
				<< " micros" << std::endl;
			std::cout << "lookup time   : " << mills.count()
				<< " millis" << std::endl;
			std::cout << std::endl;
			
		}
		else if (lookup == EQR)
		{
			ro_string_db::field_pair& src = sources[i];
			std::vector<const char *>& tgts = targets[i];
			std::vector<ro_string_db::eq_range_result> eqr_in_out_tgts;
			for (uint k = 0, end = tgts.size(); k < end; ++k)
			{
				eqr_in_out_tgts.push_back(
					ro_string_db::eq_range_result(tgts[k])
				);
			}
			auto start = std::chrono::steady_clock::now();
			
			str_db.lookup_equal_range(src, eqr_in_out_tgts);
			
			auto end = std::chrono::steady_clock::now();
			auto nanos =
				std::chrono::duration_cast
					<std::chrono::nanoseconds>(end - start);
			
			auto micros =
				std::chrono::duration_cast
					<std::chrono::microseconds>(end - start);
			
			auto mills =
				std::chrono::duration_cast
					<std::chrono::milliseconds>(end - start);
			
			
			std::cout << "lookup type   : equal range" << std::endl;
			std::cout << "lookup source : "
				<< src.field_name << " " << src.field_value << std::endl;
			std::cout << "lookup results: " << std::endl;
			for (auto& eqr : eqr_in_out_tgts)
			{
				std::cout << eqr.field_name << " = ";
				if (eqr.values.size())
				{
					for (auto& val : eqr.values)
						std::cout << val << " ";
				}
				else
					std::cout << "not found";
				std::cout << std::endl;
			}
			
			std::cout << "lookup time   : " << nanos.count()
				<< " nanos" << std::endl;
			std::cout << "lookup time   : " << micros.count()
				<< " micros" << std::endl;
			std::cout << "lookup time   : " << mills.count()
				<< " millis" << std::endl;
			std::cout << std::endl;
		}
	}
}

int main(int argc, char * argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Use: %s <options>\n", program_name);
		print_try();
		exit(EXIT_FAILURE);
	}
	
	program_options opts;
	handle_optios(argc, argv, opts);
	
	try
	{process(opts);}
	catch (std::exception& e)
	{equit("%s", e.what());}
	
	return 0;
}
