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
 * @file tests.c
 * @brief Test suite for parson JSON library using greatest.h
 * @author Krzysztof Gabis
 */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef PARSON_SINGLE_HEADER
/** @brief Implement parson inside single header */
#define PARSON_IMPLEMENTATION

/**
 * @brief Simulated ftell wrapper for I/O failure injection
 * @param stream Target FILE stream
 * @return Stream position or -1 when g_failing_file == 1
 */
static long failing_ftell(FILE *stream);
/**
 * @brief Simulated fseek wrapper for I/O failure injection
 * @param stream Target FILE stream
 * @param offset Byte offset
 * @param whence Seek origin
 * @return 0 on success or -1 when g_failing_file == 2
 */
static int failing_fseek(FILE *stream, long offset, int whence);
/**
 * @brief Simulated fread wrapper for I/O failure injection
 * @param ptr Destination buffer
 * @param size Element size
 * @param nmemb Element count
 * @param stream Target FILE stream
 * @return Elements read or 0 when g_failing_file == 3
 */
static size_t failing_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
/**
 * @brief Simulated ferror wrapper for I/O failure injection
 * @param stream Target FILE stream
 * @return Error indicator or 1 when g_failing_file == 4
 */
static int failing_ferror(FILE *stream);
/**
 * @brief Simulated fputs wrapper for I/O failure injection
 * @param s String to write
 * @param stream Target FILE stream
 * @return Non-negative on success or EOF when g_failing_file == 5
 */
static int failing_fputs(const char *s, FILE *stream);
/**
 * @brief Simulated fclose wrapper for I/O failure injection
 * @param stream Target FILE stream
 * @return 0 on success or EOF when g_failing_file == 6
 */
static int failing_fclose(FILE *stream);

/** @brief Override fseek inside parson */
#define PARSON_FSEEK failing_fseek
/** @brief Override ftell inside parson */
#define PARSON_FTELL failing_ftell
/** @brief Override fread inside parson */
#define PARSON_FREAD failing_fread
/** @brief Override ferror inside parson */
#define PARSON_FERROR failing_ferror
/** @brief Override fputs inside parson */
#define PARSON_FPUTS failing_fputs
/** @brief Override fclose inside parson */
#define PARSON_FCLOSE failing_fclose
#endif

#include "../parson.h"
#include "greatest.h"
#ifdef __WATCOMC__
#include <float.h>
#endif
#if defined(__DOS__) || defined(MSDOS) || defined(_MSDOS) || defined(__MSDOS__)
#include "dos_fixtures.h"
#endif
/* clang-format on */

/** @brief Macro to check string equality safely with NULL guards */
#define STREQ(A, B) ((A) && (B) ? strcmp((A), (B)) == 0 : 0)

/** @brief Double precision epsilon */
#define PARSON_TEST_EPSILON 2.2204460492503131e-16
/** @brief Check if two doubles are approximately equal */
#define DBL_EQ(a, b) (fabs((a) - (b)) < PARSON_TEST_EPSILON)

/** @brief Path to test fixture directory */
static const char *g_tests_path = "tests";

/** @brief Global active allocation counter */
static int g_malloc_count = 0;

#ifdef PARSON_SINGLE_HEADER
/** @brief Global flag to simulate failing file stream operations */
static int g_failing_file = 0;
#endif

/** @brief Tracker for deterministic OOM injection */
typedef struct failing_alloc {
  /** @brief Allocation index that should fail */
  int allocation_to_fail;
  /** @brief Active unfreed allocation count */
  int alloc_count;
  /** @brief Total allocations performed in current run */
  int total_count;
  /** @brief Flag indicating an allocation failed */
  int has_failed;
  /** @brief Flag enabling failure injection */
  int should_fail;
} failing_alloc_t;

/** @brief Global failing allocator state */
static failing_alloc_t g_failing_alloc;

/**
 * @brief Build path to file inside g_tests_path
 * @param filename Relative fixture filename
 * @return Pointer to static NUL-terminated path buffer
 */
static const char *get_file_path(const char *filename) {
#if defined(__DOS__) || defined(MSDOS) || defined(_MSDOS) || defined(__MSDOS__)
  (void)g_tests_path;
  return dos_map_fixture_path(filename);
#else
  static char path_buf[2048];
  memset(path_buf, 0, sizeof(path_buf));
#if defined(_MSC_VER)
  sprintf_s(path_buf, sizeof(path_buf), "%s/%s", g_tests_path, filename);
#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||            \
    (defined(__cplusplus) && __cplusplus >= 201103L)
  snprintf(path_buf, sizeof(path_buf), "%s/%s", g_tests_path, filename);
#else
  sprintf(path_buf, "%s/%s", g_tests_path, filename);
#endif
  return path_buf;
#endif
}

/**
 * @brief Read entire text file for test comparison
 * @param file_path Path to file
 * @param out_contents Output allocated string
 * @return JSONSuccess on success, JSONFailure on error
 */
static enum json_result_t test_read_file(const char *file_path,
                                         char **out_contents) {
  FILE *fp = NULL;
  size_t size_to_read = 0;
  size_t size_read = 0;
  size_t read_ix = 0;
  size_t write_ix = 0;
  long pos = 0;
  char *file_contents = NULL;

  *out_contents = NULL;
#if defined(_MSC_VER)
  if (fopen_s(&fp, file_path, "r") != 0) {
    fp = NULL;
  }
#else
  fp = fopen(file_path, "r");
#endif
  if (!fp) {
    return JSONFailure;
  }
  if (fseek(fp, 0L, SEEK_END) != 0) {
    fclose(fp);
    return JSONFailure;
  }
  pos = ftell(fp);
  if (pos < 0) {
    fclose(fp);
    return JSONFailure;
  }
  size_to_read = (size_t)pos;
  rewind(fp);
  file_contents = (char *)malloc(sizeof(char) * (size_to_read + 1));
  if (!file_contents) {
    fclose(fp);
    return JSONFailure;
  }
  size_read = fread(file_contents, 1, size_to_read, fp);
  if (size_read == 0 || ferror(fp)) {
    fclose(fp);
    free(file_contents);
    return JSONFailure;
  }
  fclose(fp);
  for (read_ix = 0; read_ix < size_read; read_ix++) {
    if (file_contents[read_ix] == '\r' && read_ix + 1 < size_read &&
        file_contents[read_ix + 1] == '\n') {
      continue;
    }
    file_contents[write_ix++] = file_contents[read_ix];
  }
  file_contents[write_ix] = '\0';
  *out_contents = file_contents;
  return JSONSuccess;
}

/**
 * @brief Allocation wrapper counting active allocations
 * @param size Bytes to allocate
 * @return Allocated pointer or NULL
 */
static void *counted_malloc(size_t size) {
  void *res = malloc(size);
  if (res != NULL) {
    g_malloc_count++;
  }
  return res;
}

/**
 * @brief Deallocation wrapper decrementing active allocations
 * @param ptr Pointer to free
 */
static void counted_free(void *ptr) {
  if (ptr != NULL) {
    g_malloc_count--;
  }
  free(ptr);
}

/**
 * @brief Allocation wrapper that fails at a configured call count
 * @param size Bytes to allocate
 * @return Allocated pointer or NULL on injected OOM
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
 * @brief Deallocation wrapper paired with failing_malloc
 * @param ptr Pointer to free
 */
static void failing_free(void *ptr) {
  if (ptr != NULL) {
    g_failing_alloc.alloc_count--;
  }
  free(ptr);
}

#ifdef PARSON_SINGLE_HEADER
static long failing_ftell(FILE *stream) {
  if (g_failing_file == 1) {
    return -1;
  }
  return ftell(stream);
}

static int failing_fseek(FILE *stream, long offset, int whence) {
  if (g_failing_file == 2) {
    return -1;
  }
  return fseek(stream, offset, whence);
}

static size_t failing_fread(void *ptr, size_t size, size_t nmemb,
                            FILE *stream) {
  if (g_failing_file == 3) {
    return 0;
  }
  return fread(ptr, size, nmemb, stream);
}

static int failing_ferror(FILE *stream) {
  if (g_failing_file == 4) {
    return 1;
  }
  return ferror(stream);
}

static int failing_fputs(const char *s, FILE *stream) {
  if (g_failing_file == 5) {
    return EOF;
  }
  return fputs(s, stream);
}

static int failing_fclose(FILE *stream) {
  int rc = fclose(stream);
  if (g_failing_file == 6) {
    return EOF;
  }
  return rc;
}
#endif

/** @brief Guard indicating add_tests.c is included from tests.c */
#define PARSON_ADD_TESTS_INCLUDED 1
#include "add_tests.c"

/**
 * @brief Verify compact and pretty round-trip serialization equality
 * @param val Value to serialize and re-parse
 * @return greatest_test_res
 */
static enum greatest_test_res verify_roundtrip_serialization(JSON_Value *val) {
  char *ser = NULL;
  JSON_Value *par = NULL;

  ser = json_serialize_to_string(val);
  ASSERT(ser != NULL);
  par = json_parse_string(ser);
  ASSERT(par != NULL);
  ASSERT(json_value_equals(par, val));
  json_free_serialized_string(ser);
  json_value_free(par);

  ser = json_serialize_to_string_pretty(val);
  ASSERT(ser != NULL);
  par = json_parse_string(ser);
  ASSERT(par != NULL);
  ASSERT(json_value_equals(par, val));
  json_free_serialized_string(ser);
  json_value_free(par);

  PASS();
}

/**
 * @brief Test 3 sample files from json.org and nesting depth limit
 * @return greatest_test_res
 */
TEST test_suite_1(void) {
  JSON_Value *val = NULL;

  val = json_parse_file(get_file_path("test_1_1.txt"));
  ASSERT(val != NULL);
  CHECK_CALL(verify_roundtrip_serialization(val));
  json_value_free(val);

  /* Over 2048 levels of nesting */
  val = json_parse_file(get_file_path("test_1_2.txt"));
  ASSERT(val == NULL);

  val = json_parse_file(get_file_path("test_1_3.txt"));
  ASSERT(val != NULL);
  CHECK_CALL(verify_roundtrip_serialization(val));
  json_value_free(val);

  val = json_parse_file_with_comments(get_file_path("test_1_1.txt"));
  ASSERT(val != NULL);
  CHECK_CALL(verify_roundtrip_serialization(val));
  json_value_free(val);

  val = json_parse_file_with_comments(get_file_path("test_1_2.txt"));
  ASSERT(val == NULL);

  val = json_parse_file_with_comments(get_file_path("test_1_3.txt"));
  ASSERT(val != NULL);
  CHECK_CALL(verify_roundtrip_serialization(val));
  json_value_free(val);

  PASS();
}

/**
 * @brief Shared assertions verifying parsed test_2 document structure
 * @param root_value Parsed root JSON_Value
 * @return greatest_test_res
 */
static enum greatest_test_res check_suite_2_root(JSON_Value *root_value) {
  JSON_Object *root_object = NULL;
  JSON_Array *array = NULL;
  JSON_Value *array_value = NULL;
  size_t len = 0;
  size_t i = 0;

  ASSERT(root_value != NULL);
  ASSERT_EQ(JSONObject, json_value_get_type(root_value));
  root_object = json_value_get_object(root_value);

  ASSERT(json_object_has_value(root_object, "string"));
  ASSERT_FALSE(json_object_has_value(root_object, "_string"));
  ASSERT(json_object_has_value_of_type(root_object, "object", JSONObject));
  ASSERT_FALSE(
      json_object_has_value_of_type(root_object, "string array", JSONObject));
  ASSERT(json_object_has_value_of_type(root_object, "string array", JSONArray));
  ASSERT_FALSE(json_object_has_value_of_type(root_object, "object", JSONArray));
  ASSERT(json_object_has_value_of_type(root_object, "string", JSONString));
  ASSERT_FALSE(
      json_object_has_value_of_type(root_object, "positive one", JSONString));
  ASSERT(
      json_object_has_value_of_type(root_object, "positive one", JSONNumber));
  ASSERT_FALSE(
      json_object_has_value_of_type(root_object, "string", JSONNumber));
  ASSERT(
      json_object_has_value_of_type(root_object, "boolean true", JSONBoolean));
  ASSERT_FALSE(
      json_object_has_value_of_type(root_object, "positive one", JSONBoolean));
  ASSERT(json_object_has_value_of_type(root_object, "null", JSONNull));
  ASSERT_FALSE(json_object_has_value_of_type(root_object, "object", JSONNull));

  ASSERT(json_object_dothas_value(root_object, "object.nested array"));
  ASSERT_FALSE(json_object_dothas_value(root_object, "_object.nested array"));
  ASSERT(json_object_dothas_value_of_type(root_object, "object.nested object",
                                          JSONObject));
  ASSERT_FALSE(json_object_dothas_value_of_type(
      root_object, "object.nested array", JSONObject));
  ASSERT(json_object_dothas_value_of_type(root_object, "object.nested array",
                                          JSONArray));
  ASSERT_FALSE(json_object_dothas_value_of_type(
      root_object, "object.nested object", JSONArray));
  ASSERT(json_object_dothas_value_of_type(root_object, "object.nested string",
                                          JSONString));
  ASSERT_FALSE(json_object_dothas_value_of_type(
      root_object, "object.nested number", JSONString));
  ASSERT(json_object_dothas_value_of_type(root_object, "object.nested number",
                                          JSONNumber));
  ASSERT_FALSE(json_object_dothas_value_of_type(
      root_object, "_object.nested whatever", JSONNumber));
  ASSERT(json_object_dothas_value_of_type(root_object, "object.nested true",
                                          JSONBoolean));
  ASSERT_FALSE(json_object_dothas_value_of_type(
      root_object, "object.nested number", JSONBoolean));
  ASSERT(json_object_dothas_value_of_type(root_object, "object.nested null",
                                          JSONNull));
  ASSERT_FALSE(json_object_dothas_value_of_type(
      root_object, "object.nested object", JSONNull));

  ASSERT(STREQ(json_object_get_string(root_object, "string"), "lorem ipsum"));
  ASSERT(
      STREQ(json_object_get_string(root_object, "utf string"), "lorem ipsum"));
  ASSERT(
      STREQ(json_object_get_string(root_object, "utf-8 string"), "あいうえお"));
  ASSERT(STREQ(json_object_get_string(root_object, "surrogate string"),
               "lorem𝄞ipsum𝍧lorem"));

  len = json_object_get_string_len(root_object, "string with null");
  ASSERT_EQ(7u, len);
  ASSERT_EQ(0, memcmp(json_object_get_string(root_object, "string with null"),
                      "abc\0def", len));

  ASSERT(DBL_EQ(json_object_get_number(root_object, "positive one"), 1.0));
  ASSERT(DBL_EQ(json_object_get_number(root_object, "negative one"), -1.0));
  ASSERT(DBL_EQ(json_object_get_number(root_object, "hard to parse number"),
                -0.000314));
  ASSERT_EQ(1, json_object_get_boolean(root_object, "boolean true"));
  ASSERT_EQ(0, json_object_get_boolean(root_object, "boolean false"));
  ASSERT_EQ(JSONNull,
            json_value_get_type(json_object_get_value(root_object, "null")));

  array = json_object_get_array(root_object, "string array");
  ASSERT(array != NULL);
  ASSERT(json_array_get_count(array) > 1);
  ASSERT(STREQ(json_array_get_string(array, 0), "lorem"));
  ASSERT(STREQ(json_array_get_string(array, 1), "ipsum"));

  array = json_object_get_array(root_object, "x^2 array");
  ASSERT(array != NULL);
  for (i = 0; i < json_array_get_count(array); i++) {
    ASSERT(DBL_EQ(json_array_get_number(array, i), (double)(i * i)));
  }

  ASSERT(json_object_get_array(root_object, "non existent array") == NULL);
  ASSERT(STREQ(json_object_dotget_string(root_object, "object.nested string"),
               "str"));
  ASSERT_EQ(1, json_object_dotget_boolean(root_object, "object.nested true"));
  ASSERT_EQ(0, json_object_dotget_boolean(root_object, "object.nested false"));
  ASSERT(json_object_dotget_value(root_object, "object.nested null") != NULL);
  ASSERT(DBL_EQ(json_object_dotget_number(root_object, "object.nested number"),
                123.0));

  ASSERT(json_object_dotget_value(root_object, "should.be.null") == NULL);
  ASSERT(json_object_dotget_value(root_object, "should.be.null.") == NULL);
  ASSERT(json_object_dotget_value(root_object, ".") == NULL);
  ASSERT(json_object_dotget_value(root_object, "") == NULL);

  array = json_object_dotget_array(root_object, "object.nested array");
  ASSERT(array != NULL);
  ASSERT(json_array_get_count(array) > 1);
  ASSERT(STREQ(json_array_get_string(array, 0), "lorem"));
  ASSERT(STREQ(json_array_get_string(array, 1), "ipsum"));

  ASSERT(STREQ(json_object_get_string(root_object, "/**/"), "comment"));
  ASSERT(STREQ(json_object_get_string(root_object, "//"), "comment"));
  ASSERT(STREQ(json_object_get_string(root_object, "url"),
               "https://www.example.com/search?q=12345"));
  ASSERT(
      STREQ(json_object_get_string(root_object, "escaped chars"), "\" \\ /"));

  ASSERT(json_object_get_object(root_object, "empty object") != NULL);
  ASSERT(json_object_get_array(root_object, "empty array") != NULL);

  ASSERT(json_object_get_wrapping_value(root_object) == root_value);
  array = json_object_get_array(root_object, "string array");
  array_value = json_object_get_value(root_object, "string array");
  ASSERT(json_array_get_wrapping_value(array) == array_value);
  ASSERT(json_value_get_parent(array_value) == root_value);
  ASSERT(json_value_get_parent(root_value) == NULL);

  PASS();
}

/**
 * @brief Test parsing test_2.txt without comments
 * @return greatest_test_res
 */
TEST test_suite_2_no_comments(void) {
  JSON_Value *root_value = json_parse_file(get_file_path("test_2.txt"));
  CHECK_CALL(check_suite_2_root(root_value));
  CHECK_CALL(verify_roundtrip_serialization(root_value));
  json_value_free(root_value);
  PASS();
}

/**
 * @brief Test parsing test_2_comments.txt with C and C++ comments
 * @return greatest_test_res
 */
TEST test_suite_2_with_comments(void) {
  JSON_Value *root_value =
      json_parse_file_with_comments(get_file_path("test_2_comments.txt"));
  CHECK_CALL(check_suite_2_root(root_value));
  CHECK_CALL(verify_roundtrip_serialization(root_value));
  json_value_free(root_value);
  PASS();
}

/**
 * @brief Test parsing valid and malformed JSON strings
 * @return greatest_test_res
 */
TEST test_suite_3(void) {
  static const char *const valid_inputs[] = {"{\"lorem\":\"ipsum\"}",
                                             "[\"lorem\"]",
                                             "null",
                                             "true",
                                             "false",
                                             "\"string\"",
                                             "123",
                                             "[\"lorem\",]",
                                             "{\"lorem\":\"ipsum\",}",
                                             "\"\\b\\f\\n\\r\\t\\/\\\\\\\"\""};
  size_t i = 0;
  JSON_Value *tmp = NULL;

  for (i = 0; i < sizeof(valid_inputs) / sizeof(valid_inputs[0]); i++) {
    tmp = json_parse_string(valid_inputs[i]);
    ASSERT(tmp != NULL);
    json_value_free(tmp);
  }

  /* UTF-16 escape decoding */
  tmp = json_parse_string("\"\\u0024x\"");
  ASSERT(STREQ(json_string(tmp), "$x"));
  json_value_free(tmp);

  tmp = json_parse_string("\"\\u00A2x\"");
  ASSERT(STREQ(json_string(tmp), "¢x"));
  json_value_free(tmp);

  tmp = json_parse_string("\"\\u20ACx\"");
  ASSERT(STREQ(json_string(tmp), "€x"));
  json_value_free(tmp);

  tmp = json_parse_string("\"\\uD801\\uDC37x\"");
  ASSERT(STREQ(json_string(tmp), "𐐷x"));
  json_value_free(tmp);

  /* Invalid JSON inputs */
  g_malloc_count = 0;
  ASSERT(json_parse_string(NULL) == NULL);
  ASSERT(json_parse_string("") == NULL);
  ASSERT(json_parse_string("{lorem:ipsum}") == NULL);
  ASSERT(json_parse_string("{\"lorem\":\"ipsum\",]") == NULL);
  ASSERT(json_parse_string("{\"lorem\":\"ipsum\",,}") == NULL);
  ASSERT(json_parse_string("[,]") == NULL);
  ASSERT(json_parse_string("[,") == NULL);
  ASSERT(json_parse_string("[") == NULL);
  ASSERT(json_parse_string("]") == NULL);
  ASSERT(json_parse_string("[1 2]") == NULL);
  ASSERT(json_parse_string("{\"a\":0,\"a\":0}") == NULL);
  ASSERT(json_parse_string("{:,}") == NULL);
  ASSERT(json_parse_string("{,}") == NULL);
  ASSERT(json_parse_string("{,") == NULL);
  ASSERT(json_parse_string("{:") == NULL);
  ASSERT(json_parse_string("{") == NULL);
  ASSERT(json_parse_string("}") == NULL);
  ASSERT(json_parse_string("x") == NULL);
  ASSERT(json_parse_string("{:\"no name\"}") == NULL);
  ASSERT(json_parse_string("[,\"no first value\"]") == NULL);
  ASSERT(json_parse_string("{\"key\"\"value\"}") == NULL);
  ASSERT(json_parse_string("{\"a\"}") == NULL);
  ASSERT(json_parse_string("{\"a\":1 \"b\":2}") == NULL);
  ASSERT(json_parse_string("[\"\\x\"]") == NULL);
  ASSERT(json_parse_string("[\"\\u00zz\"]") == NULL);
  ASSERT(json_parse_string("[\"\\u00\"]") == NULL);
  ASSERT(json_parse_string("[\"\\u\"]") == NULL);
  ASSERT(json_parse_string("[\"\\uD800\"]") == NULL);
  ASSERT(json_parse_string("[\"\\uD800\\x\"]") == NULL);
  ASSERT(json_parse_string("[\"\\uD800\\u0041\"]") == NULL);
  ASSERT(json_parse_string("[\"\\\"]") == NULL);
  ASSERT(json_parse_string("[\"\"\"]") == NULL);
  ASSERT(json_parse_string("[\"\0\"]") == NULL);
  ASSERT(json_parse_string("[\"\a\"]") == NULL);
  ASSERT(json_parse_string("[\"\b\"]") == NULL);
  ASSERT(json_parse_string("[\"\t\"]") == NULL);
  ASSERT(json_parse_string("[\"\n\"]") == NULL);
  ASSERT(json_parse_string("[\"\f\"]") == NULL);
  ASSERT(json_parse_string("[\"\r\"]") == NULL);
  ASSERT(json_parse_string("[0x2]") == NULL);
  ASSERT(json_parse_string("[0X2]") == NULL);
  ASSERT(json_parse_string("[07]") == NULL);
  ASSERT(json_parse_string("[0070]") == NULL);
  ASSERT(json_parse_string("[07.0]") == NULL);
  ASSERT(json_parse_string("[-07]") == NULL);
  ASSERT(json_parse_string("[-007]") == NULL);
  ASSERT(json_parse_string("[-07.0]") == NULL);
  ASSERT(json_parse_string("[\"\\uDF67\\uD834\"]") == NULL);
  ASSERT(json_parse_string("[1.7976931348623157e309]") == NULL);
  ASSERT(json_parse_string("[-1.7976931348623157e309]") == NULL);
  ASSERT_EQ(0, g_malloc_count);
  PASS();
}

/**
 * @brief Test deep copy and value equality
 * @return greatest_test_res
 */
TEST test_suite_4(void) {
  JSON_Value *a = json_parse_file(get_file_path("test_2.txt"));
  JSON_Value *a_copy = NULL;

  ASSERT(a != NULL);
  ASSERT(json_value_equals(a, a));
  a_copy = json_value_deep_copy(a);
  ASSERT(a_copy != NULL);
  ASSERT(json_value_equals(a, a_copy));
  json_value_free(a);
  json_value_free(a_copy);
  PASS();
}

/**
 * @brief Test building JSON values programmatically, UTF-8 checks, and removals
 * @return greatest_test_res
 */
TEST test_suite_5(void) {
  JSON_Value *val_from_file = json_parse_file(get_file_path("test_5.txt"));
  JSON_Value *val = json_value_init_object();
  JSON_Value *val_with_parent = NULL;
  JSON_Object *obj = NULL;
  JSON_Array *interests_arr = NULL;
  JSON_Value *remove_test_val = NULL;
  JSON_Array *remove_test_arr = NULL;
  JSON_Value *tmp = NULL;

  ASSERT(val_from_file != NULL);
  ASSERT(val != NULL);
  obj = json_value_get_object(val);
  ASSERT(obj != NULL);

  ASSERT_EQ(JSONSuccess, json_object_set_string(obj, "first", "John"));
  ASSERT_EQ(JSONSuccess, json_object_set_string(obj, "last", "Doe"));
  ASSERT_EQ(JSONSuccess, json_object_set_number(obj, "age", 25));
  ASSERT_EQ(JSONSuccess, json_object_set_boolean(obj, "registered", 1));

  ASSERT_EQ(JSONSuccess,
            json_object_set_value(obj, "interests", json_value_init_array()));
  interests_arr = json_object_get_array(obj, "interests");
  ASSERT(interests_arr != NULL);
  ASSERT_EQ(JSONSuccess, json_array_append_string(interests_arr, "Writing"));
  ASSERT_EQ(JSONSuccess,
            json_array_append_string(interests_arr, "Mountain Biking"));
  ASSERT_EQ(JSONSuccess,
            json_array_replace_string(interests_arr, 0, "Reading"));

  ASSERT_EQ(JSONSuccess,
            json_object_dotset_string(obj, "favorites.color", "blue"));
  ASSERT_EQ(JSONSuccess,
            json_object_dotset_string(obj, "favorites.sport", "running"));
  ASSERT_EQ(JSONSuccess,
            json_object_dotset_string(obj, "favorites.fruit", "apple"));
  ASSERT_EQ(JSONSuccess, json_object_dotremove(obj, "favorites.fruit"));
  ASSERT_EQ(JSONSuccess,
            json_object_set_string(obj, "utf string", "lorem ipsum"));
  ASSERT_EQ(JSONSuccess,
            json_object_set_string(obj, "utf-8 string", "あいうえお"));
  ASSERT_EQ(JSONSuccess, json_object_set_string(obj, "surrogate string",
                                                "lorem𝄞ipsum𝍧lorem"));
  ASSERT_EQ(JSONSuccess, json_object_set_string_with_len(
                             obj, "string with null", "abc\0def", 7));
  ASSERT_EQ(JSONSuccess,
            json_object_set_string(obj, "windows path", "C:\\Windows\\Path"));
  ASSERT(json_value_equals(val_from_file, val));

  ASSERT_EQ(JSONFailure, json_object_set_string(obj, NULL, ""));
  ASSERT_EQ(JSONFailure, json_object_set_string(obj, "last", NULL));
  ASSERT_EQ(JSONFailure, json_object_set_string(obj, NULL, NULL));
  ASSERT_EQ(JSONFailure, json_object_set_value(obj, NULL, NULL));

  ASSERT_EQ(JSONFailure, json_object_dotset_string(obj, NULL, ""));
  ASSERT_EQ(JSONFailure,
            json_object_dotset_string(obj, "favorites.color", NULL));
  ASSERT_EQ(JSONFailure, json_object_dotset_string(obj, NULL, NULL));
  ASSERT_EQ(JSONFailure, json_object_dotset_value(obj, NULL, NULL));

  ASSERT_EQ(JSONFailure, json_array_append_string(NULL, "lorem"));
  ASSERT_EQ(JSONFailure, json_array_append_value(interests_arr, NULL));
  ASSERT_EQ(JSONFailure, json_array_append_value(NULL, NULL));

  ASSERT_EQ(JSONFailure, json_array_remove(NULL, 0));
  ASSERT_EQ(JSONFailure, json_array_replace_value(interests_arr, 0, NULL));
  ASSERT_EQ(JSONFailure, json_array_replace_string(NULL, 0, "lorem"));
  ASSERT_EQ(JSONFailure,
            json_array_replace_string(interests_arr, 100, "not existing"));

  ASSERT_EQ(JSONFailure, json_array_append_string(
                             json_object_get_array(obj, "interests"), NULL));

  ASSERT_EQ(JSONSuccess, json_array_append_string(interests_arr, "Writing"));
  ASSERT_EQ(JSONSuccess, json_array_remove(interests_arr, 0));
  ASSERT_EQ(JSONSuccess, json_array_remove(interests_arr, 1));
  ASSERT_EQ(JSONSuccess, json_array_remove(interests_arr, 0));
  ASSERT_EQ(JSONFailure, json_array_remove(interests_arr, 0));

  val_with_parent = json_value_init_null();
  ASSERT_EQ(JSONSuccess, json_object_set_value(obj, "x", val_with_parent));
  ASSERT_EQ(JSONFailure, json_object_set_value(obj, "x", val_with_parent));

  val_with_parent = json_value_init_null();
  ASSERT_EQ(JSONSuccess,
            json_array_append_value(interests_arr, val_with_parent));
  ASSERT_EQ(JSONFailure,
            json_array_append_value(interests_arr, val_with_parent));

  val_with_parent = json_value_init_null();
  ASSERT_EQ(JSONSuccess,
            json_array_replace_value(interests_arr, 0, val_with_parent));
  ASSERT_EQ(JSONFailure,
            json_array_replace_value(interests_arr, 0, val_with_parent));

  ASSERT_EQ(JSONSuccess, json_object_remove(obj, "interests"));

  /* UTF-8 sequence validation tests */
  ASSERT_EQ(JSONSuccess,
            json_object_set_string(obj, "correct string", "κόσμε"));
  ASSERT_EQ(JSONSuccess,
            json_object_set_string(obj, "boundary 1", "\xed\x9f\xbf"));
  ASSERT_EQ(JSONSuccess,
            json_object_set_string(obj, "boundary 2", "\xee\x80\x80"));
  ASSERT_EQ(JSONSuccess,
            json_object_set_string(obj, "boundary 3", "\xef\xbf\xbd"));
  ASSERT_EQ(JSONSuccess,
            json_object_set_string(obj, "boundary 4", "\xf4\x8f\xbf\xbf"));

  ASSERT_EQ(JSONFailure,
            json_object_set_string(obj, "first continuation byte", "\x80"));
  ASSERT_EQ(JSONFailure,
            json_object_set_string(obj, "last continuation byte", "\xbf"));
  ASSERT_EQ(JSONFailure,
            json_object_set_string(obj, "impossible sequence 1", "\xfe"));
  ASSERT_EQ(JSONFailure,
            json_object_set_string(obj, "impossible sequence 2", "\xff"));
  ASSERT_EQ(JSONFailure, json_object_set_string(obj, "impossible sequence 3",
                                                "\xfe\xfe\xff\xff"));
  ASSERT_EQ(JSONFailure, json_object_set_string(obj, "overlong 1", "\xc0\xaf"));
  ASSERT_EQ(JSONFailure, json_object_set_string(obj, "overlong 2", "\xc1\xbf"));
  ASSERT_EQ(JSONFailure,
            json_object_set_string(obj, "overlong 3", "\xe0\x80\xaf"));
  ASSERT_EQ(JSONFailure,
            json_object_set_string(obj, "overlong 4", "\xe0\x9f\xbf"));
  ASSERT_EQ(JSONFailure,
            json_object_set_string(obj, "overlong 5", "\xf0\x80\x80\xaf"));
  ASSERT_EQ(JSONFailure,
            json_object_set_string(obj, "overlong 6", "\xf0\x8f\xbf\xbf"));
  ASSERT_EQ(JSONFailure,
            json_object_set_string(obj, "overlong null 1", "\xc0\x80"));
  ASSERT_EQ(JSONFailure,
            json_object_set_string(obj, "overlong null 2", "\xe0\x80\x80"));
  ASSERT_EQ(JSONFailure,
            json_object_set_string(obj, "overlong null 3", "\xf0\x80\x80\x80"));
  ASSERT_EQ(JSONFailure, json_object_set_string(obj, "overlong null 4",
                                                "\xf8\x80\x80\x80\x80"));
  ASSERT_EQ(JSONFailure, json_object_set_string(obj, "overlong null 5",
                                                "\xfc\x80\x80\x80\x80\x80"));
  ASSERT_EQ(JSONFailure,
            json_object_set_string(obj, "single surrogate 1", "\xed\xa0\x80"));
  ASSERT_EQ(JSONFailure,
            json_object_set_string(obj, "single surrogate 2", "\xed\xaf\xbf"));
  ASSERT_EQ(JSONFailure,
            json_object_set_string(obj, "single surrogate 3", "\xed\xbf\xbf"));
  /* Truncated multi-byte UTF-8 sequences */
  ASSERT_EQ(JSONFailure,
            json_object_set_string_with_len(obj, "trunc2", "\xc2", 1));
  ASSERT_EQ(JSONFailure,
            json_object_set_string_with_len(obj, "trunc3", "\xe2\x82", 2));
  ASSERT_EQ(JSONFailure,
            json_object_set_string_with_len(obj, "trunc4", "\xf0\x90\x8d", 3));
  ASSERT_EQ(JSONFailure,
            json_object_set_string(obj, "above max cp", "\xf4\x90\x80\x80"));

  /* Order-preserving array removal */
  remove_test_val = json_parse_string("[1, 2, 3, 4, 5]");
  remove_test_arr = json_array(remove_test_val);
  ASSERT_EQ(JSONSuccess, json_array_remove(remove_test_arr, 2));
  tmp = json_parse_string("[1, 2, 4, 5]");
  ASSERT(json_value_equals(remove_test_val, tmp));
  json_value_free(tmp);

  ASSERT_EQ(JSONSuccess, json_array_remove(remove_test_arr, 0));
  tmp = json_parse_string("[2, 4, 5]");
  ASSERT(json_value_equals(remove_test_val, tmp));
  json_value_free(tmp);

  ASSERT_EQ(JSONSuccess, json_array_remove(remove_test_arr, 2));
  tmp = json_parse_string("[2, 4]");
  ASSERT(json_value_equals(remove_test_val, tmp));
  json_value_free(tmp);

  ASSERT_EQ(JSONFailure, json_object_set_number(obj, "num", parson_test_nan()));
  ASSERT_EQ(JSONFailure,
            json_object_set_number(obj, "num", parson_test_pos_inf()));

  json_value_free(val_from_file);
  json_value_free(val);
  json_value_free(remove_test_val);
  PASS();
}

/**
 * @brief Test json_value_equals inequality detection on mutated values
 * @return greatest_test_res
 */
TEST test_suite_6(void) {
  JSON_Value *a = json_parse_file(get_file_path("test_2.txt"));
  JSON_Value *b = json_parse_file(get_file_path("test_2.txt"));

  ASSERT(json_value_equals(a, b));
  json_object_set_string(json_object(a), "string", "eki");
  ASSERT_FALSE(json_value_equals(a, b));
  json_value_free(a);

  a = json_value_deep_copy(b);
  ASSERT(json_value_equals(a, b));
  json_array_append_number(
      json_object_get_array(json_object(b), "string array"), 1337);
  ASSERT_FALSE(json_value_equals(a, b));
  json_value_free(a);
  json_value_free(b);
  PASS();
}

/**
 * @brief Test schema validation
 * @return greatest_test_res
 */
TEST test_suite_7(void) {
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
  ASSERT_EQ(JSONSuccess, json_validate(schema, val_from_file));
  json_object_set_string(schema_obj, "age", "");
  ASSERT_EQ(JSONFailure, json_validate(schema, val_from_file));
  json_value_free(val_from_file);
  json_value_free(schema);
  PASS();
}

/**
 * @brief Test compact file serialization and size computation
 * @return greatest_test_res
 */
TEST test_suite_8(void) {
  const char *temp_path = NULL;
  JSON_Value *a = json_parse_file(get_file_path("test_2.txt"));
  JSON_Value *b = NULL;
  char *buf = NULL;
  size_t serialization_size = 0;

  ASSERT(a != NULL);
  temp_path = get_file_path("test_2_serialized.txt");
  ASSERT_EQ(JSONSuccess, json_serialize_to_file(a, temp_path));
  b = json_parse_file(temp_path);
  ASSERT(json_value_equals(a, b));
  remove(temp_path);

  serialization_size = json_serialization_size(a);
  buf = json_serialize_to_string(a);
  ASSERT(buf != NULL);
  ASSERT_EQ(serialization_size, strlen(buf) + 1);
  json_free_serialized_string(buf);
  json_value_free(a);
  json_value_free(b);
  PASS();
}

/**
 * @brief Test pretty-printed file serialization against expected golden output
 * @return greatest_test_res
 */
TEST test_suite_9(void) {
  const char *pretty_path = NULL;
  const char *temp_path = NULL;
  char *file_contents = NULL;
  char *serialized = NULL;
  JSON_Value *a = NULL;
  JSON_Value *b = NULL;
  size_t serialization_size = 0;

  pretty_path = get_file_path("test_2_pretty.txt");
  a = json_parse_file(pretty_path);
  ASSERT(a != NULL);

  temp_path = get_file_path("test_2_serialized_pretty.txt");
  ASSERT_EQ(JSONSuccess, json_serialize_to_file_pretty(a, temp_path));
  b = json_parse_file(temp_path);
  ASSERT(json_value_equals(a, b));
  remove(temp_path);

  serialization_size = json_serialization_size_pretty(a);
  serialized = json_serialize_to_string_pretty(a);
  ASSERT(serialized != NULL);
  ASSERT_EQ(serialization_size, strlen(serialized) + 1);

  pretty_path = get_file_path("test_2_pretty.txt");
  ASSERT_EQ(JSONSuccess, test_read_file(pretty_path, &file_contents));
  ASSERT(file_contents != NULL);
  {
    size_t len = strlen(file_contents);
    if (len > 0 && file_contents[len - 1] == '\n') {
      file_contents[len - 1] = '\0';
      if (len > 1 && file_contents[len - 2] == '\r') {
        file_contents[len - 2] = '\0';
      }
    }
  }
  ASSERT_STR_EQ(file_contents, serialized);

  free(file_contents);
  json_free_serialized_string(serialized);
  json_value_free(a);
  json_value_free(b);
  PASS();
}

/**
 * @brief Test zero memory leaks across file parse and serialize lifecycles
 * @return greatest_test_res
 */
TEST test_suite_10(void) {
  JSON_Value *val = NULL;
  char *serialized = NULL;

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

  ASSERT_EQ(0, g_malloc_count);
  PASS();
}

/**
 * @brief Test configurable forward-slash escaping
 * @return greatest_test_res
 */
TEST test_suite_11(void) {
  const char *array_with_slashes = "[\"a/b/c\"]";
  const char *array_with_escaped_slashes = "[\"a\\/b\\/c\"]";
  char *serialized = NULL;
  JSON_Value *value = json_parse_string(array_with_slashes);

  json_set_escape_slashes(1);
  serialized = json_serialize_to_string(value);
  ASSERT_STR_EQ(array_with_escaped_slashes, serialized);
  json_free_serialized_string(serialized);

  json_set_escape_slashes(0);
  serialized = json_serialize_to_string(value);
  ASSERT_STR_EQ(array_with_slashes, serialized);
  json_free_serialized_string(serialized);

  json_set_escape_slashes(1);
  serialized = json_serialize_to_string(value);
  ASSERT_STR_EQ(array_with_escaped_slashes, serialized);
  json_free_serialized_string(serialized);

  json_value_free(value);
  PASS();
}

/**
 * @brief Test validation, equality, and shorthand type-mismatch branches
 * @return greatest_test_res
 */
TEST test_coverage_gaps(void) {
  JSON_Value *val1 = NULL;
  JSON_Value *val2 = NULL;

  val1 = json_parse_string("{\"a\":1}");
  val2 = json_parse_string("[1]");
  ASSERT_EQ(JSONFailure, json_validate(val1, val2));
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("[]");
  val2 = json_parse_string("[1, \"two\", true]");
  ASSERT_EQ(JSONSuccess, json_validate(val1, val2));
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("[1]");
  val2 = json_parse_string("[\"two\"]");
  ASSERT_EQ(JSONFailure, json_validate(val1, val2));
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("{\"a\":1, \"b\":2}");
  val2 = json_parse_string("{\"a\":1}");
  ASSERT_EQ(JSONFailure, json_validate(val1, val2));
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("{\"a\":1, \"b\":2}");
  val2 = json_parse_string("{\"a\":1, \"c\":3}");
  ASSERT_EQ(JSONFailure, json_validate(val1, val2));
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("{}");
  val2 = json_parse_string("{\"a\":1}");
  ASSERT_EQ(JSONSuccess, json_validate(val1, val2));
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("1");
  val2 = json_parse_string("[1]");
  ASSERT_FALSE(json_value_equals(val1, val2));
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("[1, 2]");
  val2 = json_parse_string("[1]");
  ASSERT_FALSE(json_value_equals(val1, val2));
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("[1, 2]");
  val2 = json_parse_string("[1, 3]");
  ASSERT_FALSE(json_value_equals(val1, val2));
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("true");
  val2 = json_parse_string("false");
  ASSERT_FALSE(json_value_equals(val1, val2));
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("{\"a\":1}");
  val2 = json_parse_string("{\"b\":1}");
  ASSERT_FALSE(json_value_equals(val1, val2));
  json_value_free(val1);
  json_value_free(val2);

  val1 = json_parse_string("{\"a\":1}");
  val2 = json_parse_string("{\"a\":1, \"b\":2}");
  ASSERT_FALSE(json_value_equals(val1, val2));
  json_value_free(val1);
  json_value_free(val2);

  /* Key with embedded NUL in object must be rejected */
  ASSERT(json_parse_string("{\"a\\u0000b\": 1}") == NULL);

  val1 = json_parse_string("1");
  (void)json_type(val1);
  (void)json_object(val1);
  (void)json_array(val1);
  (void)json_string(val1);
  (void)json_string_len(val1);
  (void)json_number(val1);
  (void)json_boolean(val1);
  json_value_free(val1);

  PASS();
}

/**
 * @brief Test memory leak prevention on failed setters and truncated keys
 * @return greatest_test_res
 */
TEST test_memory_leaks(void) {
  g_malloc_count = 0;

  ASSERT_EQ(JSONFailure, json_object_set_string(NULL, "lorem", "ipsum"));
  ASSERT_EQ(JSONFailure, json_object_set_number(NULL, "lorem", 42));
  ASSERT_EQ(JSONFailure, json_object_set_boolean(NULL, "lorem", 0));
  ASSERT_EQ(JSONFailure, json_object_set_null(NULL, "lorem"));
  ASSERT(json_parse_string("{\"\\u0000\"") == NULL);

  ASSERT_EQ(0, g_malloc_count);
  PASS();
}

/**
 * @brief Exhaustive OOM injection across all public APIs verifying zero leaks
 * @return greatest_test_res
 */
TEST test_failing_allocations_for_all_apis(void) {
  int n = 0;

  json_set_allocation_functions(failing_malloc, failing_free);
  for (;;) {
    g_failing_alloc.allocation_to_fail = n;
    g_failing_alloc.alloc_count = 0;
    g_failing_alloc.total_count = 0;
    g_failing_alloc.has_failed = 0;
    g_failing_alloc.should_fail = 1;
    n++;

    exercise_all_apis();
    exercise_null_and_invalid_apis();

    if (g_failing_alloc.alloc_count != 0) {
      json_set_allocation_functions(counted_malloc, counted_free);
      FAIL();
    }
    if (!g_failing_alloc.has_failed) {
      break;
    }
  }
  json_set_allocation_functions(counted_malloc, counted_free);
  PASS();
}

/**
 * @brief Exhaustive OOM injection across file parsing, rehashing, and dotset
 * @return greatest_test_res
 */
TEST test_failing_allocations(void) {
  const char *filename = "test_2.txt";
  JSON_Value *root_value = NULL;
  JSON_Object *root_object = NULL;
  int i = 0;
  int n = 0;
  char key_val_buf[32];

  json_set_allocation_functions(failing_malloc, failing_free);
  for (;;) {
    g_failing_alloc.allocation_to_fail = n;
    g_failing_alloc.alloc_count = 0;
    g_failing_alloc.total_count = 0;
    g_failing_alloc.has_failed = 0;
    g_failing_alloc.should_fail = 1;
    n++;

    root_value = json_parse_file(get_file_path(filename));
    if (g_failing_alloc.has_failed) {
      if (root_value != NULL) {
        json_value_free(root_value);
        json_set_allocation_functions(counted_malloc, counted_free);
        FAIL();
      }
    }

    if (root_value != NULL) {
      root_object = json_object(root_value);
      for (i = 0; i < 64; i++) {
#if defined(_MSC_VER)
        sprintf_s(key_val_buf, sizeof(key_val_buf), "%d", i);
#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||            \
    (defined(__cplusplus) && __cplusplus >= 201103L)
        snprintf(key_val_buf, sizeof(key_val_buf), "%d", i);
#else
        sprintf(key_val_buf, "%d", i);
#endif
        json_object_set_string(root_object, key_val_buf, key_val_buf);
      }

      for (i = 0; i < 64; i++) {
#if defined(_MSC_VER)
        sprintf_s(key_val_buf, sizeof(key_val_buf), "%d", i);
#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||            \
    (defined(__cplusplus) && __cplusplus >= 201103L)
        snprintf(key_val_buf, sizeof(key_val_buf), "%d", i);
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
      json_set_allocation_functions(counted_malloc, counted_free);
      FAIL();
    }
    if (!g_failing_alloc.has_failed) {
      break;
    }
  }
  json_set_allocation_functions(counted_malloc, counted_free);
  PASS();
}

/**
 * @brief Test custom float format string setting and resetting
 * @return greatest_test_res
 */
TEST test_custom_number_format(void) {
  char *serialized = NULL;
  JSON_Value *val = NULL;

  g_malloc_count = 0;
  val = json_value_init_number(0.6);
  json_set_float_serialization_format("%.1f");
  serialized = json_serialize_to_string(val);
  json_set_float_serialization_format(NULL);
  ASSERT_STR_EQ("0.6", serialized);
  json_free_serialized_string(serialized);
  json_value_free(val);
  ASSERT_EQ(0, g_malloc_count);
  PASS();
}

/** @brief Flag tracking invocation of custom_serialization_func */
static int g_custom_serialization_func_called = 0;

/**
 * @brief Custom number serialization test callback
 * @param num Number to format
 * @param buf Destination buffer or NULL
 * @return Characters written
 */
static int custom_serialization_func(double num, char *buf) {
  char num_buf[32];
  g_custom_serialization_func_called = 1;
  if (buf == NULL) {
    buf = num_buf;
  }
#if defined(_MSC_VER)
  return sprintf_s(buf, 32, "%.1f", num);
#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||            \
    (defined(__cplusplus) && __cplusplus >= 201103L)
  return snprintf(buf, 32, "%.1f", num);
#else
  return sprintf(buf, "%.1f", num);
#endif
}

/**
 * @brief Test custom number serialization callback registration
 * @return greatest_test_res
 */
TEST test_custom_number_serialization_function(void) {
  char *serialized = NULL;
  JSON_Value *val = NULL;

  g_malloc_count = 0;
  g_custom_serialization_func_called = 0;
  val = json_value_init_number(0.6);
  json_set_number_serialization_function(custom_serialization_func);
  serialized = json_serialize_to_string(val);
  ASSERT_STR_EQ("0.6", serialized);
  ASSERT(g_custom_serialization_func_called != 0);
  json_set_number_serialization_function(NULL);
  json_free_serialized_string(serialized);
  json_value_free(val);
  ASSERT_EQ(0, g_malloc_count);
  PASS();
}

/**
 * @brief Test json_object_clear and default allocator wrappers
 * @return greatest_test_res
 */
TEST test_object_clear_and_default_allocators(void) {
  JSON_Value *val = NULL;
  JSON_Object *obj = NULL;

  /* Exercise default malloc/free wrappers before restoring counted hooks */
  json_set_allocation_functions(malloc, free);
#ifdef PARSON_SINGLE_HEADER
  json_set_allocation_functions(parson_default_malloc, parson_default_free);
#endif
  val = json_value_init_object();
  ASSERT(val != NULL);
  obj = json_value_get_object(val);
  ASSERT_EQ(JSONSuccess, json_object_set_string(obj, "foo", "bar"));
  ASSERT_EQ(JSONSuccess, json_object_clear(obj));
  ASSERT(json_object_get_value(obj, "foo") == NULL);
  json_value_free(val);

  json_set_allocation_functions(counted_malloc, counted_free);
  g_malloc_count = 0;
  val = json_value_init_object();
  obj = json_value_get_object(val);
  ASSERT_EQ(JSONSuccess, json_object_set_string(obj, "foo", "bar"));
  ASSERT_EQ(JSONSuccess, json_object_clear(obj));
  ASSERT(json_object_get_value(obj, "foo") == NULL);
  json_value_free(val);
  ASSERT_EQ(0, g_malloc_count);
  PASS();
}

/** @brief Primary parson greatest test suite */
SUITE(parson_suite) {
  RUN_TEST(test_suite_1);
  RUN_TEST(test_suite_2_no_comments);
  RUN_TEST(test_suite_2_with_comments);
  RUN_TEST(test_suite_3);
  RUN_TEST(test_suite_4);
  RUN_TEST(test_suite_5);
  RUN_TEST(test_suite_6);
  RUN_TEST(test_suite_7);
  RUN_TEST(test_suite_8);
  RUN_TEST(test_suite_9);
  RUN_TEST(test_suite_10);
  RUN_TEST(test_suite_11);
  RUN_TEST(test_c89_nan_inf);
  RUN_TEST(test_coverage_gaps);
  RUN_TEST(test_memory_leaks);
  RUN_TEST(test_file_parsing_failures);
  RUN_TEST(test_float_formats_and_serializer_errors);
  RUN_TEST(test_json_error_coverage);
  RUN_TEST(test_failing_allocations_for_all_apis);
  RUN_TEST(test_failing_allocations);
  RUN_TEST(test_custom_number_format);
  RUN_TEST(test_custom_number_serialization_function);
  RUN_TEST(test_object_clear_and_default_allocators);
}

/** @cond DOXYGEN_SUPPRESS */
GREATEST_MAIN_DEFS();
/** @endcond */

#ifdef TESTS_MAIN
/**
 * @brief Entry point for parson test runner
 * @param argc Argument count
 * @param argv Argument vector
 * @return 0 on success, non-zero on test failure
 */
int main(int argc, char *argv[]) {
#else
/**
 * @brief Library entry point for parson test runner when embedded
 * @param argc Argument count
 * @param argv Argument vector
 * @return 0 on success, non-zero on test failure
 */
int tests_main(int argc, char *argv[]);
int tests_main(int argc, char *argv[]) {
#endif
#if defined(__DOS__) || defined(MSDOS) || defined(_MSDOS) || defined(__MSDOS__)
  JSON_Status dos_rc = JSONFailure;
#endif
#ifdef __WATCOMC__
  _control87(_CW_DEFAULT, _MCW_EM | _MCW_PC | _MCW_RC | _MCW_IC);
#endif
  if (argc == 2 && argv[1][0] != '-') {
    g_tests_path = argv[1];
    argc = 1;
  }
#if defined(__DOS__) || defined(MSDOS) || defined(_MSDOS) || defined(__MSDOS__)
  dos_rc = dos_fixtures_setup();
  if (dos_rc != JSONSuccess) {
    return 1;
  }
#endif
  json_set_allocation_functions(counted_malloc, counted_free);
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(parson_suite);
#if defined(__DOS__) || defined(MSDOS) || defined(_MSDOS) || defined(__MSDOS__)
  dos_fixtures_cleanup();
#endif
  GREATEST_MAIN_END();
}
