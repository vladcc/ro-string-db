1. About
This projects aims to achieve minimal, associative csv file representation in
memory with adequate lookup times. The memory required is the size of the csv
file + 12 bytes for each string + the overhead of a few vectors. Lookup takes
logarithmic time. Results for multiple fields can be returned from a single
lookup. Cache locality favors rows over columns, since binary search isn't
very cache friendly anyway.



2. Build
In ro-string-db/build:
cmake ..

make ro_string_db_static - compile as a static library

make ro_string_db_shared - compile as a shared library

make query-driver - compiles the query driver; this is a test program which can
perform lookups from the command line, or from a file. You can find more info
in its help message and examples in the query_driver/ directory.

make all-tests - compiles all tests; to run them the test_data directory has to
be in the parent directory of the test binary. Either move the binary, or
symlink appropriately.

make help - see all make options



3. Overview
In order to not waste memory, all strings from the input file are placed in an
array of bytes, one after the other. A matrix of unsigned ints maps the
coordinates of the strings as found in the csv to their place in the string
pool. For each separate field there exists an array of structures of a pair of
unsigned ints, which maps each field value to its place in the string pool and
to its line in the csv. These arrays are sorted by the values of the strings
they point to. Finally, all sorted field arrays go into another sorted array,
which allows to lookup the field structures by name.

Example:
If the csv file looks like this:

id;fruit;type;price
1;pineapple;fancy;12.25
2;apple;normal;5.32
3;peach;normal;4.22
4;mango;fancy;10.50
5;pear;normal;6.00

(Silly, I know, but humor me)

then the string pool looks like:

{'i','d','\0','f','r','u','i','t','\0','t','y','p','e','\0','p','r','i','c','e','\0','1','\0','p','i','n','e','a','p','p','l','e','\0','f','a','n','c','y','\0','1','2','.','2','5','\0'... etc.}

So "id" begins at index 0
"fruit" at 3
"type"  at 9
"price" at 14
"1"     at 20
"pineapple" at 22
"fancy"     at 32
"12.25"     at 38 and so on.

This structure takes exactly the size of the file, given new lines are a single
character.

The matrix will look like:
    0    1    2    3
0   0    3    9    14 
1   20   22   32   38
2   44   46   52   59
3   64   66   72   79
4   84   86   92   98
5   104  106  111  118

Rows represent lines in the csv, columns fields, and m[row][col] is the string
pool index of the string found at line row in field col. This takes
row*col*sizeof(unsigned int) bytes of memory.

Each field array if an array of

struct num_field_info
{	
	uint original_line_number;
	uint index_of_string;
};

The array for field "id" will initially look like:
 "id"   "1"     "2"     "3"     "4"     "5"
{{0,0}, {1,20}, {2,44}, {3,64}, {4,84}, {5,104}}

The array for field "type" like:
 "type"  "fancy"  "normal" "normal" "fancy"  "normal"
{{0,9},  {1,32},  {2,52},  {3,72},  {4,92},  {5,111}}

and so on for the rest.

As mentioned, all of these get sorted by the strings they point to, so after
sorting the above will look like:
 "1"     "2"     "3"     "4"     "5"      "id"
{{1,20}, {2,44}, {3,64}, {4,84}, {5,104}, {0,0}}

 "fancy"  "fancy"  "normal" "normal" "normal"  "type"
{{1,32},  {4,92},  {2,52},  {3,72},  {5,111},  {0,9}}

or some other sorted variation there of, depending on the algorithm.

A lookup is performed by associating a source field with one or more target
fields. To find out which is the fruit with id 3 you do:

struct field_pair {
	const char * field_name;
	const char * field_value;
};

source.field_name = "id";
source.field_value = "3";
target.field_name = "fruit";
lookup(source, target);

If lookup() returns true, then target.field_value will point to the string
"peach". If lookup() returns false, it means there is no "id" "3" in the table.

The algorithm works as follows:
1. First source.field_name is looked for, which takes log(number of fields).

2. If found, then source.field_value is looked for in the source.field_name
array, which takes log(number of lines). At this point, the num_field_info for
the source.field_value is known and hence the number of its line, which is also
the row index in the matrix. Note that this step requires we are able to compare
a num_field_info to a const char *. This, along with order of indirection, is
the main point of the code complexity in this project.

3. target.field_name is looked up, which takes again log(number of fields). If
found, then the number of the field is known, which is also the col index in the
matrix.

4. The string pool is indexed at location
m[(line of source.field_value)][(number of target.field_name)], which is the
string for target.field_value.

More than one targets can be specified in a single lookup and each will cost
log(number of fields), however, log(number of lines) will be payed only once.

That's the story for a key field lookup. Strings in key fields must be unique.
What about fields which may have more than one of the same value, like the
"type" field? On these an equal range operation can be performed. In this case
step 2 of the algorithm takes log(number of lines)*2 because both the lower and
the upper bound for source.field_value must be found, instead of only the lower
bound. Also, an equal range returns a

struct eq_range_result {
	std::vector<const char *> values;
	const char * field_name;
};

for each target, instead of a field_pair.



4. Structure
input/ - a namespace of convenience functions. File reading,
string splitting, etc..

string_pool/ - a vector of bytes. All strings from the csv go there.

matrix/ - a generic 2d matrix implementation. Uses linear memory.

sort_vector/ - like an ordinary vector, but sorts itself and provides context
lookup.

ro_string_table/ - where most of the actual work takes place. By far the most
complicated part.

ro_string_db/ - the user facing part. You'd only need to create and interact
with this one.

query_driver/ - lookup driver program. Provides memory and time benchmarks. Can
perform lookups from the command line or from a file. The first is useful when
testing memory consumption, the second when testing lookup times, since multiple
lookups can be called in a loop. The memory report in the latter case may not be
accurate if the query file is too big, because the file is read in its entirety.
