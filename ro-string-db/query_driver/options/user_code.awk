# v1.1

function get_cname(name) {
	gsub("-", "_", name)
	return name
}
function get_long_cname(name) {
	return (get_cname(name) "_opt_long")
}
function get_short_cname(name) {
	return (get_cname(name) "_opt_short")
}

function gen_names(opt_num,    long_name, sanitized, ret) {
	long_name = get_long_name(opt_num)
	
	ret = sprintf("static const char %s = '%s';",
		get_short_cname(long_name), get_short_name(opt_num))
	ret = (ret "\n" sprintf("static const char %s[] = \"%s\";",
		get_long_cname(long_name),
		long_name)\
	)
	ret = (ret "\n")
	
	return ret
}

function END_CODE() {return "end_code"}

function get_code(    code_str) {
	while ((getline > 0) && (END_CODE() != $1))
		code_str = (code_str $0 "\n")
	return code_str
}

function RET_TYPE() {return "static void"}
function HANDLER_UNBOUND() {return "on_unbound_arg"}
function HANDLER_ERROR() {return "on_error"}

function gen_handler_defn(opt_num,
    title_cmnt, opt_declr, fname, sign, long_name, arg_t, code) {
    
	long_name = get_long_name(opt_num)
	arg_t = get_context_arg_type(1)
	name = get_cname(long_name)
	title_cmnt = sprintf("// %s", long_name)
	opt_declr = ""
	code = ""
	
	if (match(long_name, HANDLER_UNBOUND())) {
		code = get_unbound_arg_code(1)
		sign = "const char * arg, void * ctx"
	} else if (match(long_name, HANDLER_ERROR())) {
		code = get_on_error_code(1)
		sign = "opts_err_code err_code, const char * err_opt, void * ctx"
	} else {
		title_cmnt = sprintf("// --%s|-%s", long_name, get_short_name(opt_num))
		code = get_handler_code(opt_num)
		name = sprintf("handle_%s", name)
		sign = "const char * opt, char * opt_arg, void * ctx"
		opt_declr = gen_names(opt_num)
	}
	
	print_line(title_cmnt)	
	
	if (opt_declr)
		print_string(opt_declr)
		
	print_line(sprintf("%s %s(%s)", RET_TYPE(), name, sign))
		
	print_line("{")
	print_string(code)
	print_line("}")
	print_line()
}

function gen_help_defn(opt_num,    long_name) {
	long_name = get_long_name(opt_num)
	
	print_line(sprintf(\
		"%s help_%s(const char * short_name, const char * long_name)",\
		RET_TYPE(), get_cname(long_name))\
	)
	print_line("{")
	print_string(get_help_code(opt_num))
	print_line("}")
	print_line()
}

function gen_default_handlers() {
	save_long_name("on_unbound_arg")
	gen_handler_defn(get_long_name_count())
	
	save_long_name("on_error")
	gen_handler_defn(get_long_name_count())
}

function open_tbl() {
	print_line("opts_table the_tbl;")
	print_line("opts_entry all_entries[] = {")
	print_inc_indent()
}

function gen_tbl_entry(opt_num,    ctx, long_name, underscores, short_name) {
	long_name = get_long_name(opt_num)
	ctx = (match(long_name, "^help$")) ? "(&the_tbl)" : get_context_var_name(1)
	short_name = get_short_name(opt_num)
	
	underscores = get_cname(long_name)
	print_line("{")
	print_inc_indent()
	print_line(".names = {")
		print_inc_indent()
		print_line(sprintf(".long_name = %s,", get_long_cname(long_name)))
		print_line(sprintf(".short_name = %s", get_short_cname(long_name)))
		print_dec_indent()
	print_line("},")
	print_line(".handler = {")
		print_inc_indent()
		print_line(sprintf(".handler = handle_%s,", underscores))
		print_line(sprintf(".context = (void *)%s,", ctx))
		print_dec_indent()
	print_line("},")
	print_line(sprintf(".print_help = help_%s,", underscores))
	print_line(sprintf(".takes_arg = %s,", get_takes_args(opt_num)))
	print_dec_indent()
	print_line("},")
}

function close_tbl(    src) {
	print_dec_indent()
	print_line("};")
	print_line()
	print_line("the_tbl.tbl = all_entries;")
    print_line("the_tbl.length = sizeof(all_entries)/sizeof(*all_entries);")
    print_line()
}

function opts_parse_data() {
	print_line("opts_parse_data parse_data = {")
	print_inc_indent()
	print_line(".the_tbl = &the_tbl,")
	print_line(".on_unbound = {")
		print_inc_indent()
		print_line(sprintf(".handler = %s,", HANDLER_UNBOUND()))
		print_line(sprintf(".context = (void *)%s,", get_context_var_name(1)))
		print_dec_indent()
	print_line("},")
	print_line(".on_error = {")
		print_inc_indent()
		print_line(sprintf(".handler = %s,", HANDLER_ERROR()))
		print_line(sprintf(".context = (void *)%s,", get_context_var_name(1)))
		print_dec_indent()
	print_line("}")
	print_dec_indent()
	print_line("};")
}

function main(    i, end, opt) {
	end = get_long_name_count()
	
	print_line()
	for (i = 1; i <= end; ++i) {
		gen_handler_defn(i)
		gen_help_defn(i)
	}
	gen_default_handlers()
	
	open_tbl()
	for (i = 1; i <= end; ++i)
		gen_tbl_entry(i)
	close_tbl()
	
	opts_parse_data()
	
	print_line()
	print_line("opts_parse(argc-1, argv+1, &parse_data);")
}
