set(CMAKE_MODULE_PATH .. .)

include(CMakeFunctionalUtils)

include(TestMacros)

# list_pop_front
set(input a b c d e f)
# - one entry
list_pop_front(input v)
assert_strequal(v "a")
assert_strequal(input "b;c;d;e;f")
# - multiple entries
list_pop_front(input v1 v2 v3)
assert_strequal(v1 "b")
assert_strequal(v2 "c")
assert_strequal(v3 "d")
assert_strequal(input "e;f")


# list_zip
set(list_a a b c)
set(list_b 0 1 2)
set(list_c x y z)
set(list_part f g)
# - simple
list_zip(zipped list_a list_b)
assert_strequal(zipped "a;0;b;1;c;2")
assert_strequal(list_a "a;b;c")
assert_strequal(list_b "0;1;2")
# - multiple lists
list_zip(zipped3 list_a list_b list_c)
assert_strequal(zipped3 "a;0;x;b;1;y;c;2;z")
# - stop after one of the lists is consumed
list_zip(zipped_part list_part list_b)
assert_strequal(zipped_part "f;0;g;1")
list_zip(zipped_part list_b list_part)
assert_strequal(zipped_part "0;f;1;g")


# list_unzip
list_unzip(zipped list_a1 list_b1)
assert_strequal(zipped "a;0;b;1;c;2")
assert_strequal(list_a1 "a;b;c")
assert_strequal(list_b1 "0;1;2")

list_unzip(zipped3 list_a1 list_b1 list_c1)
assert_strequal(list_a1 "a;b;c")
assert_strequal(list_b1 "0;1;2")
assert_strequal(list_c1 "x;y;z")
