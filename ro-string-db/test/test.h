#ifndef TEST_H
#define TEST_H
/* v1.3 */

#include <stdio.h>
#include <stdbool.h>

//#define REPORT_VERBOSE
#define REPORT_FILE
//#define REPORT_PASSED

#ifdef REPORT_VERBOSE
#ifndef REPORT_FILE
#define REPORT_FILE
#endif

#ifndef REPORT_PASSED
#define REPORT_PASSED
#endif
#endif

#define fail_msg "check failed\n"
#define pass_msg "check passed\n"

#ifdef REPORT_FILE

#define base_msg "\n%s\n%s()\nline %d\n(%s)\n"

#define print_fail(expr)\
if (!(expr))\
return printf(base_msg fail_msg, __FILE__, __func__, __LINE__, #expr), false;

#define print_pass(expr)\
else printf(base_msg pass_msg, __FILE__, __func__, __LINE__, #expr);

#define report(pass, fail)\
do\
{\
puts(__FILE__);\
printf("Tests passed: %d\n", (pass));\
printf("Tests failed: %d\n", (fail));\
puts("------------------------------------------------");\
} while(0)

#else

#define base_msg "\n%s()\nline %d\n(%s)\n"

#define print_fail(expr)\
if (!(expr))\
return printf(base_msg fail_msg, __func__, __LINE__, #expr), false;

#define print_pass(expr)\
else printf(base_msg pass_msg, __func__, __LINE__, #expr);

#define report(pass, fail)\
do\
{\
printf("Tests passed: %d\n", (pass));\
printf("Tests failed: %d\n", (fail));\
} while(0)

#endif

#ifdef REPORT_PASSED
#define test(expr) print_fail(expr) print_pass(expr)
#else
#define test(expr) print_fail(expr)
#endif

#define check(expr) do {test(expr)} while(0)

typedef bool (*ftest)(void);
#endif
