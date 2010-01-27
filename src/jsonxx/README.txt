h1. Introduction

JSON++ is a light-weight JSON parser written in C++.

h2. Why another JSON parser?

Perhaps because web service clients are usually written in dynamic
languages these days, none of the existing C++ JSON parsers suite my
needs very well, so I wrote one to used in another project. My goals
for JSON++ were:
* Efficient in both memory and speed.
* No third party dependencies. JSON++ only depends on the standard C++ library.
* Cross platform.
* Robust.
* Small and convenient API. Most of the time, you only need to call one function and two function templates.
* Easy to integrate. JSON++ only has one source file and one header file. Just compile the source file and link with your program.

h3. Usage

The following snippet is from one of the unit tests. It's quite self-descriptive.

<pre>
<code>
string teststr(
        "{"
        "  \"foo\" : 1,"
        "  \"bar\" : false,"
        "  \"person\" : {\"name\" : \"GWB\", \"age\" : 60},"
        "  \"data\": [\"abcd\", 42]"
        "}"
               );
istringstream input(teststr);
Object o;
assert(o.parse(input));
assert(1 == o.get<long>("foo"));
assert(o.has<bool>("bar"));
assert(o.has<Object>("person"));
assert(o.get<Object>("person").has<long>("age"));
assert(o.has<Array>("data"));
assert(o.get<Array>("data").get<long>(1) == 42);
assert(o.get<Array>("data").get<string>(0) == "abcd");
assert(!o.has<long>("data"));
</code>
</pre>
