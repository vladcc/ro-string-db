#!/usr/bin/awk -f

@include "user_code.awk"

# <user_events>
function handle_context_arg_type() {
	data_or_err()
	save_context_arg_type($2)

}

function handle_context_var_name() {
	data_or_err()
	save_context_var_name($2)

}

function handle_unbound_arg_code() {
	#data_or_err()
	save_unbound_arg_code(get_code())

}

function handle_on_error_code() {
	#data_or_err()
	save_on_error_code(get_code())

}

function handle_long_name() {
	data_or_err()
	save_long_name($2)

}

function handle_short_name() {
	data_or_err()
	save_short_name($2)

}

function handle_takes_args() {
	data_or_err()
	save_takes_args($2)

}

function handle_handler_code() {
	#data_or_err()
	save_handler_code(get_code())

}

function handle_help_code() {
	#data_or_err()
	save_help_code(get_code())

}

function handle_end() {
	#data_or_err()
	#save_end($2)
}

function awk_BEGIN() {

}

function awk_END() {
	if (get_last_rule() != __R_END)
		in_error("last read rule not last declared")

	main()
}

function in_error(error_msg) {
	__error_raise(error_msg)
}
# </user_events>

# <print_lib>
function print_set_indent(tabs) {__base_indent__ = tabs}
function print_get_indent() {return __base_indent__}
function print_inc_indent() {print_set_indent(print_get_indent()+1)}
function print_dec_indent() {print_set_indent(print_get_indent()-1)}
function print_string(str, tabs) {print_tabs(tabs); printf("%s", str)}
function print_line(str, tabs) {print_string(str, tabs); print_new_lines(1)}
function print_tabs(tabs,    i, end) {
	end = tabs + print_get_indent()
	for (i = 1; i <= end; ++i)
		printf("\t")
}
function print_new_lines(new_lines,    i) {
	for (i = 1; i <= new_lines; ++i)
		printf("\n")
}
# </print_lib>

# <utils>
function data_or_err() {
	if (NF < 2)
		__error_raise(sprintf("no data after '%s'", $1))
}

function reset_all() {
	reset_context_arg_type()
	reset_context_var_name()
	reset_unbound_arg_code()
	reset_on_error_code()
	reset_long_name()
	reset_short_name()
	reset_takes_args()
	reset_handler_code()
	reset_help_code()
	reset_end()
}

function get_last_rule() {return __state}

function save_context_arg_type(context_arg_type) {__context_arg_type_arr__[++__context_arg_type_num__] = context_arg_type}
function get_context_arg_type_count() {return __context_arg_type_num__}
function get_context_arg_type(num) {return __context_arg_type_arr__[num]}
function reset_context_arg_type() {delete __context_arg_type_arr__; __context_arg_type_num__ = 0}

function save_context_var_name(context_var_name) {__context_var_name_arr__[++__context_var_name_num__] = context_var_name}
function get_context_var_name_count() {return __context_var_name_num__}
function get_context_var_name(num) {return __context_var_name_arr__[num]}
function reset_context_var_name() {delete __context_var_name_arr__; __context_var_name_num__ = 0}

function save_unbound_arg_code(unbound_arg_code) {__unbound_arg_code_arr__[++__unbound_arg_code_num__] = unbound_arg_code}
function get_unbound_arg_code_count() {return __unbound_arg_code_num__}
function get_unbound_arg_code(num) {return __unbound_arg_code_arr__[num]}
function reset_unbound_arg_code() {delete __unbound_arg_code_arr__; __unbound_arg_code_num__ = 0}

function save_on_error_code(on_error_code) {__on_error_code_arr__[++__on_error_code_num__] = on_error_code}
function get_on_error_code_count() {return __on_error_code_num__}
function get_on_error_code(num) {return __on_error_code_arr__[num]}
function reset_on_error_code() {delete __on_error_code_arr__; __on_error_code_num__ = 0}

function save_long_name(long_name) {__long_name_arr__[++__long_name_num__] = long_name}
function get_long_name_count() {return __long_name_num__}
function get_long_name(num) {return __long_name_arr__[num]}
function reset_long_name() {delete __long_name_arr__; __long_name_num__ = 0}

function save_short_name(short_name) {__short_name_arr__[++__short_name_num__] = short_name}
function get_short_name_count() {return __short_name_num__}
function get_short_name(num) {return __short_name_arr__[num]}
function reset_short_name() {delete __short_name_arr__; __short_name_num__ = 0}

function save_takes_args(takes_args) {__takes_args_arr__[++__takes_args_num__] = takes_args}
function get_takes_args_count() {return __takes_args_num__}
function get_takes_args(num) {return __takes_args_arr__[num]}
function reset_takes_args() {delete __takes_args_arr__; __takes_args_num__ = 0}

function save_handler_code(handler_code) {__handler_code_arr__[++__handler_code_num__] = handler_code}
function get_handler_code_count() {return __handler_code_num__}
function get_handler_code(num) {return __handler_code_arr__[num]}
function reset_handler_code() {delete __handler_code_arr__; __handler_code_num__ = 0}

function save_help_code(help_code) {__help_code_arr__[++__help_code_num__] = help_code}
function get_help_code_count() {return __help_code_num__}
function get_help_code(num) {return __help_code_arr__[num]}
function reset_help_code() {delete __help_code_arr__; __help_code_num__ = 0}

function save_end(end) {__end_arr__[++__end_num__] = end}
function get_end_count() {return __end_num__}
function get_end(num) {return __end_arr__[num]}
function reset_end() {delete __end_arr__; __end_num__ = 0}
# </utils>

#==============================================================================#
#                        machine generated parser below                        #
#==============================================================================#

# <state_machine>
function __error_raise(error_msg) {
	printf("error: %s, line %d: %s\n", FILENAME, FNR, error_msg) > "/dev/stderr"
	__error_happened = 1
	exit(1)
}
function __parse_error(expct, got) {
	__error_raise(sprintf("'%s' expected, but got '%s' instead", expct, got))
}
function __state_transition(__next) {
	if (__state == "") {
		if (__next == __R_CONTEXT_ARG_TYPE) __state = __next
		else __parse_error(__R_CONTEXT_ARG_TYPE, __next)
	}
	else if (__state == __R_CONTEXT_ARG_TYPE) {
		if (__next == __R_CONTEXT_VAR_NAME) __state = __next
		else __parse_error(__R_CONTEXT_VAR_NAME, __next)
	}
	else if (__state == __R_CONTEXT_VAR_NAME) {
		if (__next == __R_UNBOUND_ARG_CODE) __state = __next
		else __parse_error(__R_UNBOUND_ARG_CODE, __next)
	}
	else if (__state == __R_UNBOUND_ARG_CODE) {
		if (__next == __R_ON_ERROR_CODE) __state = __next
		else __parse_error(__R_ON_ERROR_CODE, __next)
	}
	else if (__state == __R_ON_ERROR_CODE) {
		if (__next == __R_LONG_NAME) __state = __next
		else __parse_error(__R_LONG_NAME, __next)
	}
	else if (__state == __R_LONG_NAME) {
		if (__next == __R_SHORT_NAME) __state = __next
		else __parse_error(__R_SHORT_NAME, __next)
	}
	else if (__state == __R_SHORT_NAME) {
		if (__next == __R_TAKES_ARGS) __state = __next
		else __parse_error(__R_TAKES_ARGS, __next)
	}
	else if (__state == __R_TAKES_ARGS) {
		if (__next == __R_HANDLER_CODE) __state = __next
		else __parse_error(__R_HANDLER_CODE, __next)
	}
	else if (__state == __R_HANDLER_CODE) {
		if (__next == __R_HELP_CODE) __state = __next
		else __parse_error(__R_HELP_CODE, __next)
	}
	else if (__state == __R_HELP_CODE) {
		if (__next == __R_END) __state = __next
		else __parse_error(__R_END, __next)
	}
	else if (__state == __R_END) {
		if (__next == __R_LONG_NAME) __state = __next
		else __parse_error(__R_LONG_NAME, __next)
	}
}
# </state_machine>

# <input>
$1 == __R_CONTEXT_ARG_TYPE {__state_transition($1); handle_context_arg_type(); next}
$1 == __R_CONTEXT_VAR_NAME {__state_transition($1); handle_context_var_name(); next}
$1 == __R_UNBOUND_ARG_CODE {__state_transition($1); handle_unbound_arg_code(); next}
$1 == __R_ON_ERROR_CODE {__state_transition($1); handle_on_error_code(); next}
$1 == __R_LONG_NAME {__state_transition($1); handle_long_name(); next}
$1 == __R_SHORT_NAME {__state_transition($1); handle_short_name(); next}
$1 == __R_TAKES_ARGS {__state_transition($1); handle_takes_args(); next}
$1 == __R_HANDLER_CODE {__state_transition($1); handle_handler_code(); next}
$1 == __R_HELP_CODE {__state_transition($1); handle_help_code(); next}
$1 == __R_END {__state_transition($1); handle_end(); next}
$0 ~ /^[[:space:]]*$/ {next} # ignore empty lines
$0 ~ /^[[:space:]]*#/ {next} # ignore comments
{__error_raise("'" $1 "' unknown")} # all else is error
# </input>

# <start>
BEGIN {
	__R_CONTEXT_ARG_TYPE = "context_arg_type"
	__R_CONTEXT_VAR_NAME = "context_var_name"
	__R_UNBOUND_ARG_CODE = "unbound_arg_code"
	__R_ON_ERROR_CODE = "on_error_code"
	__R_LONG_NAME = "long_name"
	__R_SHORT_NAME = "short_name"
	__R_TAKES_ARGS = "takes_args"
	__R_HANDLER_CODE = "handler_code"
	__R_HELP_CODE = "help_code"
	__R_END = "end"
	__error_happened = 0
	awk_BEGIN()
}
# </start>

# <end>
END {
	if (!__error_happened)
		awk_END()
}
# </end>

# <user_source>
# context_arg_type -> context_var_name
# context_var_name -> unbound_arg_code
# unbound_arg_code -> on_error_code
# on_error_code -> long_name
# long_name -> short_name
# short_name -> takes_args
# takes_args -> handler_code
# handler_code -> help_code
# help_code -> end
# end -> long_name
# </user_source>

# generated by scriptscript v2.11
