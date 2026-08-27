/* clang-format off */
/**
 * \\file add_tests.c
 * \\brief Additional test suite for parson JSON library
 * \\author Krzysztof Gabis
 */

#include "../parson.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

const char *get_file_path(const char *filename);
extern int g_failing_file;
extern int g_tests_passed;
extern int g_tests_failed;
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

void test_file_parsing_failures(void) {
  JSON_Value *val;

  /* Test fseek failure */
  g_failing_file = 2;
  val = json_parse_file(get_file_path("test_2.txt"));
  TEST(val == NULL);
  if (val) {
    json_value_free(val);
    val = NULL;
  }
  val = json_parse_file_with_comments(get_file_path("test_2.txt"));
  TEST(val == NULL);
  if (val) {
    json_value_free(val);
    val = NULL;
  }

  /* Test ftell failure */
  g_failing_file = 1;
  val = json_parse_file(get_file_path("test_2.txt"));
  TEST(val == NULL);
  if (val) {
    json_value_free(val);
    val = NULL;
  }
  val = json_parse_file_with_comments(get_file_path("test_2.txt"));
  TEST(val == NULL);
  if (val) {
    json_value_free(val);
    val = NULL;
  }

  /* Test fread failure */
  g_failing_file = 3;
  val = json_parse_file(get_file_path("test_2.txt"));
  TEST(val == NULL);
  if (val) {
    json_value_free(val);
    val = NULL;
  }
  val = json_parse_file_with_comments(get_file_path("test_2.txt"));
  TEST(val == NULL);
  if (val) {
    json_value_free(val);
    val = NULL;
  }

  /* Test ferror failure */
  g_failing_file = 4;
  val = json_parse_file(get_file_path("test_2.txt"));
  TEST(val == NULL);
  if (val) {
    json_value_free(val);
    val = NULL;
  }
  val = json_parse_file_with_comments(get_file_path("test_2.txt"));
  TEST(val == NULL);
  if (val) {
    json_value_free(val);
    val = NULL;
  }

  g_failing_file = 0;
}

struct mock_json_value {
  void *parent;
  int type;
};

void test_json_error_coverage(void) {
  struct mock_json_value bad_val_mock = {NULL, JSONError};
  JSON_Value *bad_val = (JSON_Value *)&bad_val_mock;

  TEST(json_validate(bad_val, bad_val) == JSONFailure);
  TEST(json_value_equals(bad_val, bad_val) == 1);
}

/**
 * \\brief test_all_apis
 */
void test_all_apis(void) {
  JSON_Value *val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(val);
  JSON_Value *arr_val = json_value_init_array();
  JSON_Array *arr = json_value_get_array(arr_val);
  JSON_Value *schema = json_value_init_object();
  char *s = NULL;

  /* Array Appends */
  json_array_append_string_with_len(arr, "test", 4);
  json_array_append_boolean(arr, 1);
  json_array_append_null(arr);

  /* Array Replaces */
  json_array_replace_string_with_len(arr, 0, "replaced", 8);
  json_array_replace_number(arr, 1, 42.0);
  json_array_replace_boolean(arr, 2, 0);
  json_array_replace_null(arr, 2);

  /* Object dotsets */
  json_object_dotset_string_with_len(obj, "a.b", "test", 4);
  json_object_dotset_boolean(obj, "a.c", 1);
  json_object_dotset_null(obj, "a.d");

  /* Array/Object Getters */
  json_array_get_string_len(arr, 0);
  json_array_get_object(arr, 0);
  json_array_get_array(arr, 0);
  json_array_get_boolean(arr, 1);

  json_object_dotget_string_len(obj, "a.b");
  json_object_dotget_object(obj, "a");

  /* Values Getters */
  json_type(val);
  json_string_len(json_object_get_value(obj, "a"));
  json_number(json_array_get_value(arr, 1));
  json_boolean(json_object_dotget_value(obj, "a.c"));

  json_array_clear(arr);

  json_value_free(val);

  json_value_free(json_parse_string("[\"\\xFF\"]"));
  json_value_free(json_parse_string("[\"\\xED\\xA0\\x80\"]"));
  json_value_free(json_parse_string("[\"\\xF4\\x90\\x80\\x80\"]"));

  json_value_free(arr_val);

  /* Control characters serialization */
  val = json_value_init_string(
      "\x01\x02\x03\x04\x05\x06\x07\x0b\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17"
      "\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f");
  s = json_serialize_to_string(val);
  json_free_serialized_string(s);
  json_value_free(val);

  json_value_free(json_parse_string("[\"\\xFF\"]"));
  json_value_free(json_parse_string("[\"\\xED\\xA0\\x80\"]"));
  json_value_free(json_parse_string("[\"\\xF4\\x90\\x80\\x80\"]"));

  /* Missing branches in parse_utf16_hex etc. */
  json_parse_string_with_comments("/* unclosed comment ");
  json_parse_string_with_comments("// unclosed comment without newline");

  /* Serialize to file failures */
  val = json_value_init_object();
  json_serialize_to_file(val, "nonexistent_dir/file.json");
  json_serialize_to_file_pretty(val, "nonexistent_dir/file.json");
  json_serialize_to_file(val, "success_test.json");
  json_serialize_to_file_pretty(val, "success_test_pretty.json");
  json_value_free(val);

  json_value_free(json_parse_string("[\"\\xFF\"]"));
  json_value_free(json_parse_string("[\"\\xED\\xA0\\x80\"]"));
  json_value_free(json_parse_string("[\"\\xF4\\x90\\x80\\x80\"]"));

  /* Validation testing */
  val = json_value_init_object();
  json_object_set_number(json_object(val), "num", 1);
  json_object_set_string(json_object(schema), "num", "string");
  json_validate(schema, val);
  json_value_free(val);

  json_value_free(json_parse_string("[\"\\xFF\"]"));
  json_value_free(json_parse_string("[\"\\xED\\xA0\\x80\"]"));
  json_value_free(json_parse_string("[\"\\xF4\\x90\\x80\\x80\"]"));

  json_value_free(schema);

  /* Array replacement failure */
  arr_val = json_value_init_array();
  arr = json_value_get_array(arr_val);
  json_array_replace_string_with_len(arr, 0, "test", 4);
  json_array_replace_number(arr, 0, 1.0);
  json_array_replace_boolean(arr, 0, 1);
  json_array_replace_null(arr, 0);
  json_array_remove(arr, 0);
  json_value_free(arr_val);

  json_parse_file("nonexistent.json");
  json_parse_file_with_comments("nonexistent.json");
}

/**
 * \\brief test_null_and_invalid_apis
 */
void test_null_and_invalid_apis(void) {
  JSON_Value *val = json_parse_string("{\"a\":1, \"b\":[1,2]}");
  JSON_Object *obj = json_value_get_object(val);
  JSON_Array *arr = json_value_get_array(json_object_get_value(obj, "b"));
  char buf[10];

  json_serialize_to_buffer(val, buf, 1);
  json_serialize_to_buffer_pretty(val, buf, 1);
  json_serialize_to_buffer(NULL, buf, 10);
  json_serialize_to_buffer_pretty(NULL, buf, 10);
  json_serialize_to_file(NULL, "test.json");
  json_serialize_to_file_pretty(NULL, "test.json");
  json_serialize_to_string(NULL);
  json_serialize_to_string_pretty(NULL);
  json_serialization_size(NULL);
  json_serialization_size_pretty(NULL);

  json_object_set_string(NULL, "a", "b");
  json_object_set_string_with_len(NULL, "a", "b", 1);
  json_object_set_number(NULL, "a", 1);
  json_object_set_boolean(NULL, "a", 1);
  json_object_set_null(NULL, "a");

  json_object_dotset_value(NULL, "a", NULL);
  json_object_dotset_string(NULL, "a", "b");
  json_object_dotset_string_with_len(NULL, "a", "b", 1);
  json_object_dotset_number(NULL, "a", 1);
  json_object_dotset_boolean(NULL, "a", 1);
  json_object_dotset_null(NULL, "a");

  json_object_remove(NULL, "a");
  json_object_dotremove(NULL, "a");
  json_object_clear(NULL);

  json_array_remove(NULL, 0);
  json_array_remove(arr, 999);
  json_array_replace_value(NULL, 0, NULL);

  json_array_replace_string(NULL, 0, "a");
  json_array_replace_string_with_len(NULL, 0, "a", 1);
  json_array_replace_number(NULL, 0, 1);
  json_array_replace_boolean(NULL, 0, 1);
  json_array_replace_null(NULL, 0);

  json_array_clear(NULL);

  json_array_append_value(NULL, NULL);
  json_array_append_string(NULL, "a");
  json_array_append_string_with_len(NULL, "a", 1);
  json_array_append_number(NULL, 1);
  json_array_append_boolean(NULL, 1);
  json_array_append_null(NULL);

  json_value_deep_copy(NULL);
  json_value_free(NULL);

  json_value_get_type(NULL);
  json_value_get_object(NULL);
  json_value_get_array(NULL);
  json_value_get_string(NULL);
  json_value_get_string_len(NULL);
  json_value_get_number(NULL);
  json_value_get_boolean(NULL);
  json_value_get_parent(NULL);

  json_type(NULL);
  json_object(NULL);
  json_array(NULL);
  json_string(NULL);
  json_string_len(NULL);
  json_number(NULL);
  json_boolean(NULL);

  json_validate(NULL, NULL);
  json_validate(val, NULL);
  json_validate(NULL, val);

  json_value_free(val);

  json_value_free(json_parse_string("[\"\\xFF\"]"));
  json_value_free(json_parse_string("[\"\\xED\\xA0\\x80\"]"));
  json_value_free(json_parse_string("[\"\\xF4\\x90\\x80\\x80\"]"));
}
