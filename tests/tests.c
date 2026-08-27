/*
 SPDX-License-Identifier: MIT

 Parson (https://github.com/kgabis/parson)
 Copyright (c) 2012 - 2023 Krzysztof Gabis

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/
/**
 * \\file tests.c
 * \\brief Test suite for parson JSON library
 * \\author Krzysztof Gabis
 */

#ifdef _MSC_VER
/** \\brief Suppress MSVC warnings */
#define _CRT_SECURE_NO_WARNINGS
#endif

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

/* clang-format off */
#ifdef PARSON_SINGLE_HEADER
/** \\brief Implement parson inside single header */
#define PARSON_IMPLEMENTATION
#endif

#define PARSON_FORCE_HASH_COLLISIONS 1

#include <stdio.h>
/** \brief failing_ftell */
long failing_ftell(FILE *stream);
/** \brief failing_fseek */
int failing_fseek(FILE *stream, long offset, int whence);
/** \brief failing_fread */
size_t failing_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
/** \brief failing_ferror */
int failing_ferror(FILE *stream);

/** \\brief Global variable declaration for g_failing_file */
/** \\brief Global flag to simulate failing IO */
extern int g_failing_file;

/** \\brief Override fseek */
#define fseek failing_fseek
/** \\brief Override ftell */
#define ftell failing_ftell
/** \\brief Override fread */
#define fread failing_fread
/** \\brief Override ferror */
#undef ferror
#define ferror failing_ferror

#include "../parson.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "add_tests.c"
/* clang-format on */

/** \\brief Test assertion macro */
extern int g_parson_test_zero;
#define TEST(A)                                                                \
  do {                                                                         \
    if (A) {                                                                   \
      g_tests_passed++;                                                        \
    } else {                                                                   \
      printf("%d %-72s - FAILED\n", __LINE__, #A);                             \
      g_tests_failed++;                                                        \
    }                                                                          \
  } while (g_parson_test_zero)

/** \\brief Macro to check string equality */
#define STREQ(A, B) ((A) && (B) ? strcmp((A), (B)) == 0 : 0)

#define TEST_SERIALIZATION(val)                                                \
  do {                                                                         \
    char *__ser = json_serialize_to_string(val);                               \
    JSON_Value *__par = json_parse_string(__ser);                              \
    TEST(json_value_equals(__par, (val)));                                     \
    json_free_serialized_string(__ser);                                        \
    json_value_free(__par);                                                    \
  } while (g_parson_test_zero)

#define TEST_SERIALIZATION_PRETTY(val)                                         \
  do {                                                                         \
    char *__ser = json_serialize_to_string_pretty(val);                        \
    JSON_Value *__par = json_parse_string(__ser);                              \
    TEST(json_value_equals(__par, (val)));                                     \
    json_free_serialized_string(__ser);                                        \
    json_value_free(__par);                                                    \
  } while (g_parson_test_zero)

/** \\brief Double precision epsilon */
#define DBL_EPSILON 2.2204460492503131e-16
/** \\brief Check if two doubles are equal */
#define DBL_EQ(a, b) (fabs((a) - (b)) < DBL_EPSILON)

/** \brief test_suite_1 */
void test_suite_1(void); /* Test 3 files from json.org + serialization*/
/** \\brief Forward declaration for test_suite_2 */
void test_suite_2(JSON_Value *value); /* Test correctness of parsed values */
/** \brief test_suite_2_no_comments */
void test_suite_2_no_comments(void);
/** \brief test_suite_2_with_comments */
void test_suite_2_with_comments(void);
/** \brief test_suite_3 */
void test_suite_3(void); /* Test parsing valid and invalid strings */
/** \\brief Forward declaration for test_suite_2 */
void test_suite_4(void); /* Test deep copy function */
/** \brief test_suite_5 */
void test_suite_5(void); /* Test building json values from scratch */
/** \brief test_suite_6 */
void test_suite_6(void); /* Test value comparing verification */
/** \brief test_suite_7 */
void test_suite_7(void); /* Test schema validation */
/** \brief test_suite_8 */
void test_suite_8(void); /* Test serialization */
/** \brief test_suite_9 */
void test_suite_9(void); /* Test serialization (pretty) */
/** \brief test_suite_10 */
void test_suite_10(void); /* Testing for memory leaks */
/** \brief test_suite_11 */
void test_suite_11(void);
/** \brief test_coverage_gap */
void test_coverage_gap(void);
/** \brief test_file_parsing_failures */
void test_file_parsing_failures(
    void); /* Additional things that require testing */
/** \brief test_memory_leaks */
void test_memory_leaks(void);
/** \brief test_failing_allocations */
void test_failing_allocations(void);
/** \brief test_failing_allocations_for_all_apis */
void test_failing_allocations_for_all_apis(void);
/** \brief test_custom_number_format */
void test_custom_number_format(void);
/** \brief test_custom_number_serialization_function */
void test_custom_number_serialization_function(void);
/** \brief test_object_clear */
void test_object_clear(void);

/** \brief print_commits_info */
void print_commits_info(const char *username, const char *repo);
/** \brief persistence_example */
void persistence_example(void);
/** \brief serialization_example */
void serialization_example(void);

/** \brief Path to the tests directory */
static const char *g_tests_path = "tests";

/** \brief Global malloc count tracking */
static int g_malloc_count = 0;
/** \brief counted_malloc */
/** \brief Forward declaration for counted_malloc */
static void *counted_malloc(size_t size);
/** \brief counted_free */
static /** \\brief Forward declaration for test_suite_2 */
    void
    counted_free(void *ptr);

/** \brief Tracker for failing allocations */
typedef struct failing_alloc {
  int allocation_to_fail;
  int alloc_count;
  int total_count;
  int has_failed;
  int should_fail;
} failing_alloc_t;

/** \brief Global failing allocator tracking struct */
static failing_alloc_t g_failing_alloc;

/** \brief failing_malloc */
/** \brief Forward declaration for failing_malloc */
static void *failing_malloc(size_t size);
/** \brief failing_free */
static /** \\brief Forward declaration for test_suite_2 */
    void
    failing_free(void *ptr);

/** \brief test_read_file */
/** \brief Forward declaration for test_read_file */
static JSON_Status test_read_file(const char *filename, char **out_contents);
/** \\brief Forward declaration for get_file_path */
const char *get_file_path(const char *filename);

/** \brief Counter for passed tests */
int g_tests_passed;
/** \brief Counter for failed tests */
int g_tests_failed;
/** \brief Dummy variable to bypass C4127 */
int g_parson_test_zero = 0;

/**
 * \\brief test_coverage_gap8
 */
void test_coverage_gap8(void) {
  JSON_Value *val1;
  JSON_Value *val2;

  /* Validate different types */
  val1 = json_parse_string("{\"a\":1}");
  val2 = json_parse_string("[1]");
  json_validate(val1, val2);
  json_value_free(val1);
  json_value_free(val2);

  /* Validate array lengths */
  val1 = json_parse_string("[1, 2]");
  val2 = json_parse_string("[1]");
  json_validate(val1, val2);
  json_value_free(val1);
  json_value_free(val2);

  /* equals different types */
  val1 = json_parse_string("1");
  val2 = json_parse_string("[1]");
  json_value_equals(val1, val2);
  json_value_free(val1);
  json_value_free(val2);

  /* equals arrays diff length */
  val1 = json_parse_string("[1, 2]");
  val2 = json_parse_string("[1]");
  json_value_equals(val1, val2);
  json_value_free(val1);
  json_value_free(val2);

  /* equals objects diff length */
  val1 = json_parse_string("{\"a\":1, \"b\":2}");
  val2 = json_parse_string("{\"a\":1}");
  json_value_equals(val1, val2);
  json_value_free(val1);
  json_value_free(val2);

  /* type casting missing */
  val1 = json_parse_string("1");
  json_type(val1);
  json_object(val1);
  json_array(val1);
  json_string(val1);
  json_string_len(val1);
  json_number(val1);
  json_boolean(val1);
  json_value_free(val1);

  /* config setters */
  json_set_escape_slashes(0);
  json_set_float_serialization_format("%.2f");
  json_set_number_serialization_function(NULL);
  json_set_float_serialization_format(NULL);
}

/**
 * \\brief test_coverage_gap
 */
void test_coverage_gap(void) {
  JSON_Value *val1, *val2;

  val1 = json_parse_string("[]");
  val2 = json_parse_string("[1, \"two\", true]");
  json_validate(val1, val2);
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("[1]");
  val2 = json_parse_string("[\"two\"]");
  json_validate(val1, val2);
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("{\"a\":1, \"b\":2}");
  val2 = json_parse_string("{\"a\":1}");
  json_validate(val1, val2);
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("{\"a\":1, \"b\":2}");
  val2 = json_parse_string("{\"a\":1, \"c\":3}");
  json_validate(val1, val2);
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("true");
  val2 = json_parse_string("false");
  json_value_equals(val1, val2);
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("{\"a\":1}");
  val2 = json_parse_string("{\"b\":1}");
  json_value_equals(val1, val2);
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("{\"a\":1}");
  val2 = json_parse_string("{\"a\":1, \"b\":2}");
  json_value_equals(val1, val2);
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("1");
  json_type(val1);
  json_object(val1);
  json_array(val1);
  json_string(val1);
  json_string_len(val1);
  json_number(val1);
  json_boolean(val1);
  json_value_free(val1);

  json_set_escape_slashes(0);
  json_set_float_serialization_format("%.2f");
  json_set_number_serialization_function(NULL);
  json_set_float_serialization_format(NULL);
}

#ifdef TESTS_MAIN
/**
 * \\brief main
 * \\param argc Parameter argc
 * \\param argv Parameter argv
 */
int main(int argc, char *argv[]) {
#else
/** \brief tests_main */
int tests_main(int argc, char *argv[]);

/**
 * \\brief tests_main
 * \\param argc Parameter argc
 * \\param argv Parameter argv
 */
int tests_main(int argc, char *argv[]) {
#endif
#if 0 /* unconfuse xcode */
}
#endif
  /* Example functions from readme file:      */
  /* print_commits_info("torvalds", "linux"); */
  /* serialization_example(); */
  /* persistence_example(); */

  puts("#######################################################################"
       "#########");
  puts("Running parson tests");
  setvbuf(stdout, NULL, _IONBF, 0);

  if (argc == 2) {
    g_tests_path = argv[1];
  } else {
    g_tests_path = "tests";
  }

  json_set_allocation_functions(counted_malloc, counted_free);
  test_suite_1();
  test_suite_2_no_comments();
  test_suite_2_with_comments();
  test_suite_3();
  test_suite_4();
  test_suite_5();
  test_suite_6();
  test_suite_7();
  test_suite_8();
  test_suite_9();
  test_suite_10();
  test_suite_11();
  test_coverage_gap();
  test_coverage_gap8();
  test_memory_leaks();
  test_file_parsing_failures();
  test_json_error_coverage();
  test_failing_allocations_for_all_apis();
  test_failing_allocations();
  test_custom_number_format();
  test_custom_number_serialization_function();
  test_object_clear();

  printf("Tests failed: %d\n", g_tests_failed);
  printf("Tests passed: %d\n", g_tests_passed);
  puts("#######################################################################"
       "#########");
  return 0;
}

/**
 * \\brief test_suite_1
 */
void test_suite_1(void) {
  JSON_Value *val;
  TEST((val = json_parse_file(get_file_path("test_1_1.txt"))) != NULL);
  TEST_SERIALIZATION(val);
  TEST_SERIALIZATION_PRETTY(val);
  if (val) {
    json_value_free(val);
  }

  TEST((val = json_parse_file(get_file_path("test_1_2.txt"))) ==
       NULL); /* Over 2048 levels of nesting */
  if (val) {
    json_value_free(val);
  }

  TEST((val = json_parse_file(get_file_path("test_1_3.txt"))) != NULL);
  TEST_SERIALIZATION(val);
  TEST_SERIALIZATION_PRETTY(val);
  if (val) {
    json_value_free(val);
  }

  TEST((val = json_parse_file_with_comments(get_file_path("test_1_1.txt"))) !=
       NULL);
  TEST_SERIALIZATION(val);
  TEST_SERIALIZATION_PRETTY(val);
  if (val) {
    json_value_free(val);
  }

  TEST((val = json_parse_file_with_comments(get_file_path("test_1_2.txt"))) ==
       NULL); /* Over 2048 levels of nesting */
  if (val) {
    json_value_free(val);
  }

  TEST((val = json_parse_file_with_comments(get_file_path("test_1_3.txt"))) !=
       NULL);
  TEST_SERIALIZATION(val);
  TEST_SERIALIZATION_PRETTY(val);
  if (val) {
    json_value_free(val);
  }
}

/**
 * \\brief test_suite_2
 * \\param root_value Parameter root_value
 */
void test_suite_2(JSON_Value *root_value) {
  JSON_Object *root_object;
  JSON_Array *array;
  JSON_Value *array_value;
  size_t len;
  size_t i;
  TEST(root_value);
  TEST(json_value_get_type(root_value) == JSONObject);
  root_object = json_value_get_object(root_value);

  TEST(json_object_has_value(root_object, "string"));
  TEST(!json_object_has_value(root_object, "_string"));
  TEST(json_object_has_value_of_type(root_object, "object", JSONObject));
  TEST(!json_object_has_value_of_type(root_object, "string array", JSONObject));
  TEST(json_object_has_value_of_type(root_object, "string array", JSONArray));
  TEST(!json_object_has_value_of_type(root_object, "object", JSONArray));
  TEST(json_object_has_value_of_type(root_object, "string", JSONString));
  TEST(!json_object_has_value_of_type(root_object, "positive one", JSONString));
  TEST(json_object_has_value_of_type(root_object, "positive one", JSONNumber));
  TEST(!json_object_has_value_of_type(root_object, "string", JSONNumber));
  TEST(json_object_has_value_of_type(root_object, "boolean true", JSONBoolean));
  TEST(
      !json_object_has_value_of_type(root_object, "positive one", JSONBoolean));
  TEST(json_object_has_value_of_type(root_object, "null", JSONNull));
  TEST(!json_object_has_value_of_type(root_object, "object", JSONNull));

  TEST(json_object_dothas_value(root_object, "object.nested array"));
  TEST(!json_object_dothas_value(root_object, "_object.nested array"));
  TEST(json_object_dothas_value_of_type(root_object, "object.nested object",
                                        JSONObject));
  TEST(!json_object_dothas_value_of_type(root_object, "object.nested array",
                                         JSONObject));
  TEST(json_object_dothas_value_of_type(root_object, "object.nested array",
                                        JSONArray));
  TEST(!json_object_dothas_value_of_type(root_object, "object.nested object",
                                         JSONArray));
  TEST(json_object_dothas_value_of_type(root_object, "object.nested string",
                                        JSONString));
  TEST(!json_object_dothas_value_of_type(root_object, "object.nested number",
                                         JSONString));
  TEST(json_object_dothas_value_of_type(root_object, "object.nested number",
                                        JSONNumber));
  TEST(!json_object_dothas_value_of_type(root_object, "_object.nested whatever",
                                         JSONNumber));
  TEST(json_object_dothas_value_of_type(root_object, "object.nested true",
                                        JSONBoolean));
  TEST(!json_object_dothas_value_of_type(root_object, "object.nested number",
                                         JSONBoolean));
  TEST(json_object_dothas_value_of_type(root_object, "object.nested null",
                                        JSONNull));
  TEST(!json_object_dothas_value_of_type(root_object, "object.nested object",
                                         JSONNull));

  TEST(STREQ(json_object_get_string(root_object, "string"), "lorem ipsum"));
  TEST(STREQ(json_object_get_string(root_object, "utf string"), "lorem ipsum"));
  TEST(
      STREQ(json_object_get_string(root_object, "utf-8 string"), "あいうえお"));
  TEST(STREQ(json_object_get_string(root_object, "surrogate string"),
             "lorem𝄞ipsum𝍧lorem"));

  len = json_object_get_string_len(root_object, "string with null");
  TEST(len == 7);
  TEST(memcmp(json_object_get_string(root_object, "string with null"),
              "abc\0def", len) == 0);

  TEST(DBL_EQ(json_object_get_number(root_object, "positive one"), 1.0));
  TEST(DBL_EQ(json_object_get_number(root_object, "negative one"), -1.0));
  TEST(DBL_EQ(json_object_get_number(root_object, "hard to parse number"),
              -0.000314));
  TEST(json_object_get_boolean(root_object, "boolean true") == 1);
  TEST(json_object_get_boolean(root_object, "boolean false") == 0);
  TEST(json_value_get_type(json_object_get_value(root_object, "null")) ==
       JSONNull);

  array = json_object_get_array(root_object, "string array");
  if (array != NULL && json_array_get_count(array) > 1) {
    TEST(STREQ(json_array_get_string(array, 0), "lorem"));
    TEST(STREQ(json_array_get_string(array, 1), "ipsum"));
  } else {
    g_tests_failed++;
  }

  array = json_object_get_array(root_object, "x^2 array");
  if (array != NULL) {
    for (i = 0; i < json_array_get_count(array); i++) {
      TEST(DBL_EQ(json_array_get_number(array, i), (i * i)));
    }
  } else {
    g_tests_failed++;
  }

  TEST(json_object_get_array(root_object, "non existent array") == NULL);
  TEST(STREQ(json_object_dotget_string(root_object, "object.nested string"),
             "str"));
  TEST(json_object_dotget_boolean(root_object, "object.nested true") == 1);
  TEST(json_object_dotget_boolean(root_object, "object.nested false") == 0);
  TEST(json_object_dotget_value(root_object, "object.nested null") != NULL);
  TEST(DBL_EQ(json_object_dotget_number(root_object, "object.nested number"),
              123));

  TEST(json_object_dotget_value(root_object, "should.be.null") == NULL);
  TEST(json_object_dotget_value(root_object, "should.be.null.") == NULL);
  TEST(json_object_dotget_value(root_object, ".") == NULL);
  TEST(json_object_dotget_value(root_object, "") == NULL);

  array = json_object_dotget_array(root_object, "object.nested array");
  TEST(array != NULL);
  TEST(json_array_get_count(array) > 1);
  if (array != NULL && json_array_get_count(array) > 1) {
    TEST(STREQ(json_array_get_string(array, 0), "lorem"));
    TEST(STREQ(json_array_get_string(array, 1), "ipsum"));
  }
  TEST(json_object_dotget_boolean(root_object, "object.nested true") == 1);

  TEST(STREQ(json_object_get_string(root_object, "/**/"), "comment"));
  TEST(STREQ(json_object_get_string(root_object, "//"), "comment"));
  TEST(STREQ(json_object_get_string(root_object, "url"),
             "https://www.example.com/search?q=12345"));
  TEST(STREQ(json_object_get_string(root_object, "escaped chars"), "\" \\ /"));

  TEST(json_object_get_object(root_object, "empty object") != NULL);
  TEST(json_object_get_array(root_object, "empty array") != NULL);

  TEST(json_object_get_wrapping_value(root_object) == root_value);
  array = json_object_get_array(root_object, "string array");
  array_value = json_object_get_value(root_object, "string array");
  TEST(json_array_get_wrapping_value(array) == array_value);
  TEST(json_value_get_parent(array_value) == root_value);
  TEST(json_value_get_parent(root_value) == NULL);
}

/**
 * \\brief test_suite_2_no_comments
 */
void test_suite_2_no_comments(void) {
  const char *filename = "test_2.txt";
  JSON_Value *root_value = NULL;
  root_value = json_parse_file(get_file_path(filename));
  test_suite_2(root_value);
  TEST_SERIALIZATION(root_value);
  TEST_SERIALIZATION_PRETTY(root_value);
  json_value_free(root_value);
}

/**
 * \\brief test_suite_2_with_comments
 */
void test_suite_2_with_comments(void) {
  const char *filename = "test_2_comments.txt";
  JSON_Value *root_value = NULL;
  root_value = json_parse_file_with_comments(get_file_path(filename));
  test_suite_2(root_value);
  TEST_SERIALIZATION(root_value);
  TEST_SERIALIZATION_PRETTY(root_value);
  json_value_free(root_value);
}

/**
 * \\brief test_suite_3
 */
void test_suite_3(void) {
  /* Testing valid strings */

  {
    JSON_Value *__tmp = json_parse_string("{\"lorem\":\"ipsum\"}");
    TEST(__tmp != NULL);
    json_value_free(__tmp);
  }

  {
    JSON_Value *__tmp = json_parse_string("[\"lorem\"]");
    TEST(__tmp != NULL);
    json_value_free(__tmp);
  }

  {
    JSON_Value *__tmp = json_parse_string("null");
    TEST(__tmp != NULL);
    json_value_free(__tmp);
  }

  {
    JSON_Value *__tmp = json_parse_string("true");
    TEST(__tmp != NULL);
    json_value_free(__tmp);
  }

  {
    JSON_Value *__tmp = json_parse_string("false");
    TEST(__tmp != NULL);
    json_value_free(__tmp);
  }

  {
    JSON_Value *__tmp = json_parse_string("\"string\"");
    TEST(__tmp != NULL);
    json_value_free(__tmp);
  }

  {
    JSON_Value *__tmp = json_parse_string("123");
    TEST(__tmp != NULL);
    json_value_free(__tmp);
  }

  {
    JSON_Value *__tmp = json_parse_string("[\"lorem\",]");
    TEST(__tmp != NULL);
    json_value_free(__tmp);
  }

  {
    JSON_Value *__tmp = json_parse_string("{\"lorem\":\"ipsum\",}");
    TEST(__tmp != NULL);
    json_value_free(__tmp);
  }

  /* Test UTF-16 parsing */
  {
    JSON_Value *__tmp = json_parse_string("\"\\u0024x\"");
    TEST(STREQ(json_string(__tmp), "$x"));
    json_value_free(__tmp);
  }
  {
    JSON_Value *__tmp = json_parse_string("\"\\u00A2x\"");
    TEST(STREQ(json_string(__tmp), "¢x"));
    json_value_free(__tmp);
  }
  {
    JSON_Value *__tmp = json_parse_string("\"\\u20ACx\"");
    TEST(STREQ(json_string(__tmp), "€x"));
    json_value_free(__tmp);
  }
  {
    JSON_Value *__tmp = json_parse_string("\"\\uD801\\uDC37x\"");
    TEST(STREQ(json_string(__tmp), "𐐷x"));
    json_value_free(__tmp);
  }

  /* Testing invalid strings */
  g_malloc_count = 0;
  TEST(json_parse_string(NULL) == NULL);
  TEST(json_parse_string("") == NULL); /* empty string */
  TEST(json_parse_string("{lorem:ipsum}") == NULL);
  TEST(json_parse_string("{\"lorem\":\"ipsum\",]") == NULL);
  TEST(json_parse_string("{\"lorem\":\"ipsum\",,}") == NULL);
  TEST(json_parse_string("[,]") == NULL);
  TEST(json_parse_string("[,") == NULL);
  TEST(json_parse_string("[") == NULL);
  TEST(json_parse_string("]") == NULL);
  TEST(json_parse_string("{\"a\":0,\"a\":0}") == NULL); /* duplicate keys */
  TEST(json_parse_string("{:,}") == NULL);
  TEST(json_parse_string("{,}") == NULL);
  TEST(json_parse_string("{,") == NULL);
  TEST(json_parse_string("{:") == NULL);
  TEST(json_parse_string("{") == NULL);
  TEST(json_parse_string("}") == NULL);
  TEST(json_parse_string("x") == NULL);
  TEST(json_parse_string("{:\"no name\"}") == NULL);
  TEST(json_parse_string("[,\"no first value\"]") == NULL);
  TEST(json_parse_string("{\"key\"\"value\"}") == NULL);
  TEST(json_parse_string("{\"a\"}") == NULL);
  TEST(json_parse_string("[\"\\u00zz\"]") == NULL); /* invalid utf value */
  TEST(json_parse_string("[\"\\u00\"]") == NULL);   /* invalid utf value */
  TEST(json_parse_string("[\"\\u\"]") == NULL);     /* invalid utf value */
  TEST(json_parse_string("[\"\\\"]") == NULL);      /* control character */
  TEST(json_parse_string("[\"\"\"]") == NULL);      /* control character */
  TEST(json_parse_string("[\"\0\"]") == NULL);      /* control character */
  TEST(json_parse_string("[\"\a\"]") == NULL);      /* control character */
  TEST(json_parse_string("[\"\b\"]") == NULL);      /* control character */
  TEST(json_parse_string("[\"\t\"]") == NULL);      /* control character */
  TEST(json_parse_string("[\"\n\"]") == NULL);      /* control character */
  TEST(json_parse_string("[\"\f\"]") == NULL);      /* control character */
  TEST(json_parse_string("[\"\r\"]") == NULL);      /* control character */
  TEST(json_parse_string("[0x2]") == NULL);         /* hex */
  TEST(json_parse_string("[0X2]") == NULL);         /* HEX */
  TEST(json_parse_string("[07]") == NULL);          /* octals */
  TEST(json_parse_string("[0070]") == NULL);
  TEST(json_parse_string("[07.0]") == NULL);
  TEST(json_parse_string("[-07]") == NULL);
  TEST(json_parse_string("[-007]") == NULL);
  TEST(json_parse_string("[-07.0]") == NULL);
  TEST(json_parse_string("[\"\\uDF67\\uD834\"]") ==
       NULL); /* wrong order surrogate pair */
  TEST(json_parse_string("[1.7976931348623157e309]") == NULL);
  TEST(json_parse_string("[-1.7976931348623157e309]") == NULL);
  TEST(g_malloc_count == 0);
}

/**
 * \\brief test_suite_4
 */
void test_suite_4(void) {
  const char *filename = "test_2.txt";
  JSON_Value *a = NULL, *a_copy = NULL;
  a = json_parse_file(get_file_path(filename));
  TEST(json_value_equals(a, a)); /* test equality test */
  a_copy = json_value_deep_copy(a);
  TEST(a_copy != NULL);
  TEST(json_value_equals(a, a_copy));
  json_value_free(a);
  json_value_free(a_copy);
}

/**
 * \\brief test_suite_5
 */
void test_suite_5(void) {
  double zero = 0.0; /* msvc is silly (workaround for error C2124) */

  JSON_Value *val_from_file = json_parse_file(get_file_path("test_5.txt"));

  JSON_Value *val = NULL, *val_with_parent;
  JSON_Object *obj = NULL;
  JSON_Array *interests_arr = NULL;

  JSON_Value *remove_test_val = NULL;
  JSON_Array *remove_test_arr = NULL;

  val = json_value_init_object();
  TEST(val != NULL);

  obj = json_value_get_object(val);
  TEST(obj != NULL);

  TEST(json_object_set_string(obj, "first", "John") == JSONSuccess);
  TEST(json_object_set_string(obj, "last", "Doe") == JSONSuccess);
  TEST(json_object_set_number(obj, "age", 25) == JSONSuccess);
  TEST(json_object_set_boolean(obj, "registered", 1) == JSONSuccess);

  TEST(json_object_set_value(obj, "interests", json_value_init_array()) ==
       JSONSuccess);
  interests_arr = json_object_get_array(obj, "interests");
  TEST(interests_arr != NULL);
  TEST(json_array_append_string(interests_arr, "Writing") == JSONSuccess);
  TEST(json_array_append_string(interests_arr, "Mountain Biking") ==
       JSONSuccess);
  TEST(json_array_replace_string(interests_arr, 0, "Reading") == JSONSuccess);

  TEST(json_object_dotset_string(obj, "favorites.color", "blue") ==
       JSONSuccess);
  TEST(json_object_dotset_string(obj, "favorites.sport", "running") ==
       JSONSuccess);
  TEST(json_object_dotset_string(obj, "favorites.fruit", "apple") ==
       JSONSuccess);
  TEST(json_object_dotremove(obj, "favorites.fruit") == JSONSuccess);
  TEST(json_object_set_string(obj, "utf string", "lorem ipsum") == JSONSuccess);
  TEST(json_object_set_string(obj, "utf-8 string", "あいうえお") ==
       JSONSuccess);
  TEST(json_object_set_string(obj, "surrogate string", "lorem𝄞ipsum𝍧lorem") ==
       JSONSuccess);
  TEST(json_object_set_string_with_len(obj, "string with null", "abc\0def",
                                       7) == JSONSuccess);
  TEST(json_object_set_string(obj, "windows path", "C:\\Windows\\Path") ==
       JSONSuccess);
  TEST(json_value_equals(val_from_file, val));

  TEST(json_object_set_string(obj, NULL, "") == JSONFailure);
  TEST(json_object_set_string(obj, "last", NULL) == JSONFailure);
  TEST(json_object_set_string(obj, NULL, NULL) == JSONFailure);
  TEST(json_object_set_value(obj, NULL, NULL) == JSONFailure);

  TEST(json_object_dotset_string(obj, NULL, "") == JSONFailure);
  TEST(json_object_dotset_string(obj, "favorites.color", NULL) == JSONFailure);
  TEST(json_object_dotset_string(obj, NULL, NULL) == JSONFailure);
  TEST(json_object_dotset_value(obj, NULL, NULL) == JSONFailure);

  TEST(json_array_append_string(NULL, "lorem") == JSONFailure);
  TEST(json_array_append_value(interests_arr, NULL) == JSONFailure);
  TEST(json_array_append_value(NULL, NULL) == JSONFailure);

  TEST(json_array_remove(NULL, 0) == JSONFailure);
  TEST(json_array_replace_value(interests_arr, 0, NULL) == JSONFailure);
  TEST(json_array_replace_string(NULL, 0, "lorem") == JSONFailure);
  TEST(json_array_replace_string(interests_arr, 100, "not existing") ==
       JSONFailure);

  TEST(json_array_append_string(json_object_get_array(obj, "interests"),
                                NULL) == JSONFailure);

  TEST(json_array_append_string(interests_arr, "Writing") == JSONSuccess);
  TEST(json_array_remove(interests_arr, 0) == JSONSuccess);
  TEST(json_array_remove(interests_arr, 1) == JSONSuccess);
  TEST(json_array_remove(interests_arr, 0) == JSONSuccess);
  TEST(json_array_remove(interests_arr, 0) ==
       JSONFailure); /* should be empty by now */

  val_with_parent = json_value_init_null();
  TEST(json_object_set_value(obj, "x", val_with_parent) == JSONSuccess);
  TEST(json_object_set_value(obj, "x", val_with_parent) == JSONFailure);

  val_with_parent = json_value_init_null();
  TEST(json_array_append_value(interests_arr, val_with_parent) == JSONSuccess);
  TEST(json_array_append_value(interests_arr, val_with_parent) == JSONFailure);

  val_with_parent = json_value_init_null();
  TEST(json_array_replace_value(interests_arr, 0, val_with_parent) ==
       JSONSuccess);
  TEST(json_array_replace_value(interests_arr, 0, val_with_parent) ==
       JSONFailure);

  TEST(json_object_remove(obj, "interests") == JSONSuccess);

  /* UTF-8 tests */
  TEST(json_object_set_string(obj, "correct string", "κόσμε") == JSONSuccess);

  TEST(json_object_set_string(obj, "boundary 1", "\xed\x9f\xbf") ==
       JSONSuccess);
  TEST(json_object_set_string(obj, "boundary 2", "\xee\x80\x80") ==
       JSONSuccess);
  TEST(json_object_set_string(obj, "boundary 3", "\xef\xbf\xbd") ==
       JSONSuccess);
  TEST(json_object_set_string(obj, "boundary 4", "\xf4\x8f\xbf\xbf") ==
       JSONSuccess);

  TEST(json_object_set_string(obj, "first continuation byte", "\x80") ==
       JSONFailure);
  TEST(json_object_set_string(obj, "last continuation byte", "\xbf") ==
       JSONFailure);

  TEST(json_object_set_string(obj, "impossible sequence 1", "\xfe") ==
       JSONFailure);
  TEST(json_object_set_string(obj, "impossible sequence 2", "\xff") ==
       JSONFailure);
  TEST(json_object_set_string(obj, "impossible sequence 3",
                              "\xfe\xfe\xff\xff") == JSONFailure);

  TEST(json_object_set_string(obj, "overlong 1", "\xc0\xaf") == JSONFailure);
  TEST(json_object_set_string(obj, "overlong 2", "\xc1\xbf") == JSONFailure);
  TEST(json_object_set_string(obj, "overlong 3", "\xe0\x80\xaf") ==
       JSONFailure);
  TEST(json_object_set_string(obj, "overlong 4", "\xe0\x9f\xbf") ==
       JSONFailure);
  TEST(json_object_set_string(obj, "overlong 5", "\xf0\x80\x80\xaf") ==
       JSONFailure);
  TEST(json_object_set_string(obj, "overlong 6", "\xf0\x8f\xbf\xbf") ==
       JSONFailure);
  TEST(json_object_set_string(obj, "overlong 7", "\xf0\x8f\xbf\xbf") ==
       JSONFailure);

  TEST(json_object_set_string(obj, "overlong null 1", "\xc0\x80") ==
       JSONFailure);
  TEST(json_object_set_string(obj, "overlong null 2", "\xe0\x80\x80") ==
       JSONFailure);
  TEST(json_object_set_string(obj, "overlong null 3", "\xf0\x80\x80\x80") ==
       JSONFailure);
  TEST(json_object_set_string(obj, "overlong null 4", "\xf8\x80\x80\x80\x80") ==
       JSONFailure);
  TEST(json_object_set_string(obj, "overlong null 5",
                              "\xfc\x80\x80\x80\x80\x80") == JSONFailure);

  TEST(json_object_set_string(obj, "single surrogate 1", "\xed\xa0\x80") ==
       JSONFailure);
  TEST(json_object_set_string(obj, "single surrogate 2", "\xed\xaf\xbf") ==
       JSONFailure);
  TEST(json_object_set_string(obj, "single surrogate 3", "\xed\xbf\xbf") ==
       JSONFailure);

  /* Testing removing values from array, order of the elements should be
   * preserved */
  remove_test_val = json_parse_string("[1, 2, 3, 4, 5]");
  remove_test_arr = json_array(remove_test_val);
  json_array_remove(remove_test_arr, 2);

  {
    JSON_Value *__tmp = json_parse_string("[1, 2, 4, 5]");
    TEST(json_value_equals(remove_test_val, __tmp));
    json_value_free(__tmp);
  }

  json_array_remove(remove_test_arr, 0);

  {
    JSON_Value *__tmp = json_parse_string("[2, 4, 5]");
    TEST(json_value_equals(remove_test_val, __tmp));
    json_value_free(__tmp);
  }

  json_array_remove(remove_test_arr, 2);

  {
    JSON_Value *__tmp = json_parse_string("[2, 4]");
    TEST(json_value_equals(remove_test_val, __tmp));
    json_value_free(__tmp);
  }

  /* Testing nan and inf */
  TEST(json_object_set_number(obj, "num", 0.0 / zero) == JSONFailure);
  TEST(json_object_set_number(obj, "num", 1.0 / zero) == JSONFailure);
  json_value_free(val_from_file);
  json_value_free(val);
  json_value_free(remove_test_val);
}

/**
 * \\brief test_suite_6
 */
void test_suite_6(void) {
  const char *filename = "test_2.txt";
  JSON_Value *a = NULL;
  JSON_Value *b = NULL;
  a = json_parse_file(get_file_path(filename));
  b = json_parse_file(get_file_path(filename));
  TEST(json_value_equals(a, b));
  json_object_set_string(json_object(a), "string", "eki");
  TEST(!json_value_equals(a, b));
  json_value_free(a);
  a = json_value_deep_copy(b);
  TEST(json_value_equals(a, b));
  json_array_append_number(
      json_object_get_array(json_object(b), "string array"), 1337);
  TEST(!json_value_equals(a, b));
  json_value_free(a);
  json_value_free(b);
}

/**
 * \\brief test_suite_7
 */
void test_suite_7(void) {
  JSON_Value *val_from_file = json_parse_file(get_file_path("test_5.txt"));
  JSON_Value *schema = json_value_init_object();
  JSON_Object *schema_obj = json_value_get_object(schema);
  JSON_Array *interests_arr = NULL;
  json_object_set_string(schema_obj, "first", "");
  json_object_set_string(schema_obj, "last", "");
  json_object_set_number(schema_obj, "age", 0);
  json_object_set_value(schema_obj, "interests", json_value_init_array());
  interests_arr = json_object_get_array(schema_obj, "interests");
  json_array_append_string(interests_arr, "");
  json_object_set_null(schema_obj, "favorites");
  TEST(json_validate(schema, val_from_file) == JSONSuccess);
  json_object_set_string(schema_obj, "age", "");
  TEST(json_validate(schema, val_from_file) == JSONFailure);
  json_value_free(val_from_file);
  json_value_free(schema);
}

/**
 * \\brief test_suite_8
 */
void test_suite_8(void) {
  const char *filename = "test_2.txt";
  const char *temp_filename = "test_2_serialized.txt";
  JSON_Value *a = NULL;
  JSON_Value *b = NULL;
  char *buf = NULL;
  size_t serialization_size = 0;
  a = json_parse_file(get_file_path(filename));
  TEST(json_serialize_to_file(a, get_file_path(temp_filename)) == JSONSuccess);
  b = json_parse_file(get_file_path(temp_filename));
  TEST(json_value_equals(a, b));
  remove(get_file_path(temp_filename));
  serialization_size = json_serialization_size(a);
  buf = json_serialize_to_string(a);
  TEST((strlen(buf) + 1) == serialization_size);
  json_free_serialized_string(buf);
  json_value_free(a);
  json_value_free(b);
}

/**
 * \\brief test_suite_9
 */
void test_suite_9(void) {
  const char *filename = "test_2_pretty.txt";
  const char *temp_filename = "test_2_serialized_pretty.txt";
  char *file_contents = NULL;
  char *serialized = NULL;
  JSON_Value *a = NULL;
  JSON_Value *b = NULL;
  size_t serialization_size = 0;
  a = json_parse_file(get_file_path(filename));
  TEST(json_serialize_to_file_pretty(a, get_file_path(temp_filename)) ==
       JSONSuccess);
  b = json_parse_file(get_file_path(temp_filename));
  TEST(json_value_equals(a, b));
  remove(get_file_path(temp_filename));
  serialization_size = json_serialization_size_pretty(a);
  serialized = json_serialize_to_string_pretty(a);
  TEST((strlen(serialized) + 1) == serialization_size);

  if (test_read_file(get_file_path(filename), &file_contents) != JSONSuccess) {
    file_contents = NULL;
  }

  if (file_contents) {
    size_t len = strlen(file_contents);
    if (len > 0 && file_contents[len - 1] == '\n') {
      file_contents[len - 1] = '\0';
      if (len > 1 && file_contents[len - 2] == '\r') {
        file_contents[len - 2] = '\0';
      }
    }
  }

  TEST(STREQ(file_contents, serialized));
  free(file_contents);
  json_free_serialized_string(serialized);
  json_value_free(a);
  json_value_free(b);
}

/**
 * \\brief test_suite_10
 */
void test_suite_10(void) {
  JSON_Value *val;
  char *serialized;

  g_malloc_count = 0;

  val = json_parse_file(get_file_path("test_1_1.txt"));
  json_value_free(val);

  val = json_parse_file(get_file_path("test_1_3.txt"));
  json_value_free(val);

  val = json_parse_file(get_file_path("test_2.txt"));
  serialized = json_serialize_to_string_pretty(val);
  json_free_serialized_string(serialized);
  json_value_free(val);

  val = json_parse_file(get_file_path("test_2_pretty.txt"));
  json_value_free(val);

  TEST(g_malloc_count == 0);
}

/**
 * \\brief test_suite_11
 */
void test_suite_11(void) {
  const char *array_with_slashes = "[\"a/b/c\"]";
  const char *array_with_escaped_slashes = "[\"a\\/b\\/c\"]";
  char *serialized = NULL;
  JSON_Value *value = json_parse_string(array_with_slashes);

  serialized = json_serialize_to_string(value);
  TEST(STREQ(array_with_escaped_slashes, serialized));
  json_free_serialized_string(serialized);

  json_set_escape_slashes(0);
  serialized = json_serialize_to_string(value);
  TEST(STREQ(array_with_slashes, serialized));
  json_free_serialized_string(serialized);

  json_set_escape_slashes(1);
  serialized = json_serialize_to_string(value);
  TEST(STREQ(array_with_escaped_slashes, serialized));
  json_free_serialized_string(serialized);

  json_value_free(value);
}

/**
 * \\brief test_memory_leaks
 */
void test_memory_leaks(void) {
  g_malloc_count = 0;

  TEST(json_object_set_string(NULL, "lorem", "ipsum") == JSONFailure);
  TEST(json_object_set_number(NULL, "lorem", 42) == JSONFailure);
  TEST(json_object_set_boolean(NULL, "lorem", 0) == JSONFailure);
  TEST(json_object_set_null(NULL, "lorem") == JSONFailure);

  TEST(json_parse_string("{\"\\u0000\"") == NULL);

  TEST(g_malloc_count == 0);
}

/**
 * \\brief test_failing_allocations_for_all_apis
 */
void test_failing_allocations_for_all_apis(void) {
  int n = 0;

  json_set_allocation_functions(failing_malloc, failing_free);
  printf("Testing failing allocations for all APIs: ");

  for (;;) {
    g_failing_alloc.allocation_to_fail = n;
    g_failing_alloc.alloc_count = 0;
    g_failing_alloc.total_count = 0;
    g_failing_alloc.has_failed = 0;
    g_failing_alloc.should_fail = 1;
    n++;

    test_all_apis();
    test_null_and_invalid_apis();

    if (g_failing_alloc.alloc_count != 0) {
      printf("Leak after failing allocation %d (alloc_count = %d)\n", n - 1,
             g_failing_alloc.alloc_count);
      g_tests_failed++;
      return;
    }

    if (!g_failing_alloc.has_failed) {
      break;
    }
  }

  json_set_allocation_functions(counted_malloc, counted_free);
  printf("OK (tested %d failing allocations)\n", n - 1);
  g_tests_passed++;
}

/**
 * \\brief test_failing_allocations
 */
void test_failing_allocations(void) {
  const char *filename = "test_2.txt";
  JSON_Value *root_value = NULL;
  JSON_Object *root_object = NULL;
  int i = 0;
  int n = 0;
  char key_val_buf[32];

  json_set_allocation_functions(failing_malloc, failing_free);

  printf("Testing failing allocations: ");

  for (;;) {
    /*        printf("Failing at allocation %d\n", n); */
    g_failing_alloc.allocation_to_fail = n;
    g_failing_alloc.alloc_count = 0;
    g_failing_alloc.total_count = 0;
    g_failing_alloc.has_failed = 0;
    g_failing_alloc.should_fail = 1;
    n++;

    root_value = json_parse_file(get_file_path(filename));
    if (g_failing_alloc.has_failed) {
      if (root_value) {
        printf(
            "Allocation has failed but parsing succeeded after allocation %d\n",
            n - 1);
        g_tests_failed++;
        return;
      }
    }

    if (root_value) {
      root_object = json_object(root_value);
      for (i = 0; i < 64; i++) {
#if defined(_MSC_VER)
        sprintf_s(key_val_buf, sizeof(key_val_buf), "%d", i);
#else
        sprintf(key_val_buf, "%d", i);
#endif
        json_object_set_string(root_object, key_val_buf, key_val_buf);
      }

      for (i = 0; i < 64; i++) {
#if defined(_MSC_VER)
        sprintf_s(key_val_buf, sizeof(key_val_buf), "%d", i);
#else
        sprintf(key_val_buf, "%d", i);
#endif
        json_object_set_string(root_object, key_val_buf, key_val_buf);
      }

      json_object_dotset_number(root_object, "ala.ma.kota", 123);
      json_object_dotremove(root_object, "ala.ma.kota");
    }

    json_value_free(root_value);

    if (g_failing_alloc.alloc_count != 0) {
      printf("Leak after failing allocation %d (alloc_count = %d)\n", n - 1,
             g_failing_alloc.alloc_count);
      g_tests_failed++;
      return;
    }

    if (!g_failing_alloc.has_failed) {
      break;
    }
  }

  json_set_allocation_functions(counted_malloc, counted_free);
  printf("OK (tested %d failing allocations)\n", n - 1);
  g_tests_passed++;
}

/**
 * \\brief test_custom_number_format
 */
void test_custom_number_format(void) {
  g_malloc_count = 0;
  {
    char *serialized = NULL;
    JSON_Value *val = json_value_init_number(0.6);
    json_set_float_serialization_format("%.1f");
    serialized = json_serialize_to_string(val);
    json_set_float_serialization_format(NULL);
    printf("Serialized: '%s'\n", serialized);
    TEST(STREQ(serialized, "0.6"));
    json_free_serialized_string(serialized);
    json_value_free(val);
  }
  TEST(g_malloc_count == 0);
}

/** \brief Flag tracking if custom serialization was called */
static int custom_serialization_func_called = 0;
/**
 * \\brief custom_serialization_func
 * \\param num Parameter num
 * \\param buf Parameter buf
 * \\return Returns static int
 */
static int custom_serialization_func(double num, char *buf) {
  char num_buf[32];
  custom_serialization_func_called = 1;
  if (buf == NULL)
    buf = num_buf;
#if defined(_MSC_VER)
  return sprintf_s(buf, 32, "%.1f", num);
#else
  return sprintf(buf, "%.1f", num);
#endif
}

/**
 * \\brief test_custom_number_serialization_function
 */
void test_custom_number_serialization_function(void) {
  g_malloc_count = 0;
  {
    /* We just test that custom_serialization_func() gets called, not it's
     * performance */
    char *serialized = NULL;
    JSON_Value *val = json_value_init_number(0.6);
    json_set_number_serialization_function(custom_serialization_func);
    serialized = json_serialize_to_string(val);
    printf("Serialized2: '%s'\n", serialized);
    TEST(STREQ(serialized, "0.6"));
    TEST(custom_serialization_func_called);
    json_set_number_serialization_function(NULL);
    json_free_serialized_string(serialized);
    json_value_free(val);
  }
  TEST(g_malloc_count == 0);
}

/**
 * \\brief test_object_clear
 */
void test_object_clear(void) {
  g_malloc_count = 0;
  {
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    json_object_set_string(obj, "foo", "bar");
    json_object_clear(obj);
    TEST(json_object_get_value(obj, "foo") == NULL);
    json_value_free(val);
  }
  TEST(g_malloc_count == 0);
}

/**
 * \\brief print_commits_info
 * \\param username Parameter username
 * \\param repo Parameter repo
 */
void print_commits_info(const char *username, const char *repo) {
  JSON_Value *root_value;
  JSON_Array *commits;
  JSON_Object *commit;
  size_t i;

  char curl_command[512];
  char cleanup_command[256];
  char output_filename[] = "commits.json";

  /* it ain't pretty, but it's not a libcurl tutorial */
#if defined(_MSC_VER)
  sprintf_s(curl_command, sizeof(curl_command),
            "curl -s \"https://api.github.com/repos/%s/%s/commits\" > %s",
            username, repo, output_filename);
  sprintf_s(cleanup_command, sizeof(cleanup_command), "rm -f %s",
            output_filename);
#else
  sprintf(curl_command,
          "curl -s \"https://api.github.com/repos/%s/%s/commits\" > %s",
          username, repo, output_filename);
  sprintf(cleanup_command, "rm -f %s", output_filename);
#endif
  system(curl_command);

  /* parsing json and validating output */
  root_value = json_parse_file(get_file_path(output_filename));
  if (json_value_get_type(root_value) != JSONArray) {
    system(cleanup_command);
    return;
  }

  /* getting array from root value and printing commit info */
  commits = json_value_get_array(root_value);
  printf("%-10.10s %-10.10s %s\n", "Date", "SHA", "Author");
  for (i = 0; i < json_array_get_count(commits); i++) {
    commit = json_array_get_object(commits, i);
    printf("%.10s %.10s %s\n",
           json_object_dotget_string(commit, "commit.author.date"),
           json_object_get_string(commit, "sha"),
           json_object_dotget_string(commit, "commit.author.name"));
  }

  /* cleanup code */
  json_value_free(root_value);
  system(cleanup_command);
}

/**
 * \\brief persistence_example
 */
void persistence_example(void) {
  JSON_Value *schema = json_parse_string("{\"name\":\"\"}");
  JSON_Value *user_data = json_parse_file(get_file_path("user_data.json"));
  char buf[256];
  const char *name = NULL;
  if (user_data == NULL || json_validate(schema, user_data) != JSONSuccess) {
    puts("Enter your name:");
#if defined(_MSC_VER)
    scanf_s("%s", buf, (unsigned)sizeof(buf));
#else
    scanf("%s", buf);
#endif
    user_data = json_value_init_object();
    json_object_set_string(json_object(user_data), "name", buf);
    json_serialize_to_file(user_data, "user_data.json");
  }
  name = json_object_get_string(json_object(user_data), "name");
  printf("Hello, %s.", name);
  json_value_free(schema);
  json_value_free(user_data);
  return;
}

/**
 * \\brief serialization_example
 */
void serialization_example(void) {
  JSON_Value *root_value = json_value_init_object();
  JSON_Object *root_object = json_value_get_object(root_value);
  char *serialized_string = NULL;
  json_object_set_string(root_object, "name", "John Smith");
  json_object_set_number(root_object, "age", 25);
  json_object_dotset_string(root_object, "address.city", "Cupertino");
  json_object_dotset_value(
      root_object, "contact.emails",
      json_parse_string("[\"email@example.com\", \"email2@example.com\"]"));
  serialized_string = json_serialize_to_string_pretty(root_value);
  puts(serialized_string);
  json_free_serialized_string(serialized_string);
  json_value_free(root_value);
}

/**
 * \\brief test_read_file
 * \\param file_path Parameter file_path
 * \\return Returns JSON_Status
 */
static JSON_Status test_read_file(const char *file_path, char **out_contents) {
  FILE *fp = NULL;
  size_t size_to_read = 0;
  size_t size_read = 0;
  long pos;
  char *file_contents;
#if defined(_MSC_VER)
  if (fopen_s(&fp, file_path, "r") != 0) {
    fp = NULL;
  }
#else
  fp = fopen(file_path, "r");
#endif
  if (!fp) {
    assert(0);
    return JSONFailure;
  }
  fseek(fp, 0L, SEEK_END);
  pos = ftell(fp);
  if (pos < 0) {
    fclose(fp);
    assert(0);
    return JSONFailure;
  }
  size_to_read = pos;
  rewind(fp);
  file_contents = (char *)malloc(sizeof(char) * (size_to_read + 1));
  if (!file_contents) {
    fclose(fp);
    assert(0);
    return JSONFailure;
  }
  size_read = fread(file_contents, 1, size_to_read, fp);
  if (size_read == 0 || ferror(fp)) {
    fclose(fp);
    free(file_contents);
    assert(0);
    return JSONFailure;
  }
  fclose(fp);
  file_contents[size_read] = '\0';
  *out_contents = file_contents;
  return JSONSuccess;
}

/** \\brief get_file_path test */
const char *get_file_path(const char *filename) {
  static char path_buf[2048] = {0};
  memset(path_buf, 0, sizeof(path_buf));
#if defined(_MSC_VER)
  sprintf_s(path_buf, sizeof(path_buf), "%s/%s", g_tests_path, filename);
#else
  sprintf(path_buf, "%s/%s", g_tests_path, filename);
#endif
  return path_buf;
}

/**
 * \\brief counted_malloc
 * \\param size Parameter size
 * \\return Returns static void *
 */
static void *counted_malloc(size_t size) {
  void *res = malloc(size);
  if (res != NULL) {
    g_malloc_count++;
  }
  return res;
}

/**
 * \\brief counted_free
 * \\param ptr Parameter ptr
 * \\return Returns static void
 */
static void counted_free(void *ptr) {
  if (ptr != NULL) {
    g_malloc_count--;
  }
  free(ptr);
}

/**
 * \\brief failing_malloc
 * \\param size Parameter size
 * \\return Returns static void *
 */
static void *failing_malloc(size_t size) {
  void *res = NULL;
  if (g_failing_alloc.should_fail &&
      g_failing_alloc.total_count >= g_failing_alloc.allocation_to_fail) {
    g_failing_alloc.has_failed = 1;
    return NULL;
  }
  res = malloc(size);
  if (res != NULL) {
    g_failing_alloc.total_count++;
    g_failing_alloc.alloc_count++;
  }
  return res;
}

/**
 * \\brief failing_free
 * \\param ptr Parameter ptr
 * \\return Returns static void
 */
static void failing_free(void *ptr) {
  if (ptr != NULL) {
    g_failing_alloc.alloc_count--;
  }
  free(ptr);
}

int g_failing_file = 0;

#undef fseek
#undef ftell
#undef fread
#undef ferror

/**
 * \\brief failing_ftell
 * \\param stream Parameter stream
 */
long failing_ftell(FILE *stream) {
  if (g_failing_file == 1)
    return -1;
  return ftell(stream);
}
/**
 * \\brief failing_fseek
 * \\param stream Parameter stream
 * \\param offset Parameter offset
 * \\param whence Parameter whence
 */
int failing_fseek(FILE *stream, long offset, int whence) {
  if (g_failing_file == 2)
    return -1;
  return fseek(stream, offset, whence);
}
/**
 * \\brief failing_fread
 * \\param ptr Parameter ptr
 * \\param size Parameter size
 * \\param nmemb Parameter nmemb
 * \\param stream Parameter stream
 */
size_t failing_fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  if (g_failing_file == 3)
    return 0;
  return fread(ptr, size, nmemb, stream);
}
/**
 * \\brief failing_ferror
 * \\param stream Parameter stream
 */
int failing_ferror(FILE *stream) {
  if (g_failing_file == 4)
    return 1;
  return ferror(stream);
}
