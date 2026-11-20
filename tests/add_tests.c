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
 * @file add_tests.c
 * @brief Additional coverage, file I/O failure, NaN/Inf, and white-box tests
 * @author Krzysztof Gabis
 */

#ifndef PARSON_ADD_TESTS_INCLUDED
/* clang-format off */
#include "../parson.h"
#include "greatest.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */
#endif

/** @brief Zero divisor used to construct IEEE-754 NaN and Infinity in C89 */
static volatile double g_parson_zero = 0.0;

/** @brief State counter for stateful number serialization failure test */
static int g_stateful_num_ser_calls = 0;

/**
 * @brief Stateful number serialization callback that fails on its second call
 * @param num Numeric value to serialize
 * @param buf Output buffer or NULL when sizing
 * @return Number of bytes on call 1, -1 on subsequent calls
 */
static int stateful_failing_num_serializer(double num, char *buf) {
  (void)num;
  g_stateful_num_ser_calls++;
  if (g_stateful_num_ser_calls == 1) {
    if (buf != NULL) {
      buf[0] = '0';
      buf[1] = '\0';
    }
    return 1;
  }
  return -1;
}

/**
 * @brief Number serialization callback that always fails
 * @param num Numeric value to serialize
 * @param buf Output buffer or NULL
 * @return Always -1
 */
static int always_failing_num_serializer(double num, char *buf) {
  (void)num;
  (void)buf;
  return -1;
}

/**
 * @brief Test C89 NaN, +Inf, and -Inf rejection across parsing and builders
 * @return greatest_test_res
 */
TEST test_c89_nan_inf(void) {
  double nan_val = 0.0 / g_parson_zero;
  double pos_inf = 1.0 / g_parson_zero;
  double neg_inf = -1.0 / g_parson_zero;
  JSON_Value *obj_val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(obj_val);
  JSON_Value *arr_val = json_value_init_array();
  JSON_Array *arr = json_value_get_array(arr_val);

  ASSERT(obj_val != NULL);
  ASSERT(arr_val != NULL);
  ASSERT_EQ(JSONSuccess, json_array_append_number(arr, 1.0));

  /* Value initializers must reject NaN, +Inf, -Inf via C89 fallback */
  ASSERT(json_value_init_number(nan_val) == NULL);
  ASSERT(json_value_init_number(pos_inf) == NULL);
  ASSERT(json_value_init_number(neg_inf) == NULL);

  /* Object setters and dotsetters must reject NaN, +Inf, -Inf */
  ASSERT_EQ(JSONFailure, json_object_set_number(obj, "nan", nan_val));
  ASSERT_EQ(JSONFailure, json_object_set_number(obj, "pinf", pos_inf));
  ASSERT_EQ(JSONFailure, json_object_set_number(obj, "ninf", neg_inf));
  ASSERT_EQ(JSONFailure, json_object_dotset_number(obj, "a.nan", nan_val));
  ASSERT_EQ(JSONFailure, json_object_dotset_number(obj, "a.pinf", pos_inf));
  ASSERT_EQ(JSONFailure, json_object_dotset_number(obj, "a.ninf", neg_inf));

  /* Array append and replace must reject NaN, +Inf, -Inf */
  ASSERT_EQ(JSONFailure, json_array_append_number(arr, nan_val));
  ASSERT_EQ(JSONFailure, json_array_append_number(arr, pos_inf));
  ASSERT_EQ(JSONFailure, json_array_append_number(arr, neg_inf));
  ASSERT_EQ(JSONFailure, json_array_replace_number(arr, 0, nan_val));
  ASSERT_EQ(JSONFailure, json_array_replace_number(arr, 0, pos_inf));
  ASSERT_EQ(JSONFailure, json_array_replace_number(arr, 0, neg_inf));

  /* Parser must reject NaN/Infinity tokens and double overflow */
  ASSERT(json_parse_string("NaN") == NULL);
  ASSERT(json_parse_string("Infinity") == NULL);
  ASSERT(json_parse_string("-Infinity") == NULL);
  ASSERT(json_parse_string("[NaN]") == NULL);
  ASSERT(json_parse_string("[Infinity]") == NULL);
  ASSERT(json_parse_string("[-Infinity]") == NULL);
  ASSERT(json_parse_string("[1e500]") == NULL);
  ASSERT(json_parse_string("[-1e500]") == NULL);

  json_value_free(obj_val);
  json_value_free(arr_val);
  PASS();
}

/**
 * @brief Test simulated file stream I/O failures for parsing and serialization
 * @return greatest_test_res
 */
TEST test_file_parsing_failures(void) {
#ifdef PARSON_SINGLE_HEADER
  JSON_Value *val = NULL;
  JSON_Value *ser_val = NULL;
  const char *tmp_fputs = NULL;
  const char *tmp_fclose = NULL;

  /* fseek failure */
  g_failing_file = 2;
  val = json_parse_file(get_file_path("test_2.txt"));
  ASSERT(val == NULL);
  val = json_parse_file_with_comments(get_file_path("test_2.txt"));
  ASSERT(val == NULL);

  /* ftell failure */
  g_failing_file = 1;
  val = json_parse_file(get_file_path("test_2.txt"));
  ASSERT(val == NULL);
  val = json_parse_file_with_comments(get_file_path("test_2.txt"));
  ASSERT(val == NULL);

  /* fread failure */
  g_failing_file = 3;
  val = json_parse_file(get_file_path("test_2.txt"));
  ASSERT(val == NULL);
  val = json_parse_file_with_comments(get_file_path("test_2.txt"));
  ASSERT(val == NULL);

  /* ferror failure */
  g_failing_file = 4;
  val = json_parse_file(get_file_path("test_2.txt"));
  ASSERT(val == NULL);
  val = json_parse_file_with_comments(get_file_path("test_2.txt"));
  ASSERT(val == NULL);

  g_failing_file = 0;
  ser_val = json_parse_string("{\"k\":1}");
  ASSERT(ser_val != NULL);

  /* fputs failure */
  g_failing_file = 5;
  tmp_fputs = get_file_path("fputs_fail_tmp.json");
  ASSERT_EQ(JSONFailure, json_serialize_to_file(ser_val, tmp_fputs));
  remove(tmp_fputs);
  ASSERT_EQ(JSONFailure, json_serialize_to_file_pretty(ser_val, tmp_fputs));
  remove(tmp_fputs);

  /* fclose failure */
  g_failing_file = 6;
  tmp_fclose = get_file_path("fclose_fail_tmp.json");
  ASSERT_EQ(JSONFailure, json_serialize_to_file(ser_val, tmp_fclose));
  remove(tmp_fclose);
  ASSERT_EQ(JSONFailure, json_serialize_to_file_pretty(ser_val, tmp_fclose));
  remove(tmp_fclose);

  g_failing_file = 0;
  json_value_free(ser_val);
  PASS();
#else
  SKIP();
#endif
}

/**
 * @brief Test all float serialization format branches and custom serializer
 * errors
 * @return greatest_test_res
 */
TEST test_float_formats_and_serializer_errors(void) {
  static const char *const valid_formats[] = {"%f",   "%.2f", "%-8.2f", "%e",
                                              "%.2e", "%E",   "%.2E",   "%g",
                                              "%.3g", "%G",   "%.3G"};
  static const char *const invalid_formats[] = {
      "invalid", "%", "%100f", "%.100f", "%f.", "%x", "%80.2f"};
  size_t i = 0;
  JSON_Value *num_val = json_value_init_number(12.5);
  JSON_Value *huge_val = json_value_init_number(1e100);
  char *ser = NULL;
  char buf[64];

  ASSERT(num_val != NULL);
  ASSERT(huge_val != NULL);

  for (i = 0; i < sizeof(valid_formats) / sizeof(valid_formats[0]); i++) {
    json_set_float_serialization_format(valid_formats[i]);
    ser = json_serialize_to_string(num_val);
    ASSERT(ser != NULL);
    json_free_serialized_string(ser);
  }

  for (i = 0; i < sizeof(invalid_formats) / sizeof(invalid_formats[0]); i++) {
    json_set_float_serialization_format(invalid_formats[i]);
    ASSERT(json_serialize_to_string(num_val) == NULL);
    ASSERT(json_serialize_to_string_pretty(num_val) == NULL);
  }

  /* Formatted output exceeding PARSON_NUM_BUF_SIZE (64 bytes) */
  json_set_float_serialization_format("%.2f");
  ASSERT(json_serialize_to_string(huge_val) == NULL);
  json_set_float_serialization_format(NULL);

  /* Custom number serializer failing on first call vs second call */
  json_set_number_serialization_function(always_failing_num_serializer);
  ASSERT_EQ(0u, json_serialization_size(num_val));
  ASSERT_EQ(0u, json_serialization_size_pretty(num_val));

  json_set_number_serialization_function(stateful_failing_num_serializer);
  g_stateful_num_ser_calls = 0;
  ASSERT(json_serialize_to_string(num_val) == NULL);
  g_stateful_num_ser_calls = 0;
  ASSERT(json_serialize_to_string_pretty(num_val) == NULL);
  g_stateful_num_ser_calls = 0;
  ASSERT_EQ(JSONFailure, json_serialize_to_buffer(num_val, buf, sizeof(buf)));
  g_stateful_num_ser_calls = 0;
  ASSERT_EQ(JSONFailure,
            json_serialize_to_buffer_pretty(num_val, buf, sizeof(buf)));

  json_set_number_serialization_function(NULL);
  json_value_free(num_val);
  json_value_free(huge_val);
  PASS();
}

/** @brief Mock struct matching JSON_Value header layout for JSONError tests */
struct mock_json_value {
  /** @brief Parent pointer */
  void *parent;
  /** @brief Value type tag */
  int type;
};

/**
 * @brief Test JSONError and internal defensive error-percolation branches
 * @return greatest_test_res
 */
TEST test_json_error_coverage(void) {
  struct mock_json_value bad_val_mock = {NULL, JSONError};
  struct mock_json_value unk_val_mock = {NULL, 999};
  JSON_Value *bad_val = (JSON_Value *)&bad_val_mock;
  JSON_Value *unk_val = (JSON_Value *)&unk_val_mock;
  JSON_Value *bool_schema = json_value_init_boolean(1);
  JSON_Value *bool_val = json_value_init_boolean(0);
  JSON_Value *good_str_val = json_value_init_string("ok");
  JSON_Value *obj_wrap = json_value_init_object();
  JSON_Object *obj_real = json_value_get_object(obj_wrap);

  ASSERT_EQ(JSONFailure, json_validate(bad_val, bad_val));
  ASSERT_EQ(JSONFailure, json_validate(unk_val, unk_val));
  ASSERT_EQ(JSONSuccess, json_validate(bool_schema, bool_val));
  ASSERT_EQ(1, json_value_equals(bad_val, bad_val));
  ASSERT_EQ(1, json_value_equals(unk_val, unk_val));
  ASSERT(json_value_deep_copy(bad_val) == NULL);
  ASSERT(json_value_deep_copy(unk_val) == NULL);
  ASSERT_EQ(0u, json_serialization_size(bad_val));
  ASSERT_EQ(0u, json_serialization_size(unk_val));

  /* Lone minus sign, parse_utf16_hex NUL/bad-hex positions, trail surrogates */
  ASSERT(json_parse_string("-") == NULL);
  ASSERT(json_parse_string("[-]") == NULL);
  ASSERT(json_parse_string("[\"\\u\"]") == NULL);
  ASSERT(json_parse_string("[\"\\u0\"]") == NULL);
  ASSERT(json_parse_string("[\"\\u00\"]") == NULL);
  ASSERT(json_parse_string("[\"\\u000\"]") == NULL);
  ASSERT(json_parse_string("[\"\\uz000\"]") == NULL);
  ASSERT(json_parse_string("[\"\\u0z00\"]") == NULL);
  ASSERT(json_parse_string("[\"\\u00z0\"]") == NULL);
  ASSERT(json_parse_string("[\"\\u000z\"]") == NULL);
  ASSERT(json_parse_string("[\"\\uD800\\uzzzz\"]") == NULL);
  ASSERT(json_parse_string("[\"\\uD800\\u0041\"]") == NULL);
  ASSERT(json_parse_string("[\"\\uD800\\uE000\"]") == NULL);

  /* Missing key removal, empty-object removal, NULL key removal, dotset NULL */
  ASSERT_EQ(JSONFailure, json_object_remove(obj_real, "missing"));
  ASSERT_EQ(JSONFailure, json_object_remove(obj_real, NULL));
  ASSERT_EQ(JSONFailure, json_object_dotset_value(obj_real, "a.b", NULL));
  ASSERT_EQ(JSONSuccess, json_object_set_number(obj_real, "present", 1.0));
  ASSERT_EQ(JSONFailure, json_object_remove(obj_real, "missing"));

  /* Exercise wrap-around cluster deletion in json_object_remove_internal:
   * 'j' and 'z' both hash to bucket 15 mod 16; 'k' hashes to bucket 0 mod 16 */
  {
    JSON_Value *tmp_ws = NULL;
    tmp_ws = json_parse_string("{\"a\": 1, }");
    ASSERT(tmp_ws != NULL);
    json_value_free(tmp_ws);
    tmp_ws = json_parse_string("[1, ]");
    ASSERT(tmp_ws != NULL);
    json_value_free(tmp_ws);
  }
  ASSERT_EQ(JSONSuccess, json_object_clear(obj_real));
  ASSERT_EQ(JSONSuccess, json_object_set_number(obj_real, "j", 1.0));
  ASSERT_EQ(JSONSuccess, json_object_set_number(obj_real, "z", 2.0));
  ASSERT_EQ(JSONSuccess, json_object_set_number(obj_real, "k", 3.0));
  ASSERT_EQ(JSONSuccess, json_object_remove(obj_real, "j"));
  ASSERT_EQ(2.0, json_object_get_number(obj_real, "z"));
  ASSERT_EQ(3.0, json_object_get_number(obj_real, "k"));

  ASSERT_EQ(JSONSuccess, json_object_clear(obj_real));
  ASSERT_EQ(JSONSuccess, json_object_set_number(obj_real, "k", 3.0));
  ASSERT_EQ(JSONSuccess, json_object_set_number(obj_real, "i", 0.5));
  ASSERT_EQ(JSONSuccess, json_object_set_number(obj_real, "j", 1.0));
  ASSERT_EQ(JSONSuccess, json_object_set_number(obj_real, "z", 2.0));
  ASSERT_EQ(JSONSuccess, json_object_remove(obj_real, "i"));
  ASSERT_EQ(JSONSuccess, json_object_remove(obj_real, "j"));
  ASSERT_EQ(2.0, json_object_get_number(obj_real, "z"));
  ASSERT_EQ(3.0, json_object_get_number(obj_real, "k"));

#ifdef PARSON_SINGLE_HEADER
  {
    unsigned int hex_out = 0;
    JSON_Value bad_str_val;
    JSON_Value *out_val = NULL;
    const char *cursor = NULL;
    FILE *fp = NULL;
    size_t cells_buf[2];
    char k0_buf[3];
    char k1_buf[5];
    char k2_buf[3];
    char *names_buf[2];
    JSON_Value *vals_buf[2];
    size_t cell_ixs_buf[2];
    unsigned long hashes_buf[2];
    struct json_object_t fake_obj;
    JSON_Value fake_obj_val;
    parson_bool_t found = PARSON_FALSE;
    size_t cell_ix = 0;
    JSON_Value *dummy_num = json_value_init_number(1.0);

    k0_buf[0] = 'k';
    k0_buf[1] = '0';
    k0_buf[2] = '\0';
    k1_buf[0] = 'l';
    k1_buf[1] = 'o';
    k1_buf[2] = 'n';
    k1_buf[3] = 'g';
    k1_buf[4] = '\0';
    k2_buf[0] = 'k';
    k2_buf[1] = '2';
    k2_buf[2] = '\0';

    /* Hexadecimal number rejection in is_decimal */
    ASSERT_EQ(JSONFailure, is_decimal("1x2", 3));
    ASSERT_EQ(JSONFailure, is_decimal("0.X2", 4));
    ASSERT_EQ(JSONFailure, parse_utf16_hex("", &hex_out));
    ASSERT_EQ(JSONFailure, parse_utf16_hex("0", &hex_out));
    ASSERT_EQ(JSONFailure, parse_utf16_hex("00", &hex_out));
    ASSERT_EQ(JSONFailure, parse_utf16_hex("000", &hex_out));

    /* NULL / invalid args on parson_fopen */
    ASSERT_EQ(JSONFailure, parson_fopen(NULL, "r", &fp));
    ASSERT_EQ(JSONFailure, parson_fopen("x", NULL, &fp));
    ASSERT_EQ(JSONFailure, parson_fopen("x", "r", NULL));

    /* JSONString with NULL chars exercises json_value_get_string_desc &
     * json_serialize_string failure percolation */
    bad_str_val.parent = NULL;
    bad_str_val.type = JSONString;
    bad_str_val.value.string.chars = NULL;
    bad_str_val.value.string.length = 0;
    ASSERT(json_value_deep_copy(&bad_str_val) == NULL);
    ASSERT_EQ(0, json_value_equals(&bad_str_val, &bad_str_val));
    ASSERT_EQ(0, json_value_equals(good_str_val, &bad_str_val));
    ASSERT_EQ(0u, json_serialization_size(&bad_str_val));

    /* Non-matching start characters on internal parser functions */
    cursor = "abc";
    ASSERT_EQ(JSONFailure, skip_quotes(&cursor));
    cursor = "\"unterminated\\";
    ASSERT_EQ(JSONFailure, skip_quotes(&cursor));
    cursor = "abc";
    ASSERT_EQ(JSONFailure, parse_object_value(&cursor, 0, &out_val));
    cursor = "abc";
    ASSERT_EQ(JSONFailure, parse_array_value(&cursor, 0, &out_val));
    cursor = "txxx";
    ASSERT_EQ(JSONFailure, parse_boolean_value(&cursor, &out_val));
    cursor = "fxxx";
    ASSERT_EQ(JSONFailure, parse_boolean_value(&cursor, &out_val));
    cursor = "nxxx";
    ASSERT_EQ(JSONFailure, parse_null_value(&cursor, &out_val));

    /* json_array_resize & json_array_add invalid guards */
    ASSERT_EQ(JSONFailure, json_array_resize(NULL, 1));
    ASSERT_EQ(JSONFailure, json_array_add(NULL, dummy_num));
    {
      JSON_Value *arr_tmp = json_value_init_array();
      JSON_Array *arr_raw = json_value_get_array(arr_tmp);
      ASSERT_EQ(JSONFailure, json_array_resize(arr_raw, 0));
      ASSERT_EQ(JSONFailure, json_array_add(arr_raw, NULL));
      json_value_free(arr_tmp);
    }

    /* json_object_getn_value & json_object_get_cell_ix guards */
    ASSERT_EQ(JSONFailure, json_object_getn_value(NULL, "a", 1, &out_val));
    ASSERT_EQ(JSONFailure, json_object_getn_value(obj_real, NULL, 0, &out_val));
    memset(&fake_obj, 0, sizeof(fake_obj));
    ASSERT_EQ(JSONFailure,
              json_object_get_cell_ix(&fake_obj, "a", 1, 0, &found, &cell_ix));

    cells_buf[0] = 0;
    cells_buf[1] = 1;
    names_buf[0] = k0_buf;
    names_buf[1] = k1_buf;
    vals_buf[0] = dummy_num;
    vals_buf[1] = dummy_num;
    cell_ixs_buf[0] = 0;
    cell_ixs_buf[1] = 1;
    hashes_buf[0] = 0ul;
    hashes_buf[1] = 0ul;

    fake_obj.wrapping_value = &fake_obj_val;
    fake_obj.cells = cells_buf;
    fake_obj.names = names_buf;
    fake_obj.values = vals_buf;
    fake_obj.cell_ixs = cell_ixs_buf;
    fake_obj.hashes = hashes_buf;
    fake_obj.count = 1;
    fake_obj.item_capacity = 2;
    fake_obj.cell_capacity = 2;
    fake_obj_val.parent = NULL;
    fake_obj_val.type = JSONObject;
    fake_obj_val.value.object = &fake_obj;

    ASSERT_EQ(JSONFailure,
              json_object_get_cell_ix(&fake_obj, "k2", 2, 0, &found, &cell_ix));
    ASSERT_EQ(JSONFailure,
              json_object_getn_value(&fake_obj, "k2", 2, &out_val));
    ASSERT_EQ(JSONFailure, json_object_remove(&fake_obj, "k2"));
    ASSERT_EQ(JSONFailure, json_object_set_value(&fake_obj, "k2", dummy_num));
    ASSERT_EQ(JSONFailure, json_object_add(&fake_obj, k2_buf, dummy_num));
    ASSERT_EQ(JSONFailure, json_object_add(&fake_obj, NULL, dummy_num));
    ASSERT_EQ(JSONFailure, json_object_add(&fake_obj, k2_buf, NULL));
    ASSERT_EQ(JSONFailure, json_object_add(NULL, NULL, NULL));

    /* Duplicate key in json_object_grow_and_rehash triggers cleanup & failure
     */
    {
      struct json_object_t dup_obj;
      char *dup_names[2];
      JSON_Value *dup_vals[2];
      unsigned long dup_hashes[2];

      memset(&dup_obj, 0, sizeof(dup_obj));
      dup_names[0] = k0_buf;
      dup_names[1] = k0_buf;
      dup_vals[0] = dummy_num;
      dup_vals[1] = dummy_num;
      dup_hashes[0] = hash_string(k0_buf, 2);
      dup_hashes[1] = hash_string(k0_buf, 2);
      dup_obj.wrapping_value = &fake_obj_val;
      dup_obj.names = dup_names;
      dup_obj.values = dup_vals;
      dup_obj.hashes = dup_hashes;
      dup_obj.count = 2;
      dup_obj.item_capacity = 2;
      dup_obj.cell_capacity = 2;
      ASSERT_EQ(JSONFailure, json_object_grow_and_rehash(&dup_obj));
    }

    /* Pre-rehash full-table failure in json_object_add when cell_capacity > 0
     * and count == 0 < item_capacity */
    fake_obj.count = 0;
    ASSERT_EQ(JSONFailure, json_object_add(&fake_obj, k2_buf, dummy_num));
    fake_obj.count = 1;

    /* Serialize object with NULL key name triggers json_serialize_string NULL
     * error percolation inside case JSONObject */
    names_buf[0] = NULL;
    ASSERT_EQ(0u, json_serialization_size(&fake_obj_val));
    ASSERT_EQ(0u, json_serialization_size_pretty(&fake_obj_val));

    /* Serialize array & object containing invalid child exercises recursive
     * error percolation in json_serialize_to_buffer_r */
    names_buf[0] = k0_buf;
    vals_buf[0] = bad_val;
    ASSERT_EQ(0u, json_serialization_size(&fake_obj_val));
    {
      JSON_Value *arr_wrap = json_value_init_array();
      JSON_Array *arr_p = json_value_get_array(arr_wrap);
      ASSERT_EQ(JSONSuccess, json_array_append_number(arr_p, 1.0));
      json_value_free(arr_p->items[0]);
      arr_p->items[0] = bad_val;
      ASSERT_EQ(0u, json_serialization_size(arr_wrap));
      arr_p->count = 0;
      json_value_free(arr_wrap);
    }

    json_value_free(dummy_num);
  }
#endif

  json_value_free(bool_schema);
  json_value_free(bool_val);
  json_value_free(good_str_val);
  json_value_free(obj_wrap);
  PASS();
}

/**
 * @brief Exercise all public APIs across normal inputs (also used under OOM)
 */
static void exercise_all_apis(void) {
  JSON_Value *val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(val);
  JSON_Value *arr_val = json_value_init_array();
  JSON_Array *arr = json_value_get_array(arr_val);
  JSON_Value *schema = json_value_init_object();
  JSON_Value *copied = NULL;
  const char *tmp_ok = NULL;
  const char *tmp_pretty = NULL;
  char *s = NULL;

  /* Array Appends */
  json_array_append_string_with_len(arr, "test", 4);
  json_array_append_boolean(arr, 1);
  json_array_append_null(arr);
  json_array_append_number(arr, 3.5);

  /* Array Replaces */
  json_array_replace_string_with_len(arr, 0, "replaced", 8);
  json_array_replace_number(arr, 1, 42.0);
  json_array_replace_boolean(arr, 2, 0);
  json_array_replace_null(arr, 2);

  /* Object dotsets */
  json_object_dotset_string_with_len(obj, "a.b", "test", 4);
  json_object_dotset_boolean(obj, "a.c", 1);
  json_object_dotset_null(obj, "a.d");
  json_object_dotset_number(obj, "a.e", 9.0);
  json_object_set_string_with_len(obj, "s_len", "hello", 5);
  json_object_set_null(obj, "n_key");

  /* Deep copy of populated object & array */
  copied = json_value_deep_copy(val);
  json_value_free(copied);
  copied = json_value_deep_copy(arr_val);
  json_value_free(copied);

  /* Array/Object Getters */
  (void)json_array_get_string_len(arr, 0);
  (void)json_array_get_object(arr, 0);
  (void)json_array_get_array(arr, 0);
  (void)json_array_get_boolean(arr, 1);

  (void)json_object_dotget_string_len(obj, "a.b");
  (void)json_object_dotget_object(obj, "a");
  (void)json_object_get_value_at(obj, 0);

  /* Values Getters */
  (void)json_type(val);
  (void)json_string_len(json_object_get_value(obj, "a"));
  (void)json_number(json_array_get_value(arr, 1));
  (void)json_boolean(json_object_dotget_value(obj, "a.c"));

  json_array_clear(arr);
  json_value_free(val);
  json_value_free(arr_val);

  /* Control characters serialization (0x00..0x1f) */
  val = json_value_init_string_with_len(
      "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
      "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f",
      32);
  s = json_serialize_to_string(val);
  json_free_serialized_string(s);
  s = json_serialize_to_string_pretty(val);
  json_free_serialized_string(s);
  copied = json_value_deep_copy(val);
  json_value_free(copied);
  json_value_free(val);

  /* Comments & escaped strings in comments */
  val = json_parse_string_with_comments(
      "/* block */ {\"a\": \"\\\"str\\\"\", \"b\": [1, 2]} // line\n");
  s = json_serialize_to_string_pretty(val);
  json_free_serialized_string(s);
  json_value_free(val);
  json_value_free(json_parse_string_with_comments("/* unclosed comment "));
  json_value_free(
      json_parse_string_with_comments("// unclosed comment without newline"));

  /* UTF-8 BOM parsing */
  val = json_parse_string("\xEF\xBB\xBF{\"bom\":true}");
  json_value_free(val);

  /* Float format setting */
  json_set_float_serialization_format("%.2f");
  json_set_float_serialization_format(NULL);

  /* Serialize to file failures & successes */
  val = json_value_init_object();
  json_serialize_to_file(val, "nonexistent_dir/file.json");
  json_serialize_to_file_pretty(val, "nonexistent_dir/file.json");
  tmp_ok = get_file_path("tmp_all_apis_ok.json");
  if (json_serialize_to_file(val, tmp_ok) == JSONSuccess) {
    remove(tmp_ok);
  }
  tmp_pretty = get_file_path("tmp_all_apis_pretty.json");
  if (json_serialize_to_file_pretty(val, tmp_pretty) == JSONSuccess) {
    remove(tmp_pretty);
  }
  json_value_free(val);

  /* Validation testing */
  val = json_value_init_object();
  json_object_set_number(json_object(val), "num", 1);
  json_object_set_string(json_object(schema), "num", "string");
  json_validate(schema, val);
  json_value_free(val);
  json_value_free(schema);

  /* Array replacement failure on empty array */
  arr_val = json_value_init_array();
  arr = json_value_get_array(arr_val);
  json_array_replace_string_with_len(arr, 0, "test", 4);
  json_array_replace_number(arr, 0, 1.0);
  json_array_replace_boolean(arr, 0, 1);
  json_array_replace_null(arr, 0);
  json_array_remove(arr, 0);
  json_value_free(arr_val);

  json_value_free(json_parse_file("nonexistent.json"));
  json_value_free(json_parse_file_with_comments("nonexistent.json"));
}

/**
 * @brief Exercise NULL and invalid parameter handling across all public APIs
 */
static void exercise_null_and_invalid_apis(void) {
  JSON_Value *val = json_parse_string("{\"a\":1, \"b\":[1,2]}");
  JSON_Object *obj = json_value_get_object(val);
  JSON_Array *arr = json_value_get_array(json_object_get_value(obj, "b"));
  char buf[10];

  json_serialize_to_buffer(val, buf, 1);
  json_serialize_to_buffer_pretty(val, buf, 1);
  json_serialize_to_buffer(val, NULL, 10);
  json_serialize_to_buffer_pretty(val, NULL, 10);
  json_serialize_to_buffer(NULL, buf, 10);
  json_serialize_to_buffer_pretty(NULL, buf, 10);
  json_serialize_to_file(NULL, "test.json");
  json_serialize_to_file_pretty(NULL, "test.json");
  json_serialize_to_file(val, NULL);
  json_serialize_to_file_pretty(val, NULL);
  json_serialize_to_string(NULL);
  json_serialize_to_string_pretty(NULL);
  (void)json_serialization_size(NULL);
  (void)json_serialization_size_pretty(NULL);

  (void)json_parse_file(NULL);
  (void)json_parse_file_with_comments(NULL);
  (void)json_parse_string(NULL);
  (void)json_parse_string_with_comments(NULL);

  (void)json_object_get_value(NULL, "a");
  (void)json_object_get_value(obj, NULL);
  (void)json_object_dotget_value(NULL, "a.b");
  (void)json_object_dotget_value(obj, NULL);
  (void)json_object_get_count(NULL);
  (void)json_object_get_name(NULL, 0);
  (void)json_object_get_name(obj, 999);
  (void)json_object_get_value_at(NULL, 0);
  (void)json_object_get_value_at(obj, 999);
  (void)json_object_get_wrapping_value(NULL);

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
  if (obj != NULL) {
    json_object_dotset_number(obj, "a.sub", 2);
  }

  json_object_remove(NULL, "a");
  json_object_dotremove(NULL, "a");
  if (obj != NULL) {
    json_object_dotremove(obj, "a.sub");
    json_object_dotremove(obj, NULL);
  }
  json_object_clear(NULL);

  (void)json_array_get_value(NULL, 0);
  (void)json_array_get_value(arr, 999);
  (void)json_array_get_count(NULL);
  (void)json_array_get_wrapping_value(NULL);

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

  (void)json_value_init_string(NULL);
  (void)json_value_init_string_with_len(NULL, 0);

  json_value_deep_copy(NULL);
  json_value_free(NULL);

  (void)json_value_get_type(NULL);
  (void)json_value_get_object(NULL);
  (void)json_value_get_array(NULL);
  (void)json_value_get_string(NULL);
  (void)json_value_get_string_len(NULL);
  (void)json_value_get_number(NULL);
  (void)json_value_get_boolean(NULL);
  (void)json_value_get_parent(NULL);

  (void)json_type(NULL);
  (void)json_object(NULL);
  (void)json_array(NULL);
  (void)json_string(NULL);
  (void)json_string_len(NULL);
  (void)json_number(NULL);
  (void)json_boolean(NULL);

  json_validate(NULL, NULL);
  json_validate(val, NULL);
  json_validate(NULL, val);

  json_value_free(val);
}
