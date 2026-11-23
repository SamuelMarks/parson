/*
 SPDX-License-Identifier: MIT

 Parson 1.5.3 (https://github.com/kgabis/parson)
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
 * \file parson.h
 * \brief Lightweight JSON library header file
 * \author Krzysztof Gabis
 */

#ifndef parson_parson_h
/** \brief Header guard */
#define parson_parson_h

/** \brief Major version of Parson */
#define PARSON_VERSION_MAJOR 1
/** \brief Minor version of Parson */
#define PARSON_VERSION_MINOR 5
/** \brief Patch version of Parson */
#define PARSON_VERSION_PATCH 3

/** \brief String representation of Parson version */
#define PARSON_VERSION_STRING "1.5.3"

/* clang-format off */
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef PARSON_API
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(PARSON_BUILD_SHARED)
/** \brief Symbol export attribute */
#define PARSON_API __declspec(dllexport)
#elif defined(PARSON_SHARED)
/** \brief Symbol import attribute */
#define PARSON_API __declspec(dllimport)
#else
/** \brief Default symbol visibility attribute */
#define PARSON_API
#endif
#elif defined(__GNUC__) && __GNUC__ >= 4
/** \brief Default visibility attribute for GCC/Clang */
#define PARSON_API __attribute__((visibility("default")))
#else
/** \brief Default symbol visibility attribute */
#define PARSON_API
#endif
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
/** \brief Format string for integer */
#define PARSON_NUM_FORMAT "%I64d"
#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||            \
    (defined(__cplusplus) && __cplusplus >= 201103L)
/** \brief Format string for integer */
#define PARSON_NUM_FORMAT "%lld"
#else
/** \brief Format string for integer */
#define PARSON_NUM_FORMAT "%ld"
#endif

/* Types and enums */
/** \brief Opaque JSON Object type */
typedef struct json_object_t JSON_Object;
/** \brief Opaque JSON Array type */
typedef struct json_array_t JSON_Array;
/** \brief Opaque JSON Value type */
typedef struct json_value_t JSON_Value;

/** \brief Enumeration of JSON value types */
enum json_value_type {
  /** \brief JSON error value */
  JSONError = -1,
  /** \brief JSON null value */
  JSONNull = 1,
  /** \brief JSON string value */
  JSONString = 2,
  /** \brief JSON number value */
  JSONNumber = 3,
  /** \brief JSON object value */
  JSONObject = 4,
  /** \brief JSON array value */
  JSONArray = 5,
  /** \brief JSON boolean value */
  JSONBoolean = 6
};
/** \brief Integer alias for JSON value type */
typedef int JSON_Value_Type;

/** \brief Status codes returned by Parson operations */
enum json_result_t {
  /** \brief Operation succeeded */
  JSONSuccess = 0,
  /** \brief Operation failed */
  JSONFailure = -1
};
/** \brief Integer alias for Parson status code */
typedef int JSON_Status;

/**
 * \brief Custom memory allocation callback type
 * \param size Number of bytes to allocate
 * \return Pointer to allocated memory or NULL on failure
 */
typedef void *(*JSON_Malloc_Function)(size_t size);

/**
 * \brief Custom memory deallocation callback type
 * \param ptr Pointer to memory block to free
 */
typedef void (*JSON_Free_Function)(void *ptr);

/**
 * \brief Custom number serialization callback type
 * \param num Numeric value to serialize
 * \param buf Destination buffer of at least PARSON_NUM_BUF_SIZE bytes, or NULL
 * \return Number of characters written (excluding NUL), or negative on failure
 */
typedef int (*JSON_Number_Serialization_Function)(double num, char *buf);

/**
 * \brief Set custom memory allocation and deallocation functions
 * \param malloc_fun Custom allocation function
 * \param free_fun Custom deallocation function
 */
PARSON_API void json_set_allocation_functions(JSON_Malloc_Function malloc_fun,
                                              JSON_Free_Function free_fun);

/**
 * \brief Configure whether forward slashes are escaped during serialization
 * \param escape_slashes Non-zero to escape slashes, 0 otherwise
 */
PARSON_API void json_set_escape_slashes(int escape_slashes);

/**
 * \brief Set printf-style format string used for serializing floating-point
 * numbers
 * \param format Format string (e.g. "%.2f"), or NULL to restore default
 */
PARSON_API void json_set_float_serialization_format(const char *format);

/**
 * \brief Set custom callback function used for serializing numbers
 * \param fun Custom serialization callback, or NULL to restore default
 */
PARSON_API void
json_set_number_serialization_function(JSON_Number_Serialization_Function fun);

/**
 * \brief Parse first JSON value in a file
 * \param filename Path to JSON file
 * \return Newly allocated JSON_Value, or NULL on error
 * \note The returned JSON_Value must be freed with json_value_free().
 */
PARSON_API JSON_Value *json_parse_file(const char *filename);

/**
 * \brief Parse first JSON value in a file while ignoring C/C++-style comments
 * \param filename Path to JSON file
 * \return Newly allocated JSON_Value, or NULL on error
 * \note The returned JSON_Value must be freed with json_value_free().
 */
PARSON_API JSON_Value *json_parse_file_with_comments(const char *filename);

/**
 * \brief Parse first JSON value in a NUL-terminated string
 * \param string Input JSON text
 * \return Newly allocated JSON_Value, or NULL on error
 * \note The returned JSON_Value must be freed with json_value_free().
 */
PARSON_API JSON_Value *json_parse_string(const char *string);

/**
 * \brief Parse first JSON value in a string while ignoring C/C++-style comments
 * \param string Input JSON text with optional comments
 * \return Newly allocated JSON_Value, or NULL on error
 * \note The returned JSON_Value must be freed with json_value_free().
 */
PARSON_API JSON_Value *json_parse_string_with_comments(const char *string);

/**
 * \brief Compute buffer size in bytes required for compact serialization
 * \param value Root JSON_Value to serialize
 * \return Required buffer size including NUL terminator, or 0 on error
 */
PARSON_API size_t json_serialization_size(const JSON_Value *value);

/**
 * \brief Serialize JSON_Value into caller-supplied buffer in compact form
 * \param value Root JSON_Value to serialize
 * \param buf Destination character buffer
 * \param buf_size_in_bytes Capacity of buf in bytes
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_serialize_to_buffer(const JSON_Value *value,
                                                char *buf,
                                                size_t buf_size_in_bytes);

/**
 * \brief Serialize JSON_Value to a file in compact form
 * \param value Root JSON_Value to serialize
 * \param filename Destination file path
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_serialize_to_file(const JSON_Value *value,
                                              const char *filename);

/**
 * \brief Serialize JSON_Value to a newly allocated compact JSON string
 * \param value Root JSON_Value to serialize
 * \return Newly allocated NUL-terminated string, or NULL on error
 * \note The returned string must be freed with json_free_serialized_string().
 */
PARSON_API char *json_serialize_to_string(const JSON_Value *value);

/**
 * \brief Compute buffer size in bytes required for pretty-printed serialization
 * \param value Root JSON_Value to serialize
 * \return Required buffer size including NUL terminator, or 0 on error
 */
PARSON_API size_t json_serialization_size_pretty(const JSON_Value *value);

/**
 * \brief Serialize JSON_Value into caller-supplied buffer with pretty
 * indentation
 * \param value Root JSON_Value to serialize
 * \param buf Destination character buffer
 * \param buf_size_in_bytes Capacity of buf in bytes
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_serialize_to_buffer_pretty(
    const JSON_Value *value, char *buf, size_t buf_size_in_bytes);

/**
 * \brief Serialize JSON_Value to a file with pretty indentation
 * \param value Root JSON_Value to serialize
 * \param filename Destination file path
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_serialize_to_file_pretty(const JSON_Value *value,
                                                     const char *filename);

/**
 * \brief Serialize JSON_Value to a newly allocated pretty-printed JSON string
 * \param value Root JSON_Value to serialize
 * \return Newly allocated NUL-terminated string, or NULL on error
 * \note The returned string must be freed with json_free_serialized_string().
 */
PARSON_API char *json_serialize_to_string_pretty(const JSON_Value *value);

/**
 * \brief Free string allocated by json_serialize_to_string or
 * json_serialize_to_string_pretty
 * \param string Serialized string to free
 */
PARSON_API void json_free_serialized_string(char *string);

/**
 * \brief Test whether two JSON_Values are deeply equal
 * \param a First JSON_Value
 * \param b Second JSON_Value
 * \return Non-zero (1) if equal, 0 otherwise
 */
PARSON_API int json_value_equals(const JSON_Value *a, const JSON_Value *b);

/**
 * \brief Validate a JSON_Value against an example-based structural schema
 * \param schema Schema JSON_Value exemplar
 * \param value Candidate JSON_Value to validate
 * \return JSONSuccess if valid, JSONFailure otherwise
 */
PARSON_API JSON_Status json_validate(const JSON_Value *schema,
                                     const JSON_Value *value);

/* JSON Object */
/**
 * \brief Retrieve a property value from a JSON_Object by exact key name
 * \param object Target JSON_Object
 * \param name Property key name
 * \return Borrowed JSON_Value pointer, or NULL if not found
 * \note The returned reference is owned by its parent. Do not free it manually.
 */
PARSON_API JSON_Value *json_object_get_value(const JSON_Object *object,
                                             const char *name);

/**
 * \brief Retrieve a string property from a JSON_Object by exact key name
 * \param object Target JSON_Object
 * \param name Property key name
 * \return Borrowed NUL-terminated string, or NULL if missing or not a string
 */
PARSON_API const char *json_object_get_string(const JSON_Object *object,
                                              const char *name);

/**
 * \brief Retrieve byte length of a string property in a JSON_Object
 * \param object Target JSON_Object
 * \param name Property key name
 * \return String length excluding NUL terminator, or 0 on error
 */
PARSON_API size_t json_object_get_string_len(const JSON_Object *object,
                                             const char *name);

/**
 * \brief Retrieve a nested JSON_Object property by exact key name
 * \param object Target JSON_Object
 * \param name Property key name
 * \return Borrowed JSON_Object pointer, or NULL if missing or not an object
 * \note The returned reference is owned by its parent. Do not free it manually.
 */
PARSON_API JSON_Object *json_object_get_object(const JSON_Object *object,
                                               const char *name);

/**
 * \brief Retrieve a nested JSON_Array property by exact key name
 * \param object Target JSON_Object
 * \param name Property key name
 * \return Borrowed JSON_Array pointer, or NULL if missing or not an array
 */
PARSON_API JSON_Array *json_object_get_array(const JSON_Object *object,
                                             const char *name);

/**
 * \brief Retrieve a numeric property from a JSON_Object by exact key name
 * \param object Target JSON_Object
 * \param name Property key name
 * \return Numeric value, or 0.0 if missing or not a number
 */
PARSON_API double json_object_get_number(const JSON_Object *object,
                                         const char *name);

/**
 * \brief Retrieve a boolean property from a JSON_Object by exact key name
 * \param object Target JSON_Object
 * \param name Property key name
 * \return 1 or 0 if boolean, or -1 if missing or not a boolean
 */
PARSON_API int json_object_get_boolean(const JSON_Object *object,
                                       const char *name);

/**
 * \brief Retrieve a nested value using dot-separated path notation
 * \param object Root JSON_Object
 * \param name Dot-separated key path (e.g. "a.b.c")
 * \return Borrowed JSON_Value pointer, or NULL if not found
 * \note The returned reference is owned by its parent. Do not free it manually.
 */
PARSON_API JSON_Value *json_object_dotget_value(const JSON_Object *object,
                                                const char *name);

/**
 * \brief Retrieve a nested string property using dot-separated path notation
 * \param object Root JSON_Object
 * \param name Dot-separated key path
 * \return Borrowed NUL-terminated string, or NULL on error
 */
PARSON_API const char *json_object_dotget_string(const JSON_Object *object,
                                                 const char *name);

/**
 * \brief Retrieve byte length of a nested string property using dot notation
 * \param object Root JSON_Object
 * \param name Dot-separated key path
 * \return String length excluding NUL terminator, or 0 on error
 */
PARSON_API size_t json_object_dotget_string_len(const JSON_Object *object,
                                                const char *name);

/**
 * \brief Retrieve a nested JSON_Object using dot-separated path notation
 * \param object Root JSON_Object
 * \param name Dot-separated key path
 * \return Borrowed JSON_Object pointer, or NULL on error
 * \note The returned reference is owned by its parent. Do not free it manually.
 */
PARSON_API JSON_Object *json_object_dotget_object(const JSON_Object *object,
                                                  const char *name);

/**
 * \brief Retrieve a nested JSON_Array using dot-separated path notation
 * \param object Root JSON_Object
 * \param name Dot-separated key path
 * \return Borrowed JSON_Array pointer, or NULL on error
 */
PARSON_API JSON_Array *json_object_dotget_array(const JSON_Object *object,
                                                const char *name);

/**
 * \brief Retrieve a nested numeric property using dot-separated path notation
 * \param object Root JSON_Object
 * \param name Dot-separated key path
 * \return Numeric value, or 0.0 on error
 */
PARSON_API double json_object_dotget_number(const JSON_Object *object,
                                            const char *name);

/**
 * \brief Retrieve a nested boolean property using dot-separated path notation
 * \param object Root JSON_Object
 * \param name Dot-separated key path
 * \return 1 or 0 if boolean, or -1 on error
 */
PARSON_API int json_object_dotget_boolean(const JSON_Object *object,
                                          const char *name);

/**
 * \brief Return number of key-value pairs stored in a JSON_Object
 * \param object Target JSON_Object
 * \return Number of key-value pairs, or 0 if object is NULL
 */
PARSON_API size_t json_object_get_count(const JSON_Object *object);

/**
 * \brief Retrieve key name at a given zero-based index in a JSON_Object
 * \param object Target JSON_Object
 * \param index Zero-based entry index
 * \return Borrowed key name string, or NULL if out of bounds
 */
PARSON_API const char *json_object_get_name(const JSON_Object *object,
                                            size_t index);

/**
 * \brief Retrieve JSON_Value at a given zero-based index in a JSON_Object
 * \param object Target JSON_Object
 * \param index Zero-based entry index
 * \return Borrowed JSON_Value pointer, or NULL if out of bounds
 */
PARSON_API JSON_Value *json_object_get_value_at(const JSON_Object *object,
                                                size_t index);

/**
 * \brief Retrieve the owning JSON_Value wrapper of a JSON_Object
 * \param object Target JSON_Object
 * \return Owning JSON_Value pointer, or NULL if object is NULL
 */
PARSON_API JSON_Value *
json_object_get_wrapping_value(const JSON_Object *object);

/**
 * \brief Check whether a JSON_Object contains a given key
 * \param object Target JSON_Object
 * \param name Property key name
 * \return 1 if key exists, 0 otherwise
 */
PARSON_API int json_object_has_value(const JSON_Object *object,
                                     const char *name);

/**
 * \brief Check whether a JSON_Object contains a key with a specific value type
 * \param object Target JSON_Object
 * \param name Property key name
 * \param type Expected JSON_Value_Type
 * \return 1 if key exists with matching type, 0 otherwise
 */
PARSON_API int json_object_has_value_of_type(const JSON_Object *object,
                                             const char *name,
                                             JSON_Value_Type type);

/**
 * \brief Check whether a nested key path exists using dot notation
 * \param object Root JSON_Object
 * \param name Dot-separated key path
 * \return 1 if nested key exists, 0 otherwise
 */
PARSON_API int json_object_dothas_value(const JSON_Object *object,
                                        const char *name);

/**
 * \brief Check whether a nested key path exists with a specific value type
 * \param object Root JSON_Object
 * \param name Dot-separated key path
 * \param type Expected JSON_Value_Type
 * \return 1 if nested key exists with matching type, 0 otherwise
 */
PARSON_API int json_object_dothas_value_of_type(const JSON_Object *object,
                                                const char *name,
                                                JSON_Value_Type type);

/**
 * \brief Insert or replace a key-value pair in a JSON_Object, taking ownership
 * of value
 * \param object Target JSON_Object
 * \param name Property key name
 * \param value Owned JSON_Value to insert
 * \return JSONSuccess on success, JSONFailure on error
 * \note This function takes ownership of the passed value on success.
 */
PARSON_API JSON_Status json_object_set_value(JSON_Object *object,
                                             const char *name,
                                             JSON_Value *value);

/**
 * \brief Set a NUL-terminated string property on a JSON_Object
 * \param object Target JSON_Object
 * \param name Property key name
 * \param string NUL-terminated UTF-8 string to copy
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_object_set_string(JSON_Object *object,
                                              const char *name,
                                              const char *string);

/**
 * \brief Set a string property of explicit byte length on a JSON_Object
 * \param object Target JSON_Object
 * \param name Property key name
 * \param string UTF-8 character buffer to copy
 * \param len Length of string in bytes (excluding NUL terminator)
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_object_set_string_with_len(JSON_Object *object,
                                                       const char *name,
                                                       const char *string,
                                                       size_t len);

/**
 * \brief Set a numeric property on a JSON_Object
 * \param object Target JSON_Object
 * \param name Property key name
 * \param number Finite numeric value
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_object_set_number(JSON_Object *object,
                                              const char *name, double number);

/**
 * \brief Set a boolean property on a JSON_Object
 * \param object Target JSON_Object
 * \param name Property key name
 * \param boolean Boolean value (non-zero for true, 0 for false)
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_object_set_boolean(JSON_Object *object,
                                               const char *name, int boolean);

/**
 * \brief Set a null property on a JSON_Object
 * \param object Target JSON_Object
 * \param name Property key name
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_object_set_null(JSON_Object *object,
                                            const char *name);

/**
 * \brief Set a nested value using dot notation, creating intermediate objects
 * as needed
 * \param object Root JSON_Object
 * \param name Dot-separated key path
 * \param value Owned JSON_Value to insert
 * \return JSONSuccess on success, JSONFailure on error
 * \note This function takes ownership of the passed value on success.
 */
PARSON_API JSON_Status json_object_dotset_value(JSON_Object *object,
                                                const char *name,
                                                JSON_Value *value);

/**
 * \brief Set a nested NUL-terminated string property using dot notation
 * \param object Root JSON_Object
 * \param name Dot-separated key path
 * \param string NUL-terminated UTF-8 string to copy
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_object_dotset_string(JSON_Object *object,
                                                 const char *name,
                                                 const char *string);

/**
 * \brief Set a nested string property of explicit length using dot notation
 * \param object Root JSON_Object
 * \param name Dot-separated key path
 * \param string UTF-8 character buffer to copy
 * \param len Length of string in bytes
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_object_dotset_string_with_len(JSON_Object *object,
                                                          const char *name,
                                                          const char *string,
                                                          size_t len);

/**
 * \brief Set a nested numeric property using dot notation
 * \param object Root JSON_Object
 * \param name Dot-separated key path
 * \param number Finite numeric value
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_object_dotset_number(JSON_Object *object,
                                                 const char *name,
                                                 double number);

/**
 * \brief Set a nested boolean property using dot notation
 * \param object Root JSON_Object
 * \param name Dot-separated key path
 * \param boolean Boolean value (non-zero for true, 0 for false)
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_object_dotset_boolean(JSON_Object *object,
                                                  const char *name,
                                                  int boolean);

/**
 * \brief Set a nested null property using dot notation
 * \param object Root JSON_Object
 * \param name Dot-separated key path
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_object_dotset_null(JSON_Object *object,
                                               const char *name);

/**
 * \brief Remove and free a key-value pair from a JSON_Object
 * \param object Target JSON_Object
 * \param name Key name to remove
 * \return JSONSuccess on success, JSONFailure if not found
 */
PARSON_API JSON_Status json_object_remove(JSON_Object *object,
                                          const char *name);

/**
 * \brief Remove and free a nested key-value pair using dot notation
 * \param object Root JSON_Object
 * \param name Dot-separated key path to remove
 * \return JSONSuccess on success, JSONFailure if not found
 */
PARSON_API JSON_Status json_object_dotremove(JSON_Object *object,
                                             const char *name);

/**
 * \brief Remove and free all key-value pairs in a JSON_Object
 * \param object Target JSON_Object
 * \return JSONSuccess on success, JSONFailure if object is NULL
 */
PARSON_API JSON_Status json_object_clear(JSON_Object *object);

/* JSON Array */
/**
 * \brief Retrieve element at zero-based index from a JSON_Array
 * \param array Target JSON_Array
 * \param index Zero-based element index
 * \return Borrowed JSON_Value pointer, or NULL if out of bounds
 * \note The returned reference is owned by its parent. Do not free it manually.
 */
PARSON_API JSON_Value *json_array_get_value(const JSON_Array *array,
                                            size_t index);

/**
 * \brief Retrieve string element at zero-based index from a JSON_Array
 * \param array Target JSON_Array
 * \param index Zero-based element index
 * \return Borrowed NUL-terminated string, or NULL on error
 */
PARSON_API const char *json_array_get_string(const JSON_Array *array,
                                             size_t index);

/**
 * \brief Retrieve byte length of string element at zero-based index in a
 * JSON_Array
 * \param array Target JSON_Array
 * \param index Zero-based element index
 * \return String length excluding NUL terminator, or 0 on error
 */
PARSON_API size_t json_array_get_string_len(const JSON_Array *array,
                                            size_t index);

/**
 * \brief Retrieve JSON_Object element at zero-based index from a JSON_Array
 * \param array Target JSON_Array
 * \param index Zero-based element index
 * \return Borrowed JSON_Object pointer, or NULL on error
 * \note The returned reference is owned by its parent. Do not free it manually.
 */
PARSON_API JSON_Object *json_array_get_object(const JSON_Array *array,
                                              size_t index);

/**
 * \brief Retrieve nested JSON_Array element at zero-based index from a
 * JSON_Array
 * \param array Target JSON_Array
 * \param index Zero-based element index
 * \return Borrowed JSON_Array pointer, or NULL on error
 */
PARSON_API JSON_Array *json_array_get_array(const JSON_Array *array,
                                            size_t index);

/**
 * \brief Retrieve numeric element at zero-based index from a JSON_Array
 * \param array Target JSON_Array
 * \param index Zero-based element index
 * \return Numeric value, or 0.0 on error
 */
PARSON_API double json_array_get_number(const JSON_Array *array, size_t index);

/**
 * \brief Retrieve boolean element at zero-based index from a JSON_Array
 * \param array Target JSON_Array
 * \param index Zero-based element index
 * \return 1 or 0 if boolean, or -1 on error
 */
PARSON_API int json_array_get_boolean(const JSON_Array *array, size_t index);

/**
 * \brief Return number of elements in a JSON_Array
 * \param array Target JSON_Array
 * \return Element count, or 0 if array is NULL
 */
PARSON_API size_t json_array_get_count(const JSON_Array *array);

/**
 * \brief Retrieve the owning JSON_Value wrapper of a JSON_Array
 * \param array Target JSON_Array
 * \return Owning JSON_Value pointer, or NULL if array is NULL
 */
PARSON_API JSON_Value *json_array_get_wrapping_value(const JSON_Array *array);

/**
 * \brief Remove and free element at zero-based index in a JSON_Array
 * \param array Target JSON_Array
 * \param ix Zero-based index to remove
 * \return JSONSuccess on success, JSONFailure if out of bounds
 */
PARSON_API JSON_Status json_array_remove(JSON_Array *array, size_t ix);

/**
 * \brief Replace element at zero-based index in a JSON_Array, taking ownership
 * of value
 * \param array Target JSON_Array
 * \param ix Zero-based index to replace
 * \param value Owned JSON_Value replacement
 * \return JSONSuccess on success, JSONFailure on error
 * \note This function takes ownership of the passed value on success.
 */
PARSON_API JSON_Status json_array_replace_value(JSON_Array *array, size_t ix,
                                                JSON_Value *value);

/**
 * \brief Replace element at zero-based index in a JSON_Array with a string
 * \param array Target JSON_Array
 * \param i Zero-based index to replace
 * \param string NUL-terminated UTF-8 string to copy
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_array_replace_string(JSON_Array *array, size_t i,
                                                 const char *string);

/**
 * \brief Replace element at zero-based index in a JSON_Array with a sized
 * string
 * \param array Target JSON_Array
 * \param i Zero-based index to replace
 * \param string UTF-8 character buffer to copy
 * \param len Length of string in bytes
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_array_replace_string_with_len(JSON_Array *array,
                                                          size_t i,
                                                          const char *string,
                                                          size_t len);

/**
 * \brief Replace element at zero-based index in a JSON_Array with a number
 * \param array Target JSON_Array
 * \param i Zero-based index to replace
 * \param number Finite numeric value
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_array_replace_number(JSON_Array *array, size_t i,
                                                 double number);

/**
 * \brief Replace element at zero-based index in a JSON_Array with a boolean
 * \param array Target JSON_Array
 * \param i Zero-based index to replace
 * \param boolean Boolean value (non-zero for true, 0 for false)
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_array_replace_boolean(JSON_Array *array, size_t i,
                                                  int boolean);

/**
 * \brief Replace element at zero-based index in a JSON_Array with null
 * \param array Target JSON_Array
 * \param i Zero-based index to replace
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_array_replace_null(JSON_Array *array, size_t i);

/**
 * \brief Remove and free all elements in a JSON_Array
 * \param array Target JSON_Array
 * \return JSONSuccess on success, JSONFailure if array is NULL
 */
PARSON_API JSON_Status json_array_clear(JSON_Array *array);

/**
 * \brief Append a JSON_Value to the end of a JSON_Array, taking ownership
 * \param array Target JSON_Array
 * \param value Owned JSON_Value to append
 * \return JSONSuccess on success, JSONFailure on error
 * \note This function takes ownership of the passed value on success.
 */
PARSON_API JSON_Status json_array_append_value(JSON_Array *array,
                                               JSON_Value *value);

/**
 * \brief Append a NUL-terminated string to a JSON_Array
 * \param array Target JSON_Array
 * \param string NUL-terminated UTF-8 string to copy
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_array_append_string(JSON_Array *array,
                                                const char *string);

/**
 * \brief Append a sized UTF-8 string to a JSON_Array
 * \param array Target JSON_Array
 * \param string UTF-8 character buffer to copy
 * \param len Length of string in bytes
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_array_append_string_with_len(JSON_Array *array,
                                                         const char *string,
                                                         size_t len);

/**
 * \brief Append a number to a JSON_Array
 * \param array Target JSON_Array
 * \param number Finite numeric value
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_array_append_number(JSON_Array *array,
                                                double number);

/**
 * \brief Append a boolean to a JSON_Array
 * \param array Target JSON_Array
 * \param boolean Boolean value (non-zero for true, 0 for false)
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_array_append_boolean(JSON_Array *array,
                                                 int boolean);

/**
 * \brief Append a null value to a JSON_Array
 * \param array Target JSON_Array
 * \return JSONSuccess on success, JSONFailure on error
 */
PARSON_API JSON_Status json_array_append_null(JSON_Array *array);

/* JSON Value */
/**
 * \brief Allocate and initialize a new empty JSONObject value
 * \return Newly allocated JSON_Value, or NULL on error
 * \note The returned JSON_Value must be freed with json_value_free().
 */
PARSON_API JSON_Value *json_value_init_object(void);

/**
 * \brief Allocate and initialize a new empty JSONArray value
 * \return Newly allocated JSON_Value, or NULL on error
 * \note The returned JSON_Value must be freed with json_value_free().
 */
PARSON_API JSON_Value *json_value_init_array(void);

/**
 * \brief Allocate and initialize a new JSONString value from a NUL-terminated
 * UTF-8 string
 * \param string NUL-terminated UTF-8 string to copy
 * \return Newly allocated JSON_Value, or NULL on error
 * \note The returned JSON_Value must be freed with json_value_free().
 */
PARSON_API JSON_Value *json_value_init_string(const char *string);

/**
 * \brief Allocate and initialize a new JSONString value with explicit byte
 * length
 * \param string UTF-8 character buffer to copy
 * \param length Length of string in bytes (excluding NUL terminator)
 * \return Newly allocated JSON_Value, or NULL on error
 * \note The returned JSON_Value must be freed with json_value_free().
 */
PARSON_API JSON_Value *json_value_init_string_with_len(const char *string,
                                                       size_t length);

/**
 * \brief Allocate and initialize a new JSONNumber value
 * \param number Finite numeric value
 * \return Newly allocated JSON_Value, or NULL if NaN/Inf or out of memory
 * \note The returned JSON_Value must be freed with json_value_free().
 */
PARSON_API JSON_Value *json_value_init_number(double number);

/**
 * \brief Allocate and initialize a new JSONBoolean value
 * \param boolean Boolean value (non-zero for true, 0 for false)
 * \return Newly allocated JSON_Value, or NULL on error
 * \note The returned JSON_Value must be freed with json_value_free().
 */
PARSON_API JSON_Value *json_value_init_boolean(int boolean);

/**
 * \brief Allocate and initialize a new JSONNull value
 * \return Newly allocated JSON_Value, or NULL on error
 * \note The returned JSON_Value must be freed with json_value_free().
 */
PARSON_API JSON_Value *json_value_init_null(void);

/**
 * \brief Create a deep copy of a JSON_Value hierarchy
 * \param value Source JSON_Value to clone
 * \return Newly allocated deep copy, or NULL on error
 * \note The returned JSON_Value must be freed with json_value_free().
 */
PARSON_API JSON_Value *json_value_deep_copy(const JSON_Value *value);

/**
 * \brief Free a JSON_Value and all child values owned by it
 * \param value JSON_Value to free (safe if NULL)
 */
PARSON_API void json_value_free(JSON_Value *value);

/**
 * \brief Return the type of a JSON_Value
 * \param value Target JSON_Value
 * \return JSON_Value_Type enum value, or JSONError if value is NULL
 */
PARSON_API JSON_Value_Type json_value_get_type(const JSON_Value *value);

/**
 * \brief Extract JSON_Object pointer from a JSONObject value
 * \param value Target JSON_Value
 * \return Borrowed JSON_Object pointer, or NULL if not a JSONObject
 */
PARSON_API JSON_Object *json_value_get_object(const JSON_Value *value);

/**
 * \brief Extract JSON_Array pointer from a JSONArray value
 * \param value Target JSON_Value
 * \return Borrowed JSON_Array pointer, or NULL if not a JSONArray
 */
PARSON_API JSON_Array *json_value_get_array(const JSON_Value *value);

/**
 * \brief Extract NUL-terminated string from a JSONString value
 * \param value Target JSON_Value
 * \return Borrowed string pointer, or NULL if not a JSONString
 */
PARSON_API const char *json_value_get_string(const JSON_Value *value);

/**
 * \brief Return byte length of a JSONString value
 * \param value Target JSON_Value
 * \return Length in bytes excluding NUL terminator, or 0 if not a JSONString
 */
PARSON_API size_t json_value_get_string_len(const JSON_Value *value);

/**
 * \brief Extract numeric value from a JSONNumber value
 * \param value Target JSON_Value
 * \return Numeric value, or 0.0 if not a JSONNumber
 */
PARSON_API double json_value_get_number(const JSON_Value *value);

/**
 * \brief Extract boolean value from a JSONBoolean value
 * \param value Target JSON_Value
 * \return 1 or 0 if JSONBoolean, or -1 otherwise
 */
PARSON_API int json_value_get_boolean(const JSON_Value *value);

/**
 * \brief Retrieve the parent container JSON_Value of a JSON_Value
 * \param value Target JSON_Value
 * \return Parent JSON_Value pointer, or NULL if root or NULL
 */
PARSON_API JSON_Value *json_value_get_parent(const JSON_Value *value);

/* Shorter aliases */
/**
 * \brief Shorthand alias for json_value_get_type
 * \param value Target JSON_Value
 * \return JSON_Value_Type enum value, or JSONError if value is NULL
 */
PARSON_API JSON_Value_Type json_type(const JSON_Value *value);

/**
 * \brief Shorthand alias for json_value_get_object
 * \param value Target JSON_Value
 * \return Borrowed JSON_Object pointer, or NULL if not a JSONObject
 */
PARSON_API JSON_Object *json_object(const JSON_Value *value);

/**
 * \brief Shorthand alias for json_value_get_array
 * \param value Target JSON_Value
 * \return Borrowed JSON_Array pointer, or NULL if not a JSONArray
 */
PARSON_API JSON_Array *json_array(const JSON_Value *value);

/**
 * \brief Shorthand alias for json_value_get_string
 * \param value Target JSON_Value
 * \return Borrowed string pointer, or NULL if not a JSONString
 */
PARSON_API const char *json_string(const JSON_Value *value);

/**
 * \brief Shorthand alias for json_value_get_string_len
 * \param value Target JSON_Value
 * \return Length in bytes excluding NUL terminator, or 0 if not a JSONString
 */
PARSON_API size_t json_string_len(const JSON_Value *value);

/**
 * \brief Shorthand alias for json_value_get_number
 * \param value Target JSON_Value
 * \return Numeric value, or 0.0 if not a JSONNumber
 */
PARSON_API double json_number(const JSON_Value *value);

/**
 * \brief Shorthand alias for json_value_get_boolean
 * \param value Target JSON_Value
 * \return 1 or 0 if JSONBoolean, or -1 otherwise
 */
PARSON_API int json_boolean(const JSON_Value *value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

#ifdef PARSON_IMPLEMENTATION
#ifndef PARSON_IMPLEMENTED
/** \brief Implementation guard */
#define PARSON_IMPLEMENTED

#ifndef LOG_DEBUG
/** \brief Macro for logging debug information */
#define LOG_DEBUG parson_log_debug
/**
 * \brief Format and write debug diagnostic message to stderr
 * \param format Printf-style format string
 */
#if defined(__GNUC__) || defined(__clang__)
static void parson_log_debug(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
#endif
static void parson_log_debug(const char *format, ...) {
#ifdef PARSON_DEBUG
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
#else
  (void)format;
#endif
}
#endif

/** \brief Major implementation version */
#define PARSON_IMPL_VERSION_MAJOR 1
/** \brief Minor implementation version */
#define PARSON_IMPL_VERSION_MINOR 5
/** \brief Patch implementation version */
#define PARSON_IMPL_VERSION_PATCH 3

#if (PARSON_VERSION_MAJOR != PARSON_IMPL_VERSION_MAJOR) ||                     \
    (PARSON_VERSION_MINOR != PARSON_IMPL_VERSION_MINOR) ||                     \
    (PARSON_VERSION_PATCH != PARSON_IMPL_VERSION_PATCH)
#error "parson version mismatch between parson.c and parson.h"
#endif

#ifndef PARSON_FSEEK
/** \brief Seek file stream wrapper macro */
#define PARSON_FSEEK fseek
#endif

#ifndef PARSON_FTELL
/** \brief Tell file stream position wrapper macro */
#define PARSON_FTELL ftell
#endif

#ifndef PARSON_FREAD
/** \brief Read file stream wrapper macro */
#define PARSON_FREAD fread
#endif

#ifndef PARSON_FERROR
/** \brief Check file stream error wrapper macro */
#define PARSON_FERROR ferror
#endif

#ifndef PARSON_FPUTS
/** \brief Write string to file stream wrapper macro */
#define PARSON_FPUTS fputs
#endif

#ifndef PARSON_FCLOSE
/** \brief Close file stream wrapper macro */
#define PARSON_FCLOSE fclose
#endif

/** \brief Initial capacity for JSON arrays and objects */
#define STARTING_CAPACITY 16
#ifndef MAX_NESTING
#if defined(__DOS__) || defined(MSDOS) || defined(_MSDOS) ||                   \
    defined(__MSDOS__) || defined(__I86__) || defined(M_I86) ||                \
    defined(_M_I86)
/** \brief Maximum allowed nesting depth for parsed JSON on DOS/16-bit targets
 */
#define MAX_NESTING 64
#else
/** \brief Maximum allowed nesting depth for parsed JSON */
#define MAX_NESTING 2048
#endif
#endif

#ifndef PARSON_DEFAULT_FLOAT_FORMAT
/** \brief Default float format */
#define PARSON_DEFAULT_FLOAT_FORMAT                                            \
  "%1.17g" /* do not increase precision without increasing NUM_BUF_SIZE */
#endif

#ifndef PARSON_NUM_BUF_SIZE
/** \brief Size of buffer for number serialization */
#define PARSON_NUM_BUF_SIZE                                                    \
  64 /* double printed with "%1.17g" shouldn't be longer than 25 bytes so      \
        let's be paranoid and use 64 */
#endif

#ifndef PARSON_INDENT_STR
/** \brief String used for indentation in pretty serialization */
#define PARSON_INDENT_STR "    "
#endif

/** \brief Macro to calculate the string length of a token literal */
#define SIZEOF_TOKEN(a) (sizeof(a) - 1)
/** \brief Advances the string pointer by one character */
#define SKIP_CHAR(str) ((*str)++)
/**
 * \brief Advance string cursor past all ASCII/locale whitespace characters
 * \param str Pointer to string cursor to advance
 */
static void skip_whitespaces(const char **str) {
  while ((isspace)((unsigned char)(**str))) {
    SKIP_CHAR(str);
  }
}
/** \brief Macro to skip whitespace characters */
#define SKIP_WHITESPACES(str) skip_whitespaces(str)
/** \brief Returns the maximum of two values */
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#undef malloc
#undef free

#if defined(__WATCOMC__)
/**
 * \brief Check if IEEE-754 little-endian double is NaN or Inf on OpenWatcom
 * \param x Double value to inspect
 * \return Non-zero when exponent bits are all 1s (NaN or +/-Inf), 0 otherwise
 */
static int parson_is_number_invalid(double x) {
  union {
    double d;
    unsigned short w[4];
  } u;
  u.d = x;
  return (u.w[3] & 0x7FF0u) == 0x7FF0u;
}
/** \brief Check if number is NaN or Inf on OpenWatcom */
#define IS_NUMBER_INVALID(x) parson_is_number_invalid((x))
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L &&              \
    defined(isnan) && defined(isinf) && !defined(__EMSCRIPTEN__)
/** \brief Check if number is NaN or Inf */
#define IS_NUMBER_INVALID(x) (isnan((x)) || isinf((x)))
#else
/** \brief Check if number is NaN or Inf fallback */
#define IS_NUMBER_INVALID(x) (((x) * 0.0) != 0.0)
#endif

/** \brief Represents an invalid or missing index in a JSON Object */
#define OBJECT_INVALID_IX (~(size_t)0)

/**
 * \brief Default malloc wrapper calling standard library malloc
 * \param size Number of bytes to allocate
 * \return Allocated memory block or NULL
 */
static void *parson_default_malloc(size_t size) { return malloc(size); }

/**
 * \brief Default free wrapper calling standard library free
 * \param ptr Memory block to free
 */
static void parson_default_free(void *ptr) { free(ptr); }

/** \brief Global malloc function for parson */
static JSON_Malloc_Function parson_malloc = parson_default_malloc;
/** \brief Global free function for parson */
static JSON_Free_Function parson_free = parson_default_free;

/** \brief Flag to determine if slashes should be escaped during serialization
 */
static int parson_escape_slashes = 1;

/** \brief Global format string for float serialization */
static char *parson_float_format = NULL;

/** \brief Custom number serialization function */
static JSON_Number_Serialization_Function parson_number_serialization_function =
    NULL;

/** \brief Check if byte is a UTF-8 continuation byte */
#define IS_CONT(b)                                                             \
  (((unsigned char)(b) & 0xC0) == 0x80) /* is utf-8 continuation byte */

/** \brief Internal boolean integer alias */
typedef int parson_bool_t;

/** \brief True boolean value */
#define PARSON_TRUE 1
/** \brief False boolean value */
#define PARSON_FALSE 0

/** \brief Length-prefixed UTF-8 string storage */
typedef struct json_string {
  /** \brief Pointer to the string character buffer */
  char *chars;
  /** \brief Length of the string in bytes */
  size_t length;
} JSON_String;

/* Type definitions */
/** \brief Tagged payload union of a JSON_Value */
typedef union json_value_value {
  /** \brief String value */
  JSON_String string;
  /** \brief Numeric value */
  double number;
  /** \brief Object value */
  JSON_Object *object;
  /** \brief Array value */
  JSON_Array *array;
  /** \brief Boolean value */
  int boolean;
  /** \brief Null value */
  int null;
} JSON_Value_Value;

/** \brief Concrete representation of a JSON value node */
struct json_value_t {
  /** \brief Pointer to the parent value */
  JSON_Value *parent;
  /** \brief Type of the JSON value */
  JSON_Value_Type type;
  /** \brief Union holding the actual value */
  JSON_Value_Value value;
};

/** \brief Concrete hash-table representation of a JSON object */
struct json_object_t {
  /** \brief Pointer to the wrapping JSON_Value */
  JSON_Value *wrapping_value;
  /** \brief Hash table cells */
  size_t *cells;
  /** \brief Array of string hashes */
  unsigned long *hashes;
  /** \brief Array of key names */
  char **names;
  /** \brief Array of JSON_Values */
  JSON_Value **values;
  /** \brief Array of cell indices */
  size_t *cell_ixs;
  /** \brief Number of items in the object */
  size_t count;
  /** \brief Capacity of items */
  size_t item_capacity;
  /** \brief Capacity of the hash table cells */
  size_t cell_capacity;
};

/** \brief Concrete dynamic-array representation of a JSON array */
struct json_array_t {
  /** \brief Pointer to the wrapping JSON_Value */
  JSON_Value *wrapping_value;
  /** \brief Array of JSON_Values */
  JSON_Value **items;
  /** \brief Number of items in the array */
  size_t count;
  /** \brief Total capacity of the array */
  size_t capacity;
};

/* Various */
/**
 * \brief Open file safely using Safe CRT on MSVC or standard C89 fopen
 * \param filename File path to open
 * \param mode File access mode
 * \param out_fp Output pointer for opened FILE stream
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t parson_fopen(const char *filename, const char *mode,
                                       FILE **out_fp);
/**
 * \brief Read entire file contents into newly allocated NUL-terminated buffer
 * \param filename Path to file
 * \param out_contents Output pointer for allocated string
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t read_file(const char *filename, char **out_contents);
/**
 * \brief Strip comments delimited by start_token and end_token in place
 * \param string Mutable string buffer
 * \param start_token Comment start delimiter
 * \param end_token Comment end delimiter
 */
static void remove_comments(char *string, const char *start_token,
                            const char *end_token);
/**
 * \brief Duplicate n bytes of a string into a NUL-terminated buffer
 * \param string Input buffer
 * \param n Number of bytes to copy
 * \param out_string Output pointer for allocated string
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t parson_strndup(const char *string, size_t n,
                                         char **out_string);
/**
 * \brief Duplicate a NUL-terminated string using Safe CRT on MSVC or strcpy
 * \param string Input NUL-terminated string
 * \param out_string Output pointer for allocated string
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t parson_strdup(const char *string, char **out_string);
/**
 * \brief Format a floating-point number safely without non-literal format
 * strings
 * \param buf Destination character buffer
 * \param buf_size Size of destination buffer in bytes
 * \param format Optional printf-style float format (NULL uses default)
 * \param num Floating-point value to format
 * \param out_written Output number of characters written
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t parson_format_double(char *buf, size_t buf_size,
                                               const char *format, double num,
                                               int *out_written);
/**
 * \brief Convert hexadecimal ASCII character to integer value
 * \param c Hexadecimal character
 * \return Integer 0..15 or -1 if invalid
 */
static int hex_char_to_int(char c);
/**
 * \brief Parse 4 hexadecimal digits into a 16-bit code unit
 * \param string Input 4-character hex sequence
 * \param result Output parsed unsigned int
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t parse_utf16_hex(const char *string,
                                          unsigned int *result);
/**
 * \brief Compute expected byte length of a UTF-8 sequence from lead byte
 * \param c Lead unsigned byte
 * \return Number of bytes (1..4) or 0 if invalid lead byte
 */
static int num_bytes_in_utf8_sequence(unsigned char c);
/**
 * \brief Verify a single UTF-8 multi-byte sequence
 * \param string Pointer to unsigned char sequence
 * \param len Output sequence length in bytes
 * \return JSONSuccess if valid UTF-8, JSONFailure otherwise
 */
static enum json_result_t verify_utf8_sequence(const unsigned char *string,
                                               int *len);
/**
 * \brief Validate that a buffer of string_len bytes is valid UTF-8
 * \param string Input buffer
 * \param string_len Length of input buffer in bytes
 * \return JSONSuccess if valid UTF-8, JSONFailure otherwise
 */
static enum json_result_t is_valid_utf8(const char *string, size_t string_len);
/**
 * \brief Check if a string prefix of length bytes is a valid JSON decimal
 * \param string Input string
 * \param length Length of token
 * \return JSONSuccess if valid JSON decimal, JSONFailure otherwise
 */
static enum json_result_t is_decimal(const char *string, size_t length);
/**
 * \brief Compute DJB2 hash of string up to n bytes
 * \param string Input string
 * \param n Maximum number of characters to hash
 * \return Computed hash value
 */
static unsigned long hash_string(const char *string, size_t n);

/* JSON Object */
/**
 * \brief Allocate and initialize a new JSON_Object
 * \param wrapping_value Parent wrapping JSON_Value
 * \param out_object Output pointer for allocated JSON_Object
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t json_object_make(JSON_Value *wrapping_value,
                                           JSON_Object **out_object);
/**
 * \brief Initialize internal hash table storage of a JSON_Object
 * \param object Target JSON_Object
 * \param capacity Hash table cell capacity
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t json_object_init(JSON_Object *object,
                                           size_t capacity);
/**
 * \brief Deinitialize internal buffers of a JSON_Object
 * \param object Target JSON_Object
 * \param free_keys Whether to free key strings
 * \param free_values Whether to free child JSON_Values
 */
static void json_object_deinit(JSON_Object *object, parson_bool_t free_keys,
                               parson_bool_t free_values);
/**
 * \brief Grow hash table capacity of a JSON_Object and rehash items
 * \param object Target JSON_Object
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t json_object_grow_and_rehash(JSON_Object *object);
/**
 * \brief Locate hash table cell index for a key in a non-empty table
 * \param object Target JSON_Object (must have cell_capacity > 0)
 * \param key Key string
 * \param key_len Key length in bytes
 * \param hash Precomputed hash of key
 * \param out_found Output boolean set to PARSON_TRUE if key exists
 * \param out_cell_ix Output hash table cell index
 * \return JSONSuccess on success, JSONFailure if table has zero capacity
 */
static enum json_result_t
json_object_get_cell_ix(const JSON_Object *object, const char *key,
                        size_t key_len, unsigned long hash,
                        parson_bool_t *out_found, size_t *out_cell_ix);
/**
 * \brief Insert a newly owned key and JSON_Value into a JSON_Object
 * \param object Target JSON_Object
 * \param name Owned NUL-terminated key string
 * \param value Owned JSON_Value
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t json_object_add(JSON_Object *object, char *name,
                                          JSON_Value *value);
/**
 * \brief Look up a JSON_Value in a JSON_Object by key and explicit length
 * \param object Target JSON_Object
 * \param name Key buffer
 * \param name_len Key length in bytes
 * \param out_value Output pointer for found JSON_Value
 * \return JSONSuccess if found, JSONFailure otherwise
 */
static enum json_result_t json_object_getn_value(const JSON_Object *object,
                                                 const char *name,
                                                 size_t name_len,
                                                 JSON_Value **out_value);
/**
 * \brief Remove a key-value pair from a JSON_Object
 * \param object Target JSON_Object
 * \param name Key name
 * \param free_value Whether to free the removed JSON_Value
 * \return JSONSuccess on success, JSONFailure if missing or invalid
 */
static enum json_result_t json_object_remove_internal(JSON_Object *object,
                                                      const char *name,
                                                      parson_bool_t free_value);
/**
 * \brief Remove a nested key-value pair using dot notation
 * \param object Root JSON_Object
 * \param name Dot-separated key path
 * \param free_value Whether to free the removed JSON_Value
 * \return JSONSuccess on success, JSONFailure if missing or invalid
 */
static enum json_result_t
json_object_dotremove_internal(JSON_Object *object, const char *name,
                               parson_bool_t free_value);
/**
 * \brief Free a JSON_Object and all its owned children
 * \param object Target JSON_Object
 */
static void json_object_free(JSON_Object *object);

/* JSON Array */
/**
 * \brief Allocate and initialize a new JSON_Array
 * \param wrapping_value Parent wrapping JSON_Value
 * \param out_array Output pointer for allocated JSON_Array
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t json_array_make(JSON_Value *wrapping_value,
                                          JSON_Array **out_array);
/**
 * \brief Append an owned JSON_Value to a JSON_Array
 * \param array Target JSON_Array
 * \param value Owned JSON_Value to append
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t json_array_add(JSON_Array *array, JSON_Value *value);
/**
 * \brief Resize internal item pointer array of a JSON_Array
 * \param array Target JSON_Array
 * \param new_capacity New capacity in elements (must be > 0)
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t json_array_resize(JSON_Array *array,
                                            size_t new_capacity);
/**
 * \brief Free a JSON_Array and all its owned elements
 * \param array Target JSON_Array
 */
static void json_array_free(JSON_Array *array);

/* JSON Value */
/**
 * \brief Initialize a JSONString value taking ownership of an existing buffer
 * \param string Owned character buffer
 * \param length Length of string in bytes
 * \param out_value Output pointer for allocated JSON_Value
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t
json_value_init_string_no_copy(char *string, size_t length,
                               JSON_Value **out_value);
/**
 * \brief Retrieve internal JSON_String descriptor from a JSONString value
 * \param value Target JSON_Value
 * \param out_desc Output pointer for JSON_String descriptor
 * \return JSONSuccess if value is a JSONString, JSONFailure otherwise
 */
static enum json_result_t
json_value_get_string_desc(const JSON_Value *value,
                           const JSON_String **out_desc);

/* Parser */
/**
 * \brief Advance string pointer past a double-quoted JSON string literal
 * \param string In/out cursor into JSON text
 * \return JSONSuccess on success, JSONFailure on unterminated quote
 */
static enum json_result_t skip_quotes(const char **string);
/**
 * \brief Decode a UTF-16 escape sequence (\\uXXXX or surrogate pair) to UTF-8
 * \param unprocessed In/out pointer into escape sequence
 * \param processed In/out write cursor in UTF-8 output buffer
 * \return JSONSuccess on success, JSONFailure on invalid escape
 */
static enum json_result_t parse_utf16(const char **unprocessed,
                                      char **processed);
/**
 * \brief Unescape a raw JSON string body into a newly allocated buffer
 * \param input Raw string characters after opening quote
 * \param input_len Length of raw characters in bytes
 * \param output_len Output unescaped length in bytes
 * \param out_string Output newly allocated unescaped string
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t process_string(const char *input, size_t input_len,
                                         size_t *output_len, char **out_string);
/**
 * \brief Extract and unescape a quoted string token at cursor
 * \param string In/out cursor positioned at opening quote
 * \param output_string_len Output length of unescaped string
 * \param out_string Output newly allocated string
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t get_quoted_string(const char **string,
                                            size_t *output_string_len,
                                            char **out_string);
/**
 * \brief Parse a JSON object value starting at '{'
 * \param string In/out cursor into JSON text
 * \param nesting Current recursion depth
 * \param out_value Output newly allocated JSON_Value
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t
parse_object_value(const char **string, size_t nesting, JSON_Value **out_value);
/**
 * \brief Parse a JSON array value starting at '['
 * \param string In/out cursor into JSON text
 * \param nesting Current recursion depth
 * \param out_value Output newly allocated JSON_Value
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t parse_array_value(const char **string, size_t nesting,
                                            JSON_Value **out_value);
/**
 * \brief Parse a JSON string value starting at '"'
 * \param string In/out cursor into JSON text
 * \param out_value Output newly allocated JSON_Value
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t parse_string_value(const char **string,
                                             JSON_Value **out_value);
/**
 * \brief Parse a JSON boolean literal ('true' or 'false')
 * \param string In/out cursor into JSON text
 * \param out_value Output newly allocated JSON_Value
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t parse_boolean_value(const char **string,
                                              JSON_Value **out_value);
/**
 * \brief Parse a JSON number literal
 * \param string In/out cursor into JSON text
 * \param out_value Output newly allocated JSON_Value
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t parse_number_value(const char **string,
                                             JSON_Value **out_value);
/**
 * \brief Parse a JSON 'null' literal
 * \param string In/out cursor into JSON text
 * \param out_value Output newly allocated JSON_Value
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t parse_null_value(const char **string,
                                           JSON_Value **out_value);
/**
 * \brief Parse any JSON value at cursor
 * \param string In/out cursor into JSON text
 * \param nesting Current recursion depth
 * \param out_value Output newly allocated JSON_Value
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t parse_value(const char **string, size_t nesting,
                                      JSON_Value **out_value);

/* Serialization */
/**
 * \brief Append literal bytes to optional serialization buffer and advance
 * count
 * \param buf In/out buffer pointer (may point to NULL when measuring size)
 * \param str Source literal bytes
 * \param len Number of bytes to copy
 * \param written_total In/out accumulated byte count
 */
static void append_string(char **buf, const char *str, size_t len,
                          int *written_total);
/**
 * \brief Append pretty-print indentation spaces for nesting level
 * \param buf In/out buffer pointer (may point to NULL when measuring size)
 * \param level Indentation depth
 * \param written_total In/out accumulated byte count
 */
static void append_indent(char **buf, int level, int *written_total);
/**
 * \brief Recursively serialize a JSON_Value into buffer or count bytes
 * \param value JSON_Value to serialize
 * \param buf Destination buffer or NULL to measure length only
 * \param level Current pretty-print indentation depth
 * \param is_pretty Whether pretty formatting is enabled
 * \param num_buf Scratch buffer of PARSON_NUM_BUF_SIZE bytes for numbers
 * \param out_written Output number of bytes written (excluding NUL)
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t json_serialize_to_buffer_r(const JSON_Value *value,
                                                     char *buf, int level,
                                                     parson_bool_t is_pretty,
                                                     char *num_buf,
                                                     int *out_written);
/**
 * \brief Serialize and escape a JSON string into buffer or count bytes
 * \param string Raw string characters
 * \param len Length of string in bytes
 * \param buf Destination buffer or NULL to measure length only
 * \param out_written Output number of bytes written
 * \return JSONSuccess on success, JSONFailure on failure
 */
static enum json_result_t json_serialize_string(const char *string, size_t len,
                                                char *buf, int *out_written);

/* Various */
static enum json_result_t parson_fopen(const char *filename, const char *mode,
                                       FILE **out_fp) {
  FILE *fp = NULL;
  if (out_fp == NULL) {
    return JSONFailure;
  }
  *out_fp = NULL;
  if (filename == NULL || mode == NULL) {
    return JSONFailure;
  }
#if defined(_MSC_VER)
  if (fopen_s(&fp, filename, mode) != 0 || fp == NULL) {
    return JSONFailure;
  }
#else
  fp = fopen(filename, mode);
  if (fp == NULL) {
    return JSONFailure;
  }
#endif
  *out_fp = fp;
  return JSONSuccess;
}

static enum json_result_t read_file(const char *filename, char **out_contents) {
  enum json_result_t rc;
  FILE *fp = NULL;
  size_t size_to_read = 0;
  size_t size_read = 0;
  long pos = 0;
  char *file_contents = NULL;

  *out_contents = NULL;
  rc = parson_fopen(filename, "r", &fp);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    return rc;
  }
  if (PARSON_FSEEK(fp, 0L, SEEK_END) != 0) {
    PARSON_FCLOSE(fp);
    return JSONFailure;
  }
  pos = PARSON_FTELL(fp);
  if (pos < 0) {
    PARSON_FCLOSE(fp);
    return JSONFailure;
  }
  size_to_read = (size_t)pos;
  rewind(fp);
  file_contents = (char *)parson_malloc(sizeof(char) * (size_to_read + 1));
  if (file_contents == NULL) {
    PARSON_FCLOSE(fp);
    LOG_DEBUG("OOM\n");
    return JSONFailure;
  }
  size_read = PARSON_FREAD(file_contents, 1, size_to_read, fp);
  if (size_read == 0 || PARSON_FERROR(fp)) {
    PARSON_FCLOSE(fp);
    parson_free(file_contents);
    return JSONFailure;
  }
  PARSON_FCLOSE(fp);
  file_contents[size_read] = '\0';
  *out_contents = file_contents;
  return JSONSuccess;
}

static void remove_comments(char *string, const char *start_token,
                            const char *end_token) {
  parson_bool_t in_string = PARSON_FALSE, escaped = PARSON_FALSE;
  size_t i = 0;
  char *ptr = NULL, current_char = '\0';
  size_t start_token_len = strlen(start_token);
  size_t end_token_len = strlen(end_token);
  while ((current_char = *string) != '\0') {
    if (current_char == '\\' && !escaped) {
      escaped = PARSON_TRUE;
      string++;
      continue;
    } else if (current_char == '\"' && !escaped) {
      in_string = !in_string;
    } else if (!in_string &&
               strncmp(string, start_token, start_token_len) == 0) {
      for (i = 0; i < start_token_len; i++) {
        string[i] = ' ';
      }
      string = string + start_token_len;
      ptr = strstr(string, end_token);
      if (!ptr) {
        return;
      }
      for (i = 0; i < (size_t)(ptr - string) + end_token_len; i++) {
        string[i] = ' ';
      }
      string = ptr + end_token_len - 1;
    }
    escaped = PARSON_FALSE;
    string++;
  }
}

static enum json_result_t parson_strndup(const char *string, size_t n,
                                         char **out_string) {
  char *output_string = (char *)parson_malloc(n + 1);
  if (output_string == NULL) {
    LOG_DEBUG("OOM\n");
    *out_string = NULL;
    return JSONFailure;
  }
  if (n > 0) {
#if defined(_MSC_VER)
    memcpy_s(output_string, n + 1, string, n);
#else
    memcpy(output_string, string, n);
#endif
  }
  output_string[n] = '\0';
  *out_string = output_string;
  return JSONSuccess;
}

static enum json_result_t parson_strdup(const char *string, char **out_string) {
  size_t len = strlen(string);
  char *output_string = (char *)parson_malloc(len + 1);
  if (output_string == NULL) {
    LOG_DEBUG("OOM\n");
    *out_string = NULL;
    return JSONFailure;
  }
#if defined(_MSC_VER)
  strcpy_s(output_string, len + 1, string);
#else
  (strcpy)(output_string, string);
#endif
  *out_string = output_string;
  return JSONSuccess;
}

static enum json_result_t parson_format_double(char *buf, size_t buf_size,
                                               const char *format, double num,
                                               int *out_written) {
  char tmp[512];
  int written = -1;
  int width = 0;
  int prec = 0;
  int has_prec = 0;
  int left_align = 0;
  const char *p = format;
  char conv = '\0';

  *out_written = -1;
  if (format == NULL) {
#if defined(_MSC_VER)
    written = sprintf_s(buf, buf_size, PARSON_DEFAULT_FLOAT_FORMAT, num);
#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||            \
    (defined(__cplusplus) && __cplusplus >= 201103L)
    written = snprintf(buf, buf_size, PARSON_DEFAULT_FLOAT_FORMAT, num);
#else
    (void)buf_size;
    written = sprintf(buf, PARSON_DEFAULT_FLOAT_FORMAT, num);
#endif
    *out_written = written;
    return JSONSuccess;
  }

  if (*p != '%') {
    return JSONFailure;
  }
  p++;
  if (*p == '-') {
    left_align = 1;
    p++;
  }
  while (*p >= '0' && *p <= '9') {
    width = width * 10 + (int)(*p - '0');
    if ((size_t)width >= buf_size) {
      return JSONFailure;
    }
    p++;
  }
  if (left_align) {
    width = -width;
  }
  if (*p == '.') {
    has_prec = 1;
    p++;
    while (*p >= '0' && *p <= '9') {
      prec = prec * 10 + (int)(*p - '0');
      if ((size_t)prec >= buf_size) {
        return JSONFailure;
      }
      p++;
    }
  }
  conv = *p;
  if (conv == '\0' || p[1] != '\0') {
    return JSONFailure;
  }
  switch (conv) {
  case 'f':
#if defined(_MSC_VER)
    written = has_prec ? sprintf_s(tmp, sizeof(tmp), "%*.*f", width, prec, num)
                       : sprintf_s(tmp, sizeof(tmp), "%*f", width, num);
#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||            \
    (defined(__cplusplus) && __cplusplus >= 201103L)
    written = has_prec ? snprintf(tmp, sizeof(tmp), "%*.*f", width, prec, num)
                       : snprintf(tmp, sizeof(tmp), "%*f", width, num);
#else
    written = has_prec ? sprintf(tmp, "%*.*f", width, prec, num)
                       : sprintf(tmp, "%*f", width, num);
#endif
    break;
  case 'e':
#if defined(_MSC_VER)
    written = has_prec ? sprintf_s(tmp, sizeof(tmp), "%*.*e", width, prec, num)
                       : sprintf_s(tmp, sizeof(tmp), "%*e", width, num);
#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||            \
    (defined(__cplusplus) && __cplusplus >= 201103L)
    written = has_prec ? snprintf(tmp, sizeof(tmp), "%*.*e", width, prec, num)
                       : snprintf(tmp, sizeof(tmp), "%*e", width, num);
#else
    written = has_prec ? sprintf(tmp, "%*.*e", width, prec, num)
                       : sprintf(tmp, "%*e", width, num);
#endif
    break;
  case 'E':
#if defined(_MSC_VER)
    written = has_prec ? sprintf_s(tmp, sizeof(tmp), "%*.*E", width, prec, num)
                       : sprintf_s(tmp, sizeof(tmp), "%*E", width, num);
#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||            \
    (defined(__cplusplus) && __cplusplus >= 201103L)
    written = has_prec ? snprintf(tmp, sizeof(tmp), "%*.*E", width, prec, num)
                       : snprintf(tmp, sizeof(tmp), "%*E", width, num);
#else
    written = has_prec ? sprintf(tmp, "%*.*E", width, prec, num)
                       : sprintf(tmp, "%*E", width, num);
#endif
    break;
  case 'g':
#if defined(_MSC_VER)
    written = has_prec ? sprintf_s(tmp, sizeof(tmp), "%*.*g", width, prec, num)
                       : sprintf_s(tmp, sizeof(tmp), "%*g", width, num);
#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||            \
    (defined(__cplusplus) && __cplusplus >= 201103L)
    written = has_prec ? snprintf(tmp, sizeof(tmp), "%*.*g", width, prec, num)
                       : snprintf(tmp, sizeof(tmp), "%*g", width, num);
#else
    written = has_prec ? sprintf(tmp, "%*.*g", width, prec, num)
                       : sprintf(tmp, "%*g", width, num);
#endif
    break;
  case 'G':
#if defined(_MSC_VER)
    written = has_prec ? sprintf_s(tmp, sizeof(tmp), "%*.*G", width, prec, num)
                       : sprintf_s(tmp, sizeof(tmp), "%*G", width, num);
#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||            \
    (defined(__cplusplus) && __cplusplus >= 201103L)
    written = has_prec ? snprintf(tmp, sizeof(tmp), "%*.*G", width, prec, num)
                       : snprintf(tmp, sizeof(tmp), "%*G", width, num);
#else
    written = has_prec ? sprintf(tmp, "%*.*G", width, prec, num)
                       : sprintf(tmp, "%*G", width, num);
#endif
    break;
  default:
    return JSONFailure;
  }
  if ((size_t)written >= buf_size) {
    return JSONFailure;
  }
#if defined(_MSC_VER)
  memcpy_s(buf, buf_size, tmp, (size_t)written + 1);
#else
  memcpy(buf, tmp, (size_t)written + 1);
#endif
  *out_written = written;
  return JSONSuccess;
}

static int hex_char_to_int(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return -1;
}

static enum json_result_t parse_utf16_hex(const char *s, unsigned int *result) {
  int x1, x2, x3, x4;
  if (s[0] == '\0' || s[1] == '\0' || s[2] == '\0' || s[3] == '\0') {
    return JSONFailure;
  }
  x1 = hex_char_to_int(s[0]);
  x2 = hex_char_to_int(s[1]);
  x3 = hex_char_to_int(s[2]);
  x4 = hex_char_to_int(s[3]);
  if (x1 == -1 || x2 == -1 || x3 == -1 || x4 == -1) {
    return JSONFailure;
  }
  *result = (unsigned int)((x1 << 12) | (x2 << 8) | (x3 << 4) | x4);
  return JSONSuccess;
}

static int num_bytes_in_utf8_sequence(unsigned char c) {
  if (c == 0xC0 || c == 0xC1 || c > 0xF4 || IS_CONT(c)) {
    return 0;
  } else if ((c & 0x80) == 0) { /* 0xxxxxxx */
    return 1;
  } else if ((c & 0xE0) == 0xC0) { /* 110xxxxx */
    return 2;
  } else if ((c & 0xF0) == 0xE0) { /* 1110xxxx */
    return 3;
  }
  return 4; /* 11110xxx (0xF0..0xF4) */
}

static enum json_result_t verify_utf8_sequence(const unsigned char *string,
                                               int *len) {
  unsigned long cp = 0;
  *len = num_bytes_in_utf8_sequence(string[0]);

  if (*len == 1) {
    cp = string[0];
  } else if (*len == 2 && IS_CONT(string[1])) {
    cp = (unsigned long)(string[0] & 0x1F);
    cp = (cp << 6) | (unsigned long)(string[1] & 0x3F);
  } else if (*len == 3 && IS_CONT(string[1]) && IS_CONT(string[2])) {
    cp = (unsigned long)(string[0] & 0xF);
    cp = (cp << 6) | (unsigned long)(string[1] & 0x3F);
    cp = (cp << 6) | (unsigned long)(string[2] & 0x3F);
  } else if (*len == 4 && IS_CONT(string[1]) && IS_CONT(string[2]) &&
             IS_CONT(string[3])) {
    cp = (unsigned long)(string[0] & 0x7);
    cp = (cp << 6) | (unsigned long)(string[1] & 0x3F);
    cp = (cp << 6) | (unsigned long)(string[2] & 0x3F);
    cp = (cp << 6) | (unsigned long)(string[3] & 0x3F);
  } else {
    return JSONFailure;
  }

  /* overlong encodings */
  if ((cp < 0x80UL && *len > 1) || (cp < 0x800UL && *len > 2) ||
      (cp < 0x10000UL && *len > 3)) {
    return JSONFailure;
  }

  /* invalid unicode */
  if (cp > 0x10FFFFUL) {
    return JSONFailure;
  }

  /* surrogate halves */
  if (cp >= 0xD800UL && cp <= 0xDFFFUL) {
    return JSONFailure;
  }

  return JSONSuccess;
}

static enum json_result_t is_valid_utf8(const char *string, size_t string_len) {
  enum json_result_t rc;
  int len = 0;
  const char *string_end = string + string_len;
  while (string < string_end) {
    rc = verify_utf8_sequence((const unsigned char *)string, &len);
    if (rc != JSONSuccess) {
      LOG_DEBUG("Error: %d\n", (int)rc);
      return rc;
    }
    string += len;
  }
  return JSONSuccess;
}

static enum json_result_t is_decimal(const char *string, size_t length) {
  if (length == 1 && string[0] == '-') {
    return JSONFailure;
  }
  if (length > 1 && string[0] == '0' && string[1] != '.') {
    return JSONFailure;
  }
  if (length > 2 && !strncmp(string, "-0", 2) && string[2] != '.') {
    return JSONFailure;
  }
  while (length--) {
    if (strchr("xX", string[length])) {
      return JSONFailure;
    }
  }
  return JSONSuccess;
}

static unsigned long hash_string(const char *string, size_t n) {
#ifdef PARSON_FORCE_HASH_COLLISIONS
  (void)string;
  (void)n;
  return 0;
#else
  unsigned long hash = 5381;
  unsigned char c = 0;
  size_t i = 0;
  for (i = 0; i < n; i++) {
    c = (unsigned char)string[i];
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }
  return hash;
#endif
}

/* JSON Object */
static enum json_result_t json_object_make(JSON_Value *wrapping_value,
                                           JSON_Object **out_object) {
  enum json_result_t rc;
  JSON_Object *new_obj = (JSON_Object *)parson_malloc(sizeof(JSON_Object));
  if (new_obj == NULL) {
    LOG_DEBUG("OOM\n");
    *out_object = NULL;
    return JSONFailure;
  }
  new_obj->wrapping_value = wrapping_value;
  rc = json_object_init(new_obj, 0);
  *out_object = new_obj;
  return rc;
}

static enum json_result_t json_object_init(JSON_Object *object,
                                           size_t capacity) {
  size_t i = 0;

  object->cells = NULL;
  object->names = NULL;
  object->values = NULL;
  object->cell_ixs = NULL;
  object->hashes = NULL;

  object->count = 0;
  object->cell_capacity = capacity;
  object->item_capacity = (capacity * 7) / 10;

  if (capacity == 0) {
    return JSONSuccess;
  }

  object->cells =
      (size_t *)parson_malloc(object->cell_capacity * sizeof(*object->cells));
  object->names =
      (char **)parson_malloc(object->item_capacity * sizeof(*object->names));
  object->values = (JSON_Value **)parson_malloc(object->item_capacity *
                                                sizeof(*object->values));
  object->cell_ixs = (size_t *)parson_malloc(object->item_capacity *
                                             sizeof(*object->cell_ixs));
  object->hashes = (unsigned long *)parson_malloc(object->item_capacity *
                                                  sizeof(*object->hashes));
  if (object->cells == NULL || object->names == NULL ||
      object->values == NULL || object->cell_ixs == NULL ||
      object->hashes == NULL) {
    LOG_DEBUG("OOM\n");
    goto error;
  }
  for (i = 0; i < object->cell_capacity; i++) {
    object->cells[i] = OBJECT_INVALID_IX;
  }
  return JSONSuccess;
error:
  parson_free(object->cells);
  parson_free(object->names);
  parson_free(object->values);
  parson_free(object->cell_ixs);
  parson_free(object->hashes);
  return JSONFailure;
}

static void json_object_deinit(JSON_Object *object, parson_bool_t free_keys,
                               parson_bool_t free_values) {
  size_t i = 0;
  for (i = 0; i < object->count; i++) {
    if (free_keys) {
      parson_free(object->names[i]);
    }
    if (free_values) {
      json_value_free(object->values[i]);
    }
  }

  object->count = 0;
  object->item_capacity = 0;
  object->cell_capacity = 0;

  parson_free(object->cells);
  parson_free(object->names);
  parson_free(object->values);
  parson_free(object->cell_ixs);
  parson_free(object->hashes);

  object->cells = NULL;
  object->names = NULL;
  object->values = NULL;
  object->cell_ixs = NULL;
  object->hashes = NULL;
}

static enum json_result_t json_object_grow_and_rehash(JSON_Object *object) {
  enum json_result_t rc;
  JSON_Value *wrapping_value = NULL;
  JSON_Object new_object;
  char *key = NULL;
  JSON_Value *value = NULL;
  unsigned long hash = 0;
  parson_bool_t found = PARSON_FALSE;
  size_t cell_ix = 0;
  size_t i = 0;
  size_t new_capacity = MAX(object->cell_capacity * 2, STARTING_CAPACITY);

  rc = json_object_init(&new_object, new_capacity);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    return rc;
  }

  wrapping_value = json_object_get_wrapping_value(object);
  new_object.wrapping_value = wrapping_value;

  for (i = 0; i < object->count; i++) {
    key = object->names[i];
    value = object->values[i];
    hash = object->hashes[i];
    rc = json_object_get_cell_ix(&new_object, key, strlen(key), hash, &found,
                                 &cell_ix);
    if (rc != JSONSuccess || found) {
      LOG_DEBUG("Error: %d\n", (int)rc);
      json_object_deinit(&new_object, PARSON_FALSE, PARSON_FALSE);
      return JSONFailure;
    }
    new_object.names[new_object.count] = key;
    new_object.cells[cell_ix] = new_object.count;
    new_object.values[new_object.count] = value;
    new_object.cell_ixs[new_object.count] = cell_ix;
    new_object.hashes[new_object.count] = hash;
    new_object.count++;
    value->parent = wrapping_value;
  }
  json_object_deinit(object, PARSON_FALSE, PARSON_FALSE);
  *object = new_object;
  return JSONSuccess;
}

static enum json_result_t
json_object_get_cell_ix(const JSON_Object *object, const char *key,
                        size_t key_len, unsigned long hash,
                        parson_bool_t *out_found, size_t *out_cell_ix) {
  size_t cell_ix = 0;
  size_t cell = 0;
  size_t ix = 0;
  size_t i = 0;
  unsigned long hash_to_check = 0;
  const char *key_to_check = NULL;
  size_t key_to_check_len = 0;

  *out_found = PARSON_FALSE;
  *out_cell_ix = OBJECT_INVALID_IX;
  if (object->cell_capacity == 0) {
    return JSONFailure;
  }

  cell_ix = hash & (object->cell_capacity - 1);
  for (i = 0; i < object->cell_capacity; i++) {
    ix = (cell_ix + i) & (object->cell_capacity - 1);
    cell = object->cells[ix];
    if (cell == OBJECT_INVALID_IX) {
      *out_cell_ix = ix;
      return JSONSuccess;
    }
    hash_to_check = object->hashes[cell];
    if (hash != hash_to_check) {
      continue;
    }
    key_to_check = object->names[cell];
    key_to_check_len = strlen(key_to_check);
    if (key_to_check_len == key_len &&
        strncmp(key, key_to_check, key_len) == 0) {
      *out_found = PARSON_TRUE;
      *out_cell_ix = ix;
      return JSONSuccess;
    }
  }
  return JSONFailure;
}

static enum json_result_t json_object_add(JSON_Object *object, char *name,
                                          JSON_Value *value) {
  enum json_result_t rc = JSONFailure;
  unsigned long hash = 0;
  parson_bool_t found = PARSON_FALSE;
  size_t cell_ix = 0;
  size_t name_len = 0;

  if (!object || !name || !value) {
    return JSONFailure;
  }

  name_len = strlen(name);
  hash = hash_string(name, name_len);
  if (object->cell_capacity > 0) {
    rc =
        json_object_get_cell_ix(object, name, name_len, hash, &found, &cell_ix);
    if (rc == JSONSuccess && found) {
      return JSONFailure;
    }
  }

  if (object->count >= object->item_capacity) {
    rc = json_object_grow_and_rehash(object);
    if (rc != JSONSuccess) {
      LOG_DEBUG("Error: %d\n", (int)rc);
      return rc;
    }
    rc =
        json_object_get_cell_ix(object, name, name_len, hash, &found, &cell_ix);
  }
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    return rc;
  }

  object->names[object->count] = name;
  object->cells[cell_ix] = object->count;
  object->values[object->count] = value;
  object->cell_ixs[object->count] = cell_ix;
  object->hashes[object->count] = hash;
  object->count++;
  value->parent = json_object_get_wrapping_value(object);

  return JSONSuccess;
}

static enum json_result_t json_object_getn_value(const JSON_Object *object,
                                                 const char *name,
                                                 size_t name_len,
                                                 JSON_Value **out_value) {
  enum json_result_t rc;
  unsigned long hash = 0;
  parson_bool_t found = PARSON_FALSE;
  size_t cell_ix = 0;
  size_t item_ix = 0;

  *out_value = NULL;
  if (!object || !name || object->count == 0) {
    return JSONFailure;
  }
  hash = hash_string(name, name_len);
  rc = json_object_get_cell_ix(object, name, name_len, hash, &found, &cell_ix);
  if (rc != JSONSuccess || !found) {
    return JSONFailure;
  }
  item_ix = object->cells[cell_ix];
  *out_value = object->values[item_ix];
  return JSONSuccess;
}

static enum json_result_t
json_object_remove_internal(JSON_Object *object, const char *name,
                            parson_bool_t free_value) {
  enum json_result_t rc;
  unsigned long hash = 0;
  parson_bool_t found = PARSON_FALSE;
  size_t cell = 0;
  size_t item_ix = 0;
  size_t last_item_ix = 0;
  size_t i = 0;
  size_t j = 0;
  size_t x = 0;
  size_t k = 0;
  size_t name_len = 0;
  JSON_Value *val = NULL;

  if (object == NULL || name == NULL || object->count == 0) {
    return JSONFailure;
  }

  name_len = strlen(name);
  hash = hash_string(name, name_len);
  rc = json_object_get_cell_ix(object, name, name_len, hash, &found, &cell);
  if (rc != JSONSuccess || !found) {
    return JSONFailure;
  }

  item_ix = object->cells[cell];
  if (free_value) {
    val = object->values[item_ix];
    json_value_free(val);
    val = NULL;
  }

  parson_free(object->names[item_ix]);
  last_item_ix = object->count - 1;
  if (item_ix < last_item_ix) {
    object->names[item_ix] = object->names[last_item_ix];
    object->values[item_ix] = object->values[last_item_ix];
    object->cell_ixs[item_ix] = object->cell_ixs[last_item_ix];
    object->hashes[item_ix] = object->hashes[last_item_ix];
    object->cells[object->cell_ixs[item_ix]] = item_ix;
  }
  object->count--;

  i = cell;
  j = i;
  for (x = 0; x < (object->cell_capacity - 1); x++) {
    j = (j + 1) & (object->cell_capacity - 1);
    if (object->cells[j] == OBJECT_INVALID_IX) {
      break;
    }
    k = object->hashes[object->cells[j]] & (object->cell_capacity - 1);

    if ((j > i && (k <= i || k > j)) || (j < i && (k <= i && k > j))) {
      object->cell_ixs[object->cells[j]] = i;
      object->cells[i] = object->cells[j];
      i = j;
    }
  }
  object->cells[i] = OBJECT_INVALID_IX;
  return JSONSuccess;
}

static enum json_result_t
json_object_dotremove_internal(JSON_Object *object, const char *name,
                               parson_bool_t free_value) {
  enum json_result_t rc;
  JSON_Value *temp_value = NULL;
  JSON_Object *temp_object = NULL;
  const char *dot_pos = NULL;

  if (object == NULL || name == NULL) {
    return JSONFailure;
  }
  dot_pos = strchr(name, '.');
  if (!dot_pos) {
    return json_object_remove_internal(object, name, free_value);
  }
  rc = json_object_getn_value(object, name, (size_t)(dot_pos - name),
                              &temp_value);
  if (rc != JSONSuccess || json_value_get_type(temp_value) != JSONObject) {
    return JSONFailure;
  }
  temp_object = json_value_get_object(temp_value);
  return json_object_dotremove_internal(temp_object, dot_pos + 1, free_value);
}

static void json_object_free(JSON_Object *object) {
  json_object_deinit(object, PARSON_TRUE, PARSON_TRUE);
  parson_free(object);
}

/* JSON Array */
static enum json_result_t json_array_make(JSON_Value *wrapping_value,
                                          JSON_Array **out_array) {
  JSON_Array *new_array = (JSON_Array *)parson_malloc(sizeof(JSON_Array));
  if (new_array == NULL) {
    LOG_DEBUG("OOM\n");
    *out_array = NULL;
    return JSONFailure;
  }
  new_array->wrapping_value = wrapping_value;
  new_array->items = (JSON_Value **)NULL;
  new_array->capacity = 0;
  new_array->count = 0;
  *out_array = new_array;
  return JSONSuccess;
}

static enum json_result_t json_array_add(JSON_Array *array, JSON_Value *value) {
  enum json_result_t rc;
  if (array == NULL || value == NULL) {
    return JSONFailure;
  }
  if (array->count >= array->capacity) {
    size_t new_capacity = MAX(array->capacity * 2, STARTING_CAPACITY);
    rc = json_array_resize(array, new_capacity);
    if (rc != JSONSuccess) {
      LOG_DEBUG("Error: %d\n", (int)rc);
      return rc;
    }
  }
  value->parent = json_array_get_wrapping_value(array);
  array->items[array->count] = value;
  array->count++;
  return JSONSuccess;
}

static enum json_result_t json_array_resize(JSON_Array *array,
                                            size_t new_capacity) {
  JSON_Value **new_items = NULL;
  if (array == NULL || new_capacity == 0) {
    return JSONFailure;
  }
  new_items = (JSON_Value **)parson_malloc(new_capacity * sizeof(JSON_Value *));
  if (new_items == NULL) {
    LOG_DEBUG("OOM\n");
    return JSONFailure;
  }
  if (array->items != NULL && array->count > 0) {
#if defined(_MSC_VER)
    memcpy_s(new_items, new_capacity * sizeof(JSON_Value *), array->items,
             array->count * sizeof(JSON_Value *));
#else
    memcpy(new_items, array->items, array->count * sizeof(JSON_Value *));
#endif
  }
  parson_free(array->items);
  array->items = new_items;
  array->capacity = new_capacity;
  return JSONSuccess;
}

static void json_array_free(JSON_Array *array) {
  size_t i = 0;
  for (i = 0; i < array->count; i++) {
    json_value_free(array->items[i]);
  }
  parson_free(array->items);
  parson_free(array);
}

/* JSON Value */
static enum json_result_t
json_value_init_string_no_copy(char *string, size_t length,
                               JSON_Value **out_value) {
  JSON_Value *new_value = (JSON_Value *)parson_malloc(sizeof(JSON_Value));
  if (!new_value) {
    LOG_DEBUG("OOM\n");
    *out_value = NULL;
    return JSONFailure;
  }
  new_value->parent = NULL;
  new_value->type = JSONString;
  new_value->value.string.chars = string;
  new_value->value.string.length = length;
  *out_value = new_value;
  return JSONSuccess;
}

/* Parser */
static enum json_result_t skip_quotes(const char **string) {
  if (**string != '\"') {
    return JSONFailure;
  }
  SKIP_CHAR(string);
  while (**string != '\"') {
    if (**string == '\0') {
      return JSONFailure;
    } else if (**string == '\\') {
      SKIP_CHAR(string);
      if (**string == '\0') {
        return JSONFailure;
      }
    }
    SKIP_CHAR(string);
  }
  SKIP_CHAR(string);
  return JSONSuccess;
}

static enum json_result_t parse_utf16(const char **unprocessed,
                                      char **processed) {
  enum json_result_t rc;
  unsigned int hex_unit = 0;
  unsigned long cp = 0;
  unsigned long lead = 0;
  unsigned long trail = 0;
  char *processed_ptr = *processed;
  const char *unprocessed_ptr = *unprocessed;

  unprocessed_ptr++; /* skips u */
  rc = parse_utf16_hex(unprocessed_ptr, &hex_unit);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    return rc;
  }
  cp = (unsigned long)hex_unit;
  if (cp < 0x80UL) {
    processed_ptr[0] = (char)cp; /* 0xxxxxxx */
  } else if (cp < 0x800UL) {
    processed_ptr[0] = (char)(((cp >> 6) & 0x1FUL) | 0xC0UL); /* 110xxxxx */
    processed_ptr[1] = (char)(((cp) & 0x3FUL) | 0x80UL);      /* 10xxxxxx */
    processed_ptr += 1;
  } else if (cp < 0xD800UL || cp > 0xDFFFUL) {
    processed_ptr[0] = (char)(((cp >> 12) & 0x0FUL) | 0xE0UL); /* 1110xxxx */
    processed_ptr[1] = (char)(((cp >> 6) & 0x3FUL) | 0x80UL);  /* 10xxxxxx */
    processed_ptr[2] = (char)(((cp) & 0x3FUL) | 0x80UL);       /* 10xxxxxx */
    processed_ptr += 2;
  } else if (cp <= 0xDBFFUL) { /* lead surrogate (0xD800..0xDBFF) */
    lead = cp;
    unprocessed_ptr += 4;
    if (*unprocessed_ptr++ != '\\' || *unprocessed_ptr++ != 'u') {
      return JSONFailure;
    }
    rc = parse_utf16_hex(unprocessed_ptr, &hex_unit);
    trail = (unsigned long)hex_unit;
    if (rc != JSONSuccess || trail < 0xDC00UL || trail > 0xDFFFUL) {
      return JSONFailure;
    }
    cp = ((((lead - 0xD800UL) & 0x3FFUL) << 10) |
          ((trail - 0xDC00UL) & 0x3FFUL)) +
         0x010000UL;
    processed_ptr[0] = (char)((((cp >> 18) & 0x07UL) | 0xF0UL)); /* 11110xxx */
    processed_ptr[1] = (char)((((cp >> 12) & 0x3FUL) | 0x80UL)); /* 10xxxxxx */
    processed_ptr[2] = (char)((((cp >> 6) & 0x3FUL) | 0x80UL));  /* 10xxxxxx */
    processed_ptr[3] = (char)((((cp) & 0x3FUL) | 0x80UL));       /* 10xxxxxx */
    processed_ptr += 3;
  } else { /* trail surrogate before lead surrogate */
    return JSONFailure;
  }
  unprocessed_ptr += 3;
  *processed = processed_ptr;
  *unprocessed = unprocessed_ptr;
  return JSONSuccess;
}

static enum json_result_t process_string(const char *input, size_t input_len,
                                         size_t *output_len,
                                         char **out_string) {
  enum json_result_t rc;
  const char *input_ptr = input;
  size_t initial_size = (input_len + 1) * sizeof(char);
  size_t final_size = 0;
  char *output = NULL;
  char *output_ptr = NULL;
  char *resized_output = NULL;

  *out_string = NULL;
  output = (char *)parson_malloc(initial_size);
  if (output == NULL) {
    LOG_DEBUG("OOM\n");
    goto error;
  }
  output_ptr = output;
  while ((*input_ptr != '\0') && (size_t)(input_ptr - input) < input_len) {
    if (*input_ptr == '\\') {
      input_ptr++;
      switch (*input_ptr) {
      case '\"':
        *output_ptr = '\"';
        break;
      case '\\':
        *output_ptr = '\\';
        break;
      case '/':
        *output_ptr = '/';
        break;
      case 'b':
        *output_ptr = '\b';
        break;
      case 'f':
        *output_ptr = '\f';
        break;
      case 'n':
        *output_ptr = '\n';
        break;
      case 'r':
        *output_ptr = '\r';
        break;
      case 't':
        *output_ptr = '\t';
        break;
      case 'u':
        rc = parse_utf16(&input_ptr, &output_ptr);
        if (rc != JSONSuccess) {
          LOG_DEBUG("Error: %d\n", (int)rc);
          goto error;
        }
        break;
      default:
        goto error;
      }
    } else if ((unsigned char)*input_ptr < 0x20) {
      goto error; /* 0x00-0x19 are invalid characters for json string */
    } else {
      *output_ptr = *input_ptr;
    }
    output_ptr++;
    input_ptr++;
  }
  *output_ptr = '\0';
  final_size = (size_t)(output_ptr - output) + 1;
  resized_output = (char *)parson_malloc(final_size);
  if (resized_output == NULL) {
    LOG_DEBUG("OOM\n");
    goto error;
  }
#if defined(_MSC_VER)
  memcpy_s(resized_output, final_size, output, final_size);
#else
  memcpy(resized_output, output, final_size);
#endif
  *output_len = final_size - 1;
  parson_free(output);
  *out_string = resized_output;
  return JSONSuccess;
error:
  parson_free(output);
  return JSONFailure;
}

static enum json_result_t get_quoted_string(const char **string,
                                            size_t *output_string_len,
                                            char **out_string) {
  enum json_result_t rc;
  const char *string_start = *string;
  size_t input_string_len = 0;

  *out_string = NULL;
  rc = skip_quotes(string);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    return rc;
  }
  input_string_len = (size_t)(*string - string_start) - 2;
  rc = process_string(string_start + 1, input_string_len, output_string_len,
                      out_string);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    return rc;
  }
  return JSONSuccess;
}

static enum json_result_t parse_value(const char **string, size_t nesting,
                                      JSON_Value **out_value) {
  *out_value = NULL;
  if (nesting > MAX_NESTING) {
    return JSONFailure;
  }
  SKIP_WHITESPACES(string);
  switch (**string) {
  case '{':
    return parse_object_value(string, nesting + 1, out_value);
  case '[':
    return parse_array_value(string, nesting + 1, out_value);
  case '\"':
    return parse_string_value(string, out_value);
  case 'f':
  case 't':
    return parse_boolean_value(string, out_value);
  case '-':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return parse_number_value(string, out_value);
  case 'n':
    return parse_null_value(string, out_value);
  default:
    return JSONFailure;
  }
}

static enum json_result_t parse_object_value(const char **string,
                                             size_t nesting,
                                             JSON_Value **out_value) {
  enum json_result_t rc;
  JSON_Value *output_value = NULL;
  JSON_Value *new_value = NULL;
  JSON_Object *output_object = NULL;
  char *new_key = NULL;

  *out_value = NULL;
  output_value = json_value_init_object();
  if (output_value == NULL) {
    return JSONFailure;
  }
  if (**string != '{') {
    json_value_free(output_value);
    return JSONFailure;
  }
  output_object = json_value_get_object(output_value);
  SKIP_CHAR(string);
  SKIP_WHITESPACES(string);
  if (**string == '}') { /* empty object */
    SKIP_CHAR(string);
    *out_value = output_value;
    return JSONSuccess;
  }
  while (**string != '\0') {
    size_t key_len = 0;
    rc = get_quoted_string(string, &key_len, &new_key);
    if (rc != JSONSuccess) {
      LOG_DEBUG("Error: %d\n", (int)rc);
      json_value_free(output_value);
      return rc;
    }
    /* We do not support key names with embedded \0 chars */
    if (key_len != strlen(new_key)) {
      parson_free(new_key);
      json_value_free(output_value);
      return JSONFailure;
    }
    SKIP_WHITESPACES(string);
    if (**string != ':') {
      parson_free(new_key);
      json_value_free(output_value);
      return JSONFailure;
    }
    SKIP_CHAR(string);
    rc = parse_value(string, nesting, &new_value);
    if (rc != JSONSuccess) {
      LOG_DEBUG("Error: %d\n", (int)rc);
      parson_free(new_key);
      json_value_free(output_value);
      return rc;
    }
    rc = json_object_add(output_object, new_key, new_value);
    if (rc != JSONSuccess) {
      LOG_DEBUG("Error: %d\n", (int)rc);
      parson_free(new_key);
      json_value_free(new_value);
      json_value_free(output_value);
      return rc;
    }
    SKIP_WHITESPACES(string);
    if (**string != ',') {
      break;
    }
    SKIP_CHAR(string);
    SKIP_WHITESPACES(string);
    if (**string == '}') {
      break;
    }
  }
  SKIP_WHITESPACES(string);
  if (**string != '}') {
    json_value_free(output_value);
    return JSONFailure;
  }
  SKIP_CHAR(string);
  *out_value = output_value;
  return JSONSuccess;
}

static enum json_result_t parse_array_value(const char **string, size_t nesting,
                                            JSON_Value **out_value) {
  enum json_result_t rc;
  JSON_Value *output_value = NULL;
  JSON_Value *new_array_value = NULL;
  JSON_Array *output_array = NULL;

  *out_value = NULL;
  output_value = json_value_init_array();
  if (output_value == NULL) {
    return JSONFailure;
  }
  if (**string != '[') {
    json_value_free(output_value);
    return JSONFailure;
  }
  output_array = json_value_get_array(output_value);
  SKIP_CHAR(string);
  SKIP_WHITESPACES(string);
  if (**string == ']') { /* empty array */
    SKIP_CHAR(string);
    *out_value = output_value;
    return JSONSuccess;
  }
  while (**string != '\0') {
    rc = parse_value(string, nesting, &new_array_value);
    if (rc != JSONSuccess) {
      LOG_DEBUG("Error: %d\n", (int)rc);
      json_value_free(output_value);
      return rc;
    }
    rc = json_array_add(output_array, new_array_value);
    if (rc != JSONSuccess) {
      LOG_DEBUG("Error: %d\n", (int)rc);
      json_value_free(new_array_value);
      json_value_free(output_value);
      return rc;
    }
    SKIP_WHITESPACES(string);
    if (**string != ',') {
      break;
    }
    SKIP_CHAR(string);
    SKIP_WHITESPACES(string);
    if (**string == ']') {
      break;
    }
  }
  SKIP_WHITESPACES(string);
  if (**string != ']') {
    json_value_free(output_value);
    return JSONFailure;
  }
  rc = json_array_resize(output_array, json_array_get_count(output_array));
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(output_value);
    return rc;
  }
  SKIP_CHAR(string);
  *out_value = output_value;
  return JSONSuccess;
}

static enum json_result_t parse_string_value(const char **string,
                                             JSON_Value **out_value) {
  enum json_result_t rc;
  JSON_Value *value = NULL;
  size_t new_string_len = 0;
  char *new_string = NULL;

  *out_value = NULL;
  rc = get_quoted_string(string, &new_string_len, &new_string);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    return rc;
  }
  rc = json_value_init_string_no_copy(new_string, new_string_len, &value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    parson_free(new_string);
    return rc;
  }
  *out_value = value;
  return JSONSuccess;
}

static enum json_result_t parse_boolean_value(const char **string,
                                              JSON_Value **out_value) {
  size_t true_token_size = SIZEOF_TOKEN("true");
  size_t false_token_size = SIZEOF_TOKEN("false");

  *out_value = NULL;
  if (strncmp("true", *string, true_token_size) == 0) {
    *string += true_token_size;
    *out_value = json_value_init_boolean(1);
    return *out_value != NULL ? JSONSuccess : JSONFailure;
  } else if (strncmp("false", *string, false_token_size) == 0) {
    *string += false_token_size;
    *out_value = json_value_init_boolean(0);
    return *out_value != NULL ? JSONSuccess : JSONFailure;
  }
  return JSONFailure;
}

static enum json_result_t parse_number_value(const char **string,
                                             JSON_Value **out_value) {
  enum json_result_t rc;
  char *end = NULL;
  double number = 0;

  *out_value = NULL;
  errno = 0;
  number = strtod(*string, &end);
  if (errno == ERANGE && (number <= -HUGE_VAL || number >= HUGE_VAL)) {
    return JSONFailure;
  }
  if (end == *string) {
    return JSONFailure;
  }
  rc = is_decimal(*string, (size_t)(end - *string));
  if (rc != JSONSuccess) {
    return rc;
  }
  *string = end;
  *out_value = json_value_init_number(number);
  return *out_value != NULL ? JSONSuccess : JSONFailure;
}

static enum json_result_t parse_null_value(const char **string,
                                           JSON_Value **out_value) {
  size_t token_size = SIZEOF_TOKEN("null");

  *out_value = NULL;
  if (strncmp("null", *string, token_size) == 0) {
    *string += token_size;
    *out_value = json_value_init_null();
    return *out_value != NULL ? JSONSuccess : JSONFailure;
  }
  return JSONFailure;
}

/* Serialization */
static void append_string(char **buf, const char *str, size_t len,
                          int *written_total) {
  if (*buf != NULL) {
#if defined(_MSC_VER)
    memcpy_s(*buf, len + 1, str, len);
#else
    memcpy(*buf, str, len);
#endif
    (*buf)[len] = '\0';
    *buf += len;
  }
  *written_total += (int)len;
}

static void append_indent(char **buf, int level, int *written_total) {
  int level_i = 0;
  for (level_i = 0; level_i < level; level_i++) {
    append_string(buf, PARSON_INDENT_STR, SIZEOF_TOKEN(PARSON_INDENT_STR),
                  written_total);
  }
}

/** \brief Macro to append string literal */
#define APPEND_STRING(str)                                                     \
  append_string(&buf, (str), SIZEOF_TOKEN(str), &written_total)

/** \brief Macro to append indentation */
#define APPEND_INDENT(lvl) append_indent(&buf, (lvl), &written_total)

static enum json_result_t json_serialize_to_buffer_r(const JSON_Value *value,
                                                     char *buf, int level,
                                                     parson_bool_t is_pretty,
                                                     char *num_buf,
                                                     int *out_written) {
  enum json_result_t rc;
  const char *key = NULL;
  const char *string = NULL;
  JSON_Value *temp_value = NULL;
  JSON_Array *array = NULL;
  JSON_Object *object = NULL;
  size_t i = 0;
  size_t count = 0;
  double num = 0.0;
  int written = -1;
  int written_total = 0;
  size_t len = 0;

  *out_written = -1;
  switch (json_value_get_type(value)) {
  case JSONArray:
    array = json_value_get_array(value);
    count = json_array_get_count(array);
    APPEND_STRING("[");
    if (count > 0 && is_pretty) {
      APPEND_STRING("\n");
    }
    for (i = 0; i < count; i++) {
      if (is_pretty) {
        APPEND_INDENT(level + 1);
      }
      temp_value = json_array_get_value(array, i);
      rc = json_serialize_to_buffer_r(temp_value, buf, level + 1, is_pretty,
                                      num_buf, &written);
      if (rc != JSONSuccess) {
        LOG_DEBUG("Error: %d\n", (int)rc);
        return rc;
      }
      if (buf != NULL) {
        buf += written;
      }
      written_total += written;
      if (i < (count - 1)) {
        APPEND_STRING(",");
      }
      if (is_pretty) {
        APPEND_STRING("\n");
      }
    }
    if (count > 0 && is_pretty) {
      APPEND_INDENT(level);
    }
    APPEND_STRING("]");
    *out_written = written_total;
    return JSONSuccess;
  case JSONObject:
    object = json_value_get_object(value);
    count = json_object_get_count(object);
    APPEND_STRING("{");
    if (count > 0 && is_pretty) {
      APPEND_STRING("\n");
    }
    for (i = 0; i < count; i++) {
      key = json_object_get_name(object, i);
      if (is_pretty) {
        APPEND_INDENT(level + 1);
      }
      rc = json_serialize_string(key, key != NULL ? strlen(key) : 0, buf,
                                 &written);
      if (rc != JSONSuccess) {
        LOG_DEBUG("Error: %d\n", (int)rc);
        return rc;
      }
      if (buf != NULL) {
        buf += written;
      }
      written_total += written;
      APPEND_STRING(":");
      if (is_pretty) {
        APPEND_STRING(" ");
      }
      temp_value = json_object_get_value_at(object, i);
      rc = json_serialize_to_buffer_r(temp_value, buf, level + 1, is_pretty,
                                      num_buf, &written);
      if (rc != JSONSuccess) {
        LOG_DEBUG("Error: %d\n", (int)rc);
        return rc;
      }
      if (buf != NULL) {
        buf += written;
      }
      written_total += written;
      if (i < (count - 1)) {
        APPEND_STRING(",");
      }
      if (is_pretty) {
        APPEND_STRING("\n");
      }
    }
    if (count > 0 && is_pretty) {
      APPEND_INDENT(level);
    }
    APPEND_STRING("}");
    *out_written = written_total;
    return JSONSuccess;
  case JSONString:
    string = json_value_get_string(value);
    len = json_value_get_string_len(value);
    rc = json_serialize_string(string, len, buf, &written);
    if (rc != JSONSuccess) {
      LOG_DEBUG("Error: %d\n", (int)rc);
      return rc;
    }
    if (buf != NULL) {
      buf += written;
    }
    written_total += written;
    *out_written = written_total;
    return JSONSuccess;
  case JSONBoolean:
    if (json_value_get_boolean(value)) {
      APPEND_STRING("true");
    } else {
      APPEND_STRING("false");
    }
    *out_written = written_total;
    return JSONSuccess;
  case JSONNumber:
    num = json_value_get_number(value);
    if (parson_number_serialization_function) {
      written = parson_number_serialization_function(num, num_buf);
      if (written < 0) {
        return JSONFailure;
      }
    } else {
      rc = parson_format_double(num_buf, PARSON_NUM_BUF_SIZE,
                                parson_float_format, num, &written);
      if (rc != JSONSuccess) {
        LOG_DEBUG("Error: %d\n", (int)rc);
        return rc;
      }
    }
    if (buf != NULL) {
#if defined(_MSC_VER)
      memcpy_s(buf, (size_t)written + 1, num_buf, (size_t)written);
#else
      memcpy(buf, num_buf, (size_t)written);
#endif
      buf[written] = '\0';
      buf += written;
    }
    written_total += written;
    *out_written = written_total;
    return JSONSuccess;
  case JSONNull:
    APPEND_STRING("null");
    *out_written = written_total;
    return JSONSuccess;
  case JSONError:
  default:
    return JSONFailure;
  }
}

static enum json_result_t json_serialize_string(const char *string, size_t len,
                                                char *buf, int *out_written) {
  size_t i = 0;
  char c = '\0';
  int written_total = 0;

  *out_written = -1;
  if (string == NULL) {
    return JSONFailure;
  }
  APPEND_STRING("\"");
  for (i = 0; i < len; i++) {
    c = string[i];
    switch (c) {
    case '\"':
      APPEND_STRING("\\\"");
      break;
    case '\\':
      APPEND_STRING("\\\\");
      break;
    case '\b':
      APPEND_STRING("\\b");
      break;
    case '\f':
      APPEND_STRING("\\f");
      break;
    case '\n':
      APPEND_STRING("\\n");
      break;
    case '\r':
      APPEND_STRING("\\r");
      break;
    case '\t':
      APPEND_STRING("\\t");
      break;
    case '\x00':
      APPEND_STRING("\\u0000");
      break;
    case '\x01':
      APPEND_STRING("\\u0001");
      break;
    case '\x02':
      APPEND_STRING("\\u0002");
      break;
    case '\x03':
      APPEND_STRING("\\u0003");
      break;
    case '\x04':
      APPEND_STRING("\\u0004");
      break;
    case '\x05':
      APPEND_STRING("\\u0005");
      break;
    case '\x06':
      APPEND_STRING("\\u0006");
      break;
    case '\x07':
      APPEND_STRING("\\u0007");
      break;
    /* '\x08' duplicate: '\b' */
    /* '\x09' duplicate: '\t' */
    /* '\x0a' duplicate: '\n' */
    case '\x0b':
      APPEND_STRING("\\u000b");
      break;
    /* '\x0c' duplicate: '\f' */
    /* '\x0d' duplicate: '\r' */
    case '\x0e':
      APPEND_STRING("\\u000e");
      break;
    case '\x0f':
      APPEND_STRING("\\u000f");
      break;
    case '\x10':
      APPEND_STRING("\\u0010");
      break;
    case '\x11':
      APPEND_STRING("\\u0011");
      break;
    case '\x12':
      APPEND_STRING("\\u0012");
      break;
    case '\x13':
      APPEND_STRING("\\u0013");
      break;
    case '\x14':
      APPEND_STRING("\\u0014");
      break;
    case '\x15':
      APPEND_STRING("\\u0015");
      break;
    case '\x16':
      APPEND_STRING("\\u0016");
      break;
    case '\x17':
      APPEND_STRING("\\u0017");
      break;
    case '\x18':
      APPEND_STRING("\\u0018");
      break;
    case '\x19':
      APPEND_STRING("\\u0019");
      break;
    case '\x1a':
      APPEND_STRING("\\u001a");
      break;
    case '\x1b':
      APPEND_STRING("\\u001b");
      break;
    case '\x1c':
      APPEND_STRING("\\u001c");
      break;
    case '\x1d':
      APPEND_STRING("\\u001d");
      break;
    case '\x1e':
      APPEND_STRING("\\u001e");
      break;
    case '\x1f':
      APPEND_STRING("\\u001f");
      break;
    case '/':
      if (parson_escape_slashes) {
        APPEND_STRING("\\/"); /* to make json embeddable in xml\/html */
      } else {
        APPEND_STRING("/");
      }
      break;
    default:
      if (buf != NULL) {
        buf[0] = c;
        buf += 1;
      }
      written_total += 1;
      break;
    }
  }
  APPEND_STRING("\"");
  *out_written = written_total;
  return JSONSuccess;
}

#undef APPEND_STRING
#undef APPEND_INDENT

/* Parser API */
/* Implementation */
PARSON_API JSON_Value *json_parse_file(const char *filename) {
  enum json_result_t rc;
  char *file_contents = NULL;
  JSON_Value *output_value = NULL;
  rc = read_file(filename, &file_contents);
  if (rc != JSONSuccess) {
    return NULL;
  }
  output_value = json_parse_string(file_contents);
  parson_free(file_contents);
  return output_value;
}

/* Implementation */
PARSON_API JSON_Value *json_parse_file_with_comments(const char *filename) {
  enum json_result_t rc;
  char *file_contents = NULL;
  JSON_Value *output_value = NULL;
  rc = read_file(filename, &file_contents);
  if (rc != JSONSuccess) {
    return NULL;
  }
  output_value = json_parse_string_with_comments(file_contents);
  parson_free(file_contents);
  return output_value;
}

/* Implementation */
PARSON_API JSON_Value *json_parse_string(const char *string) {
  enum json_result_t rc;
  JSON_Value *output_value = NULL;
  if (string == NULL) {
    return NULL;
  }
  if (string[0] == '\xEF' && string[1] == '\xBB' && string[2] == '\xBF') {
    string = string + 3; /* Support for UTF-8 BOM */
  }
  rc = parse_value(&string, 0, &output_value);
  if (rc != JSONSuccess) {
    return NULL;
  }
  return output_value;
}

/* Implementation */
PARSON_API JSON_Value *json_parse_string_with_comments(const char *string) {
  enum json_result_t rc;
  JSON_Value *result = NULL;
  char *string_mutable_copy = NULL;
  const char *string_mutable_copy_ptr = NULL;
  if (string == NULL) {
    return NULL;
  }
  rc = parson_strdup(string, &string_mutable_copy);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    return NULL;
  }
  remove_comments(string_mutable_copy, "/*", "*/");
  remove_comments(string_mutable_copy, "//", "\n");
  string_mutable_copy_ptr = string_mutable_copy;
  rc = parse_value(&string_mutable_copy_ptr, 0, &result);
  parson_free(string_mutable_copy);
  if (rc != JSONSuccess) {
    return NULL;
  }
  return result;
}

/* JSON Object API */

/* Implementation */
PARSON_API JSON_Value *json_object_get_value(const JSON_Object *object,
                                             const char *name) {
  enum json_result_t rc;
  JSON_Value *val = NULL;
  if (object == NULL || name == NULL) {
    return NULL;
  }
  rc = json_object_getn_value(object, name, strlen(name), &val);
  if (rc != JSONSuccess) {
    return NULL;
  }
  return val;
}

/* Implementation */
PARSON_API const char *json_object_get_string(const JSON_Object *object,
                                              const char *name) {
  return json_value_get_string(json_object_get_value(object, name));
}

/* Implementation */
PARSON_API size_t json_object_get_string_len(const JSON_Object *object,
                                             const char *name) {
  return json_value_get_string_len(json_object_get_value(object, name));
}

/* Implementation */
PARSON_API double json_object_get_number(const JSON_Object *object,
                                         const char *name) {
  return json_value_get_number(json_object_get_value(object, name));
}

/* Implementation */
PARSON_API JSON_Object *json_object_get_object(const JSON_Object *object,
                                               const char *name) {
  return json_value_get_object(json_object_get_value(object, name));
}

/* Implementation */
PARSON_API JSON_Array *json_object_get_array(const JSON_Object *object,
                                             const char *name) {
  return json_value_get_array(json_object_get_value(object, name));
}

/* Implementation */
PARSON_API int json_object_get_boolean(const JSON_Object *object,
                                       const char *name) {
  return json_value_get_boolean(json_object_get_value(object, name));
}

/* Implementation */
PARSON_API JSON_Value *json_object_dotget_value(const JSON_Object *object,
                                                const char *name) {
  enum json_result_t rc;
  const char *dot_position = NULL;
  JSON_Value *sub_val = NULL;
  if (object == NULL || name == NULL) {
    return NULL;
  }
  dot_position = strchr(name, '.');
  if (!dot_position) {
    return json_object_get_value(object, name);
  }
  rc = json_object_getn_value(object, name, (size_t)(dot_position - name),
                              &sub_val);
  if (rc != JSONSuccess) {
    return NULL;
  }
  object = json_value_get_object(sub_val);
  return json_object_dotget_value(object, dot_position + 1);
}

/* Implementation */
PARSON_API const char *json_object_dotget_string(const JSON_Object *object,
                                                 const char *name) {
  return json_value_get_string(json_object_dotget_value(object, name));
}

/* Implementation */
PARSON_API size_t json_object_dotget_string_len(const JSON_Object *object,
                                                const char *name) {
  return json_value_get_string_len(json_object_dotget_value(object, name));
}

/* Implementation */
PARSON_API double json_object_dotget_number(const JSON_Object *object,
                                            const char *name) {
  return json_value_get_number(json_object_dotget_value(object, name));
}

/* Implementation */
PARSON_API JSON_Object *json_object_dotget_object(const JSON_Object *object,
                                                  const char *name) {
  return json_value_get_object(json_object_dotget_value(object, name));
}

/* Implementation */
PARSON_API JSON_Array *json_object_dotget_array(const JSON_Object *object,
                                                const char *name) {
  return json_value_get_array(json_object_dotget_value(object, name));
}

/* Implementation */
PARSON_API int json_object_dotget_boolean(const JSON_Object *object,
                                          const char *name) {
  return json_value_get_boolean(json_object_dotget_value(object, name));
}

/* Implementation */
PARSON_API size_t json_object_get_count(const JSON_Object *object) {
  return object ? object->count : 0;
}

/* Implementation */
PARSON_API const char *json_object_get_name(const JSON_Object *object,
                                            size_t index) {
  if (object == NULL || index >= json_object_get_count(object)) {
    return NULL;
  }
  return object->names[index];
}

/* Implementation */
PARSON_API JSON_Value *json_object_get_value_at(const JSON_Object *object,
                                                size_t index) {
  if (object == NULL || index >= json_object_get_count(object)) {
    return NULL;
  }
  return object->values[index];
}

/* Implementation */
PARSON_API JSON_Value *
json_object_get_wrapping_value(const JSON_Object *object) {
  if (!object) {
    return NULL;
  }
  return object->wrapping_value;
}

/* Implementation */
PARSON_API int json_object_has_value(const JSON_Object *object,
                                     const char *name) {
  return json_object_get_value(object, name) != NULL;
}

/* Implementation */
PARSON_API int json_object_has_value_of_type(const JSON_Object *object,
                                             const char *name,
                                             JSON_Value_Type type) {
  JSON_Value *val = json_object_get_value(object, name);
  return val != NULL && json_value_get_type(val) == type;
}

/* Implementation */
PARSON_API int json_object_dothas_value(const JSON_Object *object,
                                        const char *name) {
  return json_object_dotget_value(object, name) != NULL;
}

/* Implementation */
PARSON_API int json_object_dothas_value_of_type(const JSON_Object *object,
                                                const char *name,
                                                JSON_Value_Type type) {
  JSON_Value *val = json_object_dotget_value(object, name);
  return val != NULL && json_value_get_type(val) == type;
}

/* JSON Array API */
/* Implementation */
PARSON_API JSON_Value *json_array_get_value(const JSON_Array *array,
                                            size_t index) {
  if (array == NULL || index >= json_array_get_count(array)) {
    return NULL;
  }
  return array->items[index];
}

/* Implementation */
PARSON_API const char *json_array_get_string(const JSON_Array *array,
                                             size_t index) {
  return json_value_get_string(json_array_get_value(array, index));
}

/* Implementation */
PARSON_API size_t json_array_get_string_len(const JSON_Array *array,
                                            size_t index) {
  return json_value_get_string_len(json_array_get_value(array, index));
}

/* Implementation */
PARSON_API double json_array_get_number(const JSON_Array *array, size_t index) {
  return json_value_get_number(json_array_get_value(array, index));
}

/* Implementation */
PARSON_API JSON_Object *json_array_get_object(const JSON_Array *array,
                                              size_t index) {
  return json_value_get_object(json_array_get_value(array, index));
}

/* Implementation */
PARSON_API JSON_Array *json_array_get_array(const JSON_Array *array,
                                            size_t index) {
  return json_value_get_array(json_array_get_value(array, index));
}

/* Implementation */
PARSON_API int json_array_get_boolean(const JSON_Array *array, size_t index) {
  return json_value_get_boolean(json_array_get_value(array, index));
}

/* Implementation */
PARSON_API size_t json_array_get_count(const JSON_Array *array) {
  return array ? array->count : 0;
}

/* Implementation */
PARSON_API JSON_Value *json_array_get_wrapping_value(const JSON_Array *array) {
  if (!array) {
    return NULL;
  }
  return array->wrapping_value;
}

/* JSON Value API */
/* Implementation */
PARSON_API JSON_Value_Type json_value_get_type(const JSON_Value *value) {
  return value ? value->type : JSONError;
}

/* Implementation */
PARSON_API JSON_Object *json_value_get_object(const JSON_Value *value) {
  return json_value_get_type(value) == JSONObject ? value->value.object : NULL;
}

/* Implementation */
PARSON_API JSON_Array *json_value_get_array(const JSON_Value *value) {
  return json_value_get_type(value) == JSONArray ? value->value.array : NULL;
}

static enum json_result_t
json_value_get_string_desc(const JSON_Value *value,
                           const JSON_String **out_desc) {
  if (json_value_get_type(value) == JSONString &&
      value->value.string.chars != NULL) {
    *out_desc = &value->value.string;
    return JSONSuccess;
  }
  *out_desc = NULL;
  return JSONFailure;
}

/* Implementation */
PARSON_API const char *json_value_get_string(const JSON_Value *value) {
  enum json_result_t rc;
  const JSON_String *str = NULL;
  rc = json_value_get_string_desc(value, &str);
  return rc == JSONSuccess ? str->chars : NULL;
}

/* Implementation */
PARSON_API size_t json_value_get_string_len(const JSON_Value *value) {
  enum json_result_t rc;
  const JSON_String *str = NULL;
  rc = json_value_get_string_desc(value, &str);
  return rc == JSONSuccess ? str->length : 0;
}

/* Implementation */
PARSON_API double json_value_get_number(const JSON_Value *value) {
  return json_value_get_type(value) == JSONNumber ? value->value.number : 0;
}

/* Implementation */
PARSON_API int json_value_get_boolean(const JSON_Value *value) {
  return json_value_get_type(value) == JSONBoolean ? value->value.boolean : -1;
}

/* Implementation */
PARSON_API JSON_Value *json_value_get_parent(const JSON_Value *value) {
  return value ? value->parent : NULL;
}

/* Implementation */
PARSON_API void json_value_free(JSON_Value *value) {
  switch (json_value_get_type(value)) {
  case JSONObject:
    json_object_free(value->value.object);
    break;
  case JSONString:
    parson_free(value->value.string.chars);
    break;
  case JSONArray:
    json_array_free(value->value.array);
    break;
  default:
    break;
  }
  parson_free(value);
}

/* Implementation */
PARSON_API JSON_Value *json_value_init_object(void) {
  enum json_result_t rc;
  JSON_Value *new_value = (JSON_Value *)parson_malloc(sizeof(JSON_Value));
  if (!new_value) {
    LOG_DEBUG("OOM\n");
    return NULL;
  }
  new_value->parent = NULL;
  new_value->type = JSONObject;
  rc = json_object_make(new_value, &new_value->value.object);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    parson_free(new_value);
    return NULL;
  }
  return new_value;
}

/* Implementation */
PARSON_API JSON_Value *json_value_init_array(void) {
  enum json_result_t rc;
  JSON_Value *new_value = (JSON_Value *)parson_malloc(sizeof(JSON_Value));
  if (!new_value) {
    LOG_DEBUG("OOM\n");
    return NULL;
  }
  new_value->parent = NULL;
  new_value->type = JSONArray;
  rc = json_array_make(new_value, &new_value->value.array);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    parson_free(new_value);
    return NULL;
  }
  return new_value;
}

/* Implementation */
PARSON_API JSON_Value *json_value_init_string(const char *string) {
  if (string == NULL) {
    return NULL;
  }
  return json_value_init_string_with_len(string, strlen(string));
}

/* Implementation */
PARSON_API JSON_Value *json_value_init_string_with_len(const char *string,
                                                       size_t length) {
  enum json_result_t rc;
  char *copy = NULL;
  JSON_Value *value = NULL;
  if (string == NULL) {
    return NULL;
  }
  rc = is_valid_utf8(string, length);
  if (rc != JSONSuccess) {
    return NULL;
  }
  rc = parson_strndup(string, length, &copy);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    return NULL;
  }
  rc = json_value_init_string_no_copy(copy, length, &value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    parson_free(copy);
    return NULL;
  }
  return value;
}

/* Implementation */
PARSON_API JSON_Value *json_value_init_number(double number) {
  JSON_Value *new_value = NULL;
  if (IS_NUMBER_INVALID(number)) {
    return NULL;
  }
  new_value = (JSON_Value *)parson_malloc(sizeof(JSON_Value));
  if (new_value == NULL) {
    LOG_DEBUG("OOM\n");
    return NULL;
  }
  new_value->parent = NULL;
  new_value->type = JSONNumber;
  new_value->value.number = number;
  return new_value;
}

/* Implementation */
PARSON_API JSON_Value *json_value_init_boolean(int boolean) {
  JSON_Value *new_value = (JSON_Value *)parson_malloc(sizeof(JSON_Value));
  if (!new_value) {
    LOG_DEBUG("OOM\n");
    return NULL;
  }
  new_value->parent = NULL;
  new_value->type = JSONBoolean;
  new_value->value.boolean = boolean ? 1 : 0;
  return new_value;
}

/* Implementation */
PARSON_API JSON_Value *json_value_init_null(void) {
  JSON_Value *new_value = (JSON_Value *)parson_malloc(sizeof(JSON_Value));
  if (!new_value) {
    LOG_DEBUG("OOM\n");
    return NULL;
  }
  new_value->parent = NULL;
  new_value->type = JSONNull;
  return new_value;
}

/* Implementation */
PARSON_API JSON_Value *json_value_deep_copy(const JSON_Value *value) {
  enum json_result_t rc;
  size_t i = 0;
  JSON_Value *return_value = NULL;
  JSON_Value *temp_value_copy = NULL;
  JSON_Value *temp_value = NULL;
  const JSON_String *temp_string = NULL;
  const char *temp_key = NULL;
  char *temp_string_copy = NULL;
  JSON_Array *temp_array = NULL;
  JSON_Array *temp_array_copy = NULL;
  JSON_Object *temp_object = NULL;
  JSON_Object *temp_object_copy = NULL;
  char *key_copy = NULL;

  switch (json_value_get_type(value)) {
  case JSONArray:
    temp_array = json_value_get_array(value);
    return_value = json_value_init_array();
    if (return_value == NULL) {
      return NULL;
    }
    temp_array_copy = json_value_get_array(return_value);
    for (i = 0; i < json_array_get_count(temp_array); i++) {
      temp_value = json_array_get_value(temp_array, i);
      temp_value_copy = json_value_deep_copy(temp_value);
      if (temp_value_copy == NULL) {
        json_value_free(return_value);
        return NULL;
      }
      rc = json_array_add(temp_array_copy, temp_value_copy);
      if (rc != JSONSuccess) {
        LOG_DEBUG("Error: %d\n", (int)rc);
        json_value_free(return_value);
        json_value_free(temp_value_copy);
        return NULL;
      }
    }
    return return_value;
  case JSONObject:
    temp_object = json_value_get_object(value);
    return_value = json_value_init_object();
    if (!return_value) {
      return NULL;
    }
    temp_object_copy = json_value_get_object(return_value);
    for (i = 0; i < json_object_get_count(temp_object); i++) {
      temp_key = json_object_get_name(temp_object, i);
      temp_value = json_object_get_value(temp_object, temp_key);
      temp_value_copy = json_value_deep_copy(temp_value);
      if (!temp_value_copy) {
        json_value_free(return_value);
        return NULL;
      }
      rc = parson_strdup(temp_key, &key_copy);
      if (rc != JSONSuccess) {
        LOG_DEBUG("Error: %d\n", (int)rc);
        json_value_free(temp_value_copy);
        json_value_free(return_value);
        return NULL;
      }
      rc = json_object_add(temp_object_copy, key_copy, temp_value_copy);
      if (rc != JSONSuccess) {
        LOG_DEBUG("Error: %d\n", (int)rc);
        parson_free(key_copy);
        json_value_free(temp_value_copy);
        json_value_free(return_value);
        return NULL;
      }
    }
    return return_value;
  case JSONBoolean:
    return json_value_init_boolean(json_value_get_boolean(value));
  case JSONNumber:
    return json_value_init_number(json_value_get_number(value));
  case JSONString:
    rc = json_value_get_string_desc(value, &temp_string);
    if (rc != JSONSuccess) {
      return NULL;
    }
    rc = parson_strndup(temp_string->chars, temp_string->length,
                        &temp_string_copy);
    if (rc != JSONSuccess) {
      LOG_DEBUG("Error: %d\n", (int)rc);
      return NULL;
    }
    rc = json_value_init_string_no_copy(temp_string_copy, temp_string->length,
                                        &return_value);
    if (rc != JSONSuccess) {
      LOG_DEBUG("Error: %d\n", (int)rc);
      parson_free(temp_string_copy);
      return NULL;
    }
    return return_value;
  case JSONNull:
    return json_value_init_null();
  case JSONError:
  default:
    return NULL;
  }
}

/* Implementation */
PARSON_API size_t json_serialization_size(const JSON_Value *value) {
  enum json_result_t rc;
  char num_buf[PARSON_NUM_BUF_SIZE];
  int res = -1;
  rc = json_serialize_to_buffer_r(value, NULL, 0, PARSON_FALSE, num_buf, &res);
  return rc != JSONSuccess ? 0 : (size_t)res + 1;
}

/* Implementation */
PARSON_API JSON_Status json_serialize_to_buffer(const JSON_Value *value,
                                                char *buf,
                                                size_t buf_size_in_bytes) {
  enum json_result_t rc;
  int written = -1;
  size_t needed_size_in_bytes = json_serialization_size(value);
  char num_buf[PARSON_NUM_BUF_SIZE];
  if (buf == NULL || needed_size_in_bytes == 0 ||
      buf_size_in_bytes < needed_size_in_bytes) {
    return JSONFailure;
  }
  rc = json_serialize_to_buffer_r(value, buf, 0, PARSON_FALSE, num_buf,
                                  &written);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    return JSONFailure;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_serialize_to_file(const JSON_Value *value,
                                              const char *filename) {
  enum json_result_t rc;
  JSON_Status return_code = JSONSuccess;
  FILE *fp = NULL;
  char *serialized_string = NULL;
  if (filename == NULL) {
    return JSONFailure;
  }
  serialized_string = json_serialize_to_string(value);
  if (serialized_string == NULL) {
    return JSONFailure;
  }
  rc = parson_fopen(filename, "w", &fp);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_free_serialized_string(serialized_string);
    return JSONFailure;
  }
  if (PARSON_FPUTS(serialized_string, fp) == EOF) {
    return_code = JSONFailure;
  }
  if (PARSON_FCLOSE(fp) == EOF) {
    return_code = JSONFailure;
  }
  json_free_serialized_string(serialized_string);
  return return_code;
}

/* Implementation */
PARSON_API char *json_serialize_to_string(const JSON_Value *value) {
  JSON_Status serialization_result = JSONFailure;
  size_t buf_size_bytes = json_serialization_size(value);
  char *buf = NULL;
  if (buf_size_bytes == 0) {
    return NULL;
  }
  buf = (char *)parson_malloc(buf_size_bytes);
  if (buf == NULL) {
    LOG_DEBUG("OOM\n");
    return NULL;
  }
  serialization_result = json_serialize_to_buffer(value, buf, buf_size_bytes);
  if (serialization_result != JSONSuccess) {
    json_free_serialized_string(buf);
    return NULL;
  }
  return buf;
}

/* Implementation */
PARSON_API size_t json_serialization_size_pretty(const JSON_Value *value) {
  enum json_result_t rc;
  char num_buf[PARSON_NUM_BUF_SIZE];
  int res = -1;
  rc = json_serialize_to_buffer_r(value, NULL, 0, PARSON_TRUE, num_buf, &res);
  return rc != JSONSuccess ? 0 : (size_t)res + 1;
}

/* Implementation */
PARSON_API JSON_Status json_serialize_to_buffer_pretty(
    const JSON_Value *value, char *buf, size_t buf_size_in_bytes) {
  enum json_result_t rc;
  int written = -1;
  size_t needed_size_in_bytes = json_serialization_size_pretty(value);
  char num_buf[PARSON_NUM_BUF_SIZE];
  if (buf == NULL || needed_size_in_bytes == 0 ||
      buf_size_in_bytes < needed_size_in_bytes) {
    return JSONFailure;
  }
  rc =
      json_serialize_to_buffer_r(value, buf, 0, PARSON_TRUE, num_buf, &written);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    return JSONFailure;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_serialize_to_file_pretty(const JSON_Value *value,
                                                     const char *filename) {
  enum json_result_t rc;
  JSON_Status return_code = JSONSuccess;
  FILE *fp = NULL;
  char *serialized_string = NULL;
  if (filename == NULL) {
    return JSONFailure;
  }
  serialized_string = json_serialize_to_string_pretty(value);
  if (serialized_string == NULL) {
    return JSONFailure;
  }
  rc = parson_fopen(filename, "w", &fp);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_free_serialized_string(serialized_string);
    return JSONFailure;
  }
  if (PARSON_FPUTS(serialized_string, fp) == EOF) {
    return_code = JSONFailure;
  }
  if (PARSON_FCLOSE(fp) == EOF) {
    return_code = JSONFailure;
  }
  json_free_serialized_string(serialized_string);
  return return_code;
}

/* Implementation */
PARSON_API char *json_serialize_to_string_pretty(const JSON_Value *value) {
  JSON_Status serialization_result = JSONFailure;
  size_t buf_size_bytes = json_serialization_size_pretty(value);
  char *buf = NULL;
  if (buf_size_bytes == 0) {
    return NULL;
  }
  buf = (char *)parson_malloc(buf_size_bytes);
  if (buf == NULL) {
    LOG_DEBUG("OOM\n");
    return NULL;
  }
  serialization_result =
      json_serialize_to_buffer_pretty(value, buf, buf_size_bytes);
  if (serialization_result != JSONSuccess) {
    json_free_serialized_string(buf);
    return NULL;
  }
  return buf;
}

/* Implementation */
PARSON_API void json_free_serialized_string(char *string) {
  parson_free(string);
}

/* Implementation */
PARSON_API JSON_Status json_array_remove(JSON_Array *array, size_t ix) {
  size_t to_move_bytes = 0;
  if (array == NULL || ix >= json_array_get_count(array)) {
    return JSONFailure;
  }
  json_value_free(json_array_get_value(array, ix));
  to_move_bytes = (json_array_get_count(array) - 1 - ix) * sizeof(JSON_Value *);
  if (to_move_bytes > 0) {
#if defined(_MSC_VER)
    memmove_s(array->items + ix, (array->capacity - ix) * sizeof(JSON_Value *),
              array->items + ix + 1, to_move_bytes);
#else
    memmove(array->items + ix, array->items + ix + 1, to_move_bytes);
#endif
  }
  array->count -= 1;
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_array_replace_value(JSON_Array *array, size_t ix,
                                                JSON_Value *value) {
  if (array == NULL || value == NULL || value->parent != NULL ||
      ix >= json_array_get_count(array)) {
    return JSONFailure;
  }
  json_value_free(json_array_get_value(array, ix));
  value->parent = json_array_get_wrapping_value(array);
  array->items[ix] = value;
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_array_replace_string(JSON_Array *array, size_t i,
                                                 const char *string) {
  JSON_Status rc = JSONFailure;
  JSON_Value *value = json_value_init_string(string);
  if (value == NULL) {
    return JSONFailure;
  }
  rc = json_array_replace_value(array, i, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
    return rc;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_array_replace_string_with_len(JSON_Array *array,
                                                          size_t i,
                                                          const char *string,
                                                          size_t len) {
  JSON_Status rc = JSONFailure;
  JSON_Value *value = json_value_init_string_with_len(string, len);
  if (value == NULL) {
    return JSONFailure;
  }
  rc = json_array_replace_value(array, i, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
    return rc;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_array_replace_number(JSON_Array *array, size_t i,
                                                 double number) {
  JSON_Status rc = JSONFailure;
  JSON_Value *value = json_value_init_number(number);
  if (value == NULL) {
    return JSONFailure;
  }
  rc = json_array_replace_value(array, i, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
    return rc;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_array_replace_boolean(JSON_Array *array, size_t i,
                                                  int boolean) {
  JSON_Status rc = JSONFailure;
  JSON_Value *value = json_value_init_boolean(boolean);
  if (value == NULL) {
    return JSONFailure;
  }
  rc = json_array_replace_value(array, i, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
    return rc;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_array_replace_null(JSON_Array *array, size_t i) {
  JSON_Status rc = JSONFailure;
  JSON_Value *value = json_value_init_null();
  if (value == NULL) {
    return JSONFailure;
  }
  rc = json_array_replace_value(array, i, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
    return rc;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_array_clear(JSON_Array *array) {
  size_t i = 0;
  if (array == NULL) {
    return JSONFailure;
  }
  for (i = 0; i < json_array_get_count(array); i++) {
    json_value_free(json_array_get_value(array, i));
  }
  array->count = 0;
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_array_append_value(JSON_Array *array,
                                               JSON_Value *value) {
  if (array == NULL || value == NULL || value->parent != NULL) {
    return JSONFailure;
  }
  return json_array_add(array, value);
}

/* Implementation */
PARSON_API JSON_Status json_array_append_string(JSON_Array *array,
                                                const char *string) {
  JSON_Status rc = JSONFailure;
  JSON_Value *value = json_value_init_string(string);
  if (value == NULL) {
    return JSONFailure;
  }
  rc = json_array_append_value(array, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
    return rc;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_array_append_string_with_len(JSON_Array *array,
                                                         const char *string,
                                                         size_t len) {
  JSON_Status rc = JSONFailure;
  JSON_Value *value = json_value_init_string_with_len(string, len);
  if (value == NULL) {
    return JSONFailure;
  }
  rc = json_array_append_value(array, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
    return rc;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_array_append_number(JSON_Array *array,
                                                double number) {
  JSON_Status rc = JSONFailure;
  JSON_Value *value = json_value_init_number(number);
  if (value == NULL) {
    return JSONFailure;
  }
  rc = json_array_append_value(array, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
    return rc;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_array_append_boolean(JSON_Array *array,
                                                 int boolean) {
  JSON_Status rc = JSONFailure;
  JSON_Value *value = json_value_init_boolean(boolean);
  if (value == NULL) {
    return JSONFailure;
  }
  rc = json_array_append_value(array, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
    return rc;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_array_append_null(JSON_Array *array) {
  JSON_Status rc = JSONFailure;
  JSON_Value *value = json_value_init_null();
  if (value == NULL) {
    return JSONFailure;
  }
  rc = json_array_append_value(array, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
    return rc;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_object_set_value(JSON_Object *object,
                                             const char *name,
                                             JSON_Value *value) {
  enum json_result_t rc = JSONFailure;
  unsigned long hash = 0;
  parson_bool_t found = PARSON_FALSE;
  size_t cell_ix = 0;
  size_t item_ix = 0;
  size_t name_len = 0;
  JSON_Value *old_value = NULL;
  char *key_copy = NULL;

  if (!object || !name || !value || value->parent) {
    return JSONFailure;
  }
  name_len = strlen(name);
  hash = hash_string(name, name_len);
  if (object->cell_capacity > 0) {
    rc =
        json_object_get_cell_ix(object, name, name_len, hash, &found, &cell_ix);
    if (rc == JSONSuccess && found) {
      item_ix = object->cells[cell_ix];
      old_value = object->values[item_ix];
      json_value_free(old_value);
      object->values[item_ix] = value;
      value->parent = json_object_get_wrapping_value(object);
      return JSONSuccess;
    }
  }
  if (object->count >= object->item_capacity) {
    rc = json_object_grow_and_rehash(object);
    if (rc != JSONSuccess) {
      LOG_DEBUG("Error: %d\n", (int)rc);
      return JSONFailure;
    }
    rc =
        json_object_get_cell_ix(object, name, name_len, hash, &found, &cell_ix);
  }
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    return JSONFailure;
  }
  rc = parson_strdup(name, &key_copy);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    return JSONFailure;
  }
  object->names[object->count] = key_copy;
  object->cells[cell_ix] = object->count;
  object->values[object->count] = value;
  object->cell_ixs[object->count] = cell_ix;
  object->hashes[object->count] = hash;
  object->count++;
  value->parent = json_object_get_wrapping_value(object);
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_object_set_string(JSON_Object *object,
                                              const char *name,
                                              const char *string) {
  JSON_Value *value = json_value_init_string(string);
  JSON_Status rc = json_object_set_value(object, name, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
  }
  return rc;
}

/* Implementation */
PARSON_API JSON_Status json_object_set_string_with_len(JSON_Object *object,
                                                       const char *name,
                                                       const char *string,
                                                       size_t len) {
  JSON_Value *value = json_value_init_string_with_len(string, len);
  JSON_Status rc = json_object_set_value(object, name, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
  }
  return rc;
}

/* Implementation */
PARSON_API JSON_Status json_object_set_number(JSON_Object *object,
                                              const char *name, double number) {
  JSON_Value *value = json_value_init_number(number);
  JSON_Status rc = json_object_set_value(object, name, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
  }
  return rc;
}

/* Implementation */
PARSON_API JSON_Status json_object_set_boolean(JSON_Object *object,
                                               const char *name, int boolean) {
  JSON_Value *value = json_value_init_boolean(boolean);
  JSON_Status rc = json_object_set_value(object, name, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
  }
  return rc;
}

/* Implementation */
PARSON_API JSON_Status json_object_set_null(JSON_Object *object,
                                            const char *name) {
  JSON_Value *value = json_value_init_null();
  JSON_Status rc = json_object_set_value(object, name, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
  }
  return rc;
}

/* Implementation */
PARSON_API JSON_Status json_object_dotset_value(JSON_Object *object,
                                                const char *name,
                                                JSON_Value *value) {
  enum json_result_t rc;
  enum json_result_t cleanup_rc;
  const char *dot_pos = NULL;
  JSON_Value *temp_value = NULL;
  JSON_Value *new_value = NULL;
  JSON_Object *temp_object = NULL;
  JSON_Object *new_object = NULL;
  JSON_Status status = JSONFailure;
  size_t name_len = 0;
  char *name_copy = NULL;

  if (object == NULL || name == NULL || value == NULL) {
    return JSONFailure;
  }
  dot_pos = strchr(name, '.');
  if (dot_pos == NULL) {
    return json_object_set_value(object, name, value);
  }
  name_len = (size_t)(dot_pos - name);
  rc = json_object_getn_value(object, name, name_len, &temp_value);
  if (rc == JSONSuccess) {
    if (json_value_get_type(temp_value) != JSONObject) {
      return JSONFailure;
    }
    temp_object = json_value_get_object(temp_value);
    return json_object_dotset_value(temp_object, dot_pos + 1, value);
  }
  new_value = json_value_init_object();
  if (new_value == NULL) {
    return JSONFailure;
  }
  new_object = json_value_get_object(new_value);
  status = json_object_dotset_value(new_object, dot_pos + 1, value);
  if (status != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)status);
    json_value_free(new_value);
    return status;
  }
  rc = parson_strndup(name, name_len, &name_copy);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    cleanup_rc =
        json_object_dotremove_internal(new_object, dot_pos + 1, PARSON_FALSE);
    (void)cleanup_rc;
    json_value_free(new_value);
    return JSONFailure;
  }
  rc = json_object_add(object, name_copy, new_value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    parson_free(name_copy);
    cleanup_rc =
        json_object_dotremove_internal(new_object, dot_pos + 1, PARSON_FALSE);
    (void)cleanup_rc;
    json_value_free(new_value);
    return JSONFailure;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_object_dotset_string(JSON_Object *object,
                                                 const char *name,
                                                 const char *string) {
  JSON_Status rc = JSONFailure;
  JSON_Value *value = json_value_init_string(string);
  if (value == NULL) {
    return JSONFailure;
  }
  rc = json_object_dotset_value(object, name, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
    return rc;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_object_dotset_string_with_len(JSON_Object *object,
                                                          const char *name,
                                                          const char *string,
                                                          size_t len) {
  JSON_Status rc = JSONFailure;
  JSON_Value *value = json_value_init_string_with_len(string, len);
  if (value == NULL) {
    return JSONFailure;
  }
  rc = json_object_dotset_value(object, name, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
    return rc;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_object_dotset_number(JSON_Object *object,
                                                 const char *name,
                                                 double number) {
  JSON_Status rc = JSONFailure;
  JSON_Value *value = json_value_init_number(number);
  if (value == NULL) {
    return JSONFailure;
  }
  rc = json_object_dotset_value(object, name, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
    return rc;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_object_dotset_boolean(JSON_Object *object,
                                                  const char *name,
                                                  int boolean) {
  JSON_Status rc = JSONFailure;
  JSON_Value *value = json_value_init_boolean(boolean);
  if (value == NULL) {
    return JSONFailure;
  }
  rc = json_object_dotset_value(object, name, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
    return rc;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_object_dotset_null(JSON_Object *object,
                                               const char *name) {
  JSON_Status rc = JSONFailure;
  JSON_Value *value = json_value_init_null();
  if (value == NULL) {
    return JSONFailure;
  }
  rc = json_object_dotset_value(object, name, value);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
    json_value_free(value);
    return rc;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_object_remove(JSON_Object *object,
                                          const char *name) {
  return json_object_remove_internal(object, name, PARSON_TRUE);
}

/* Implementation */
PARSON_API JSON_Status json_object_dotremove(JSON_Object *object,
                                             const char *name) {
  return json_object_dotremove_internal(object, name, PARSON_TRUE);
}

/* Implementation */
PARSON_API JSON_Status json_object_clear(JSON_Object *object) {
  size_t i = 0;
  if (object == NULL) {
    return JSONFailure;
  }
  for (i = 0; i < json_object_get_count(object); i++) {
    parson_free(object->names[i]);
    object->names[i] = NULL;

    json_value_free(object->values[i]);
    object->values[i] = NULL;
  }
  object->count = 0;
  for (i = 0; i < object->cell_capacity; i++) {
    object->cells[i] = OBJECT_INVALID_IX;
  }
  return JSONSuccess;
}

/* Implementation */
PARSON_API JSON_Status json_validate(const JSON_Value *schema,
                                     const JSON_Value *value) {
  JSON_Status rc = JSONFailure;
  JSON_Value *temp_schema_value = NULL;
  JSON_Value *temp_value = NULL;
  JSON_Array *schema_array = NULL;
  JSON_Array *value_array = NULL;
  JSON_Object *schema_object = NULL;
  JSON_Object *value_object = NULL;
  JSON_Value_Type schema_type = JSONError;
  JSON_Value_Type value_type = JSONError;
  const char *key = NULL;
  size_t i = 0;
  size_t count = 0;
  if (schema == NULL || value == NULL) {
    return JSONFailure;
  }
  schema_type = json_value_get_type(schema);
  value_type = json_value_get_type(value);
  if (schema_type != value_type &&
      schema_type != JSONNull) { /* null represents all values */
    return JSONFailure;
  }
  switch (schema_type) {
  case JSONArray:
    schema_array = json_value_get_array(schema);
    value_array = json_value_get_array(value);
    count = json_array_get_count(schema_array);
    if (count == 0) {
      return JSONSuccess; /* Empty array allows all types */
    }
    /* Get first value from array, rest is ignored */
    temp_schema_value = json_array_get_value(schema_array, 0);
    for (i = 0; i < json_array_get_count(value_array); i++) {
      temp_value = json_array_get_value(value_array, i);
      rc = json_validate(temp_schema_value, temp_value);
      if (rc != JSONSuccess) {
        LOG_DEBUG("Error: %d\n", (int)rc);
        return rc;
      }
    }
    return JSONSuccess;
  case JSONObject:
    schema_object = json_value_get_object(schema);
    value_object = json_value_get_object(value);
    count = json_object_get_count(schema_object);
    if (count == 0) {
      return JSONSuccess; /* Empty object allows all objects */
    } else if (json_object_get_count(value_object) < count) {
      return JSONFailure; /* Tested object mustn't have less name-value pairs
                             than schema */
    }
    for (i = 0; i < count; i++) {
      key = json_object_get_name(schema_object, i);
      temp_schema_value = json_object_get_value(schema_object, key);
      temp_value = json_object_get_value(value_object, key);
      if (temp_value == NULL) {
        return JSONFailure;
      }
      rc = json_validate(temp_schema_value, temp_value);
      if (rc != JSONSuccess) {
        LOG_DEBUG("Error: %d\n", (int)rc);
        return rc;
      }
    }
    return JSONSuccess;
  case JSONString:
  case JSONNumber:
  case JSONBoolean:
  case JSONNull:
    return JSONSuccess; /* equality already tested before switch */
  case JSONError:
  default:
    return JSONFailure;
  }
}

/* Implementation */
PARSON_API int json_value_equals(const JSON_Value *a, const JSON_Value *b) {
  enum json_result_t rc_a;
  enum json_result_t rc_b;
  JSON_Object *a_object = NULL;
  JSON_Object *b_object = NULL;
  JSON_Array *a_array = NULL;
  JSON_Array *b_array = NULL;
  const JSON_String *a_string = NULL;
  const JSON_String *b_string = NULL;
  const char *key = NULL;
  size_t a_count = 0;
  size_t b_count = 0;
  size_t i = 0;
  JSON_Value_Type a_type;
  JSON_Value_Type b_type;
  a_type = json_value_get_type(a);
  b_type = json_value_get_type(b);
  if (a_type != b_type) {
    return PARSON_FALSE;
  }
  switch (a_type) {
  case JSONArray:
    a_array = json_value_get_array(a);
    b_array = json_value_get_array(b);
    a_count = json_array_get_count(a_array);
    b_count = json_array_get_count(b_array);
    if (a_count != b_count) {
      return PARSON_FALSE;
    }
    for (i = 0; i < a_count; i++) {
      if (!json_value_equals(json_array_get_value(a_array, i),
                             json_array_get_value(b_array, i))) {
        return PARSON_FALSE;
      }
    }
    return PARSON_TRUE;
  case JSONObject:
    a_object = json_value_get_object(a);
    b_object = json_value_get_object(b);
    a_count = json_object_get_count(a_object);
    b_count = json_object_get_count(b_object);
    if (a_count != b_count) {
      return PARSON_FALSE;
    }
    for (i = 0; i < a_count; i++) {
      key = json_object_get_name(a_object, i);
      if (!json_value_equals(json_object_get_value(a_object, key),
                             json_object_get_value(b_object, key))) {
        return PARSON_FALSE;
      }
    }
    return PARSON_TRUE;
  case JSONString:
    rc_a = json_value_get_string_desc(a, &a_string);
    rc_b = json_value_get_string_desc(b, &b_string);
    if (rc_a != JSONSuccess || rc_b != JSONSuccess) {
      return PARSON_FALSE;
    }
    return a_string->length == b_string->length &&
           memcmp(a_string->chars, b_string->chars, a_string->length) == 0;
  case JSONBoolean:
    return json_value_get_boolean(a) == json_value_get_boolean(b);
  case JSONNumber:
    return fabs(json_value_get_number(a) - json_value_get_number(b)) <
           0.000001; /* EPSILON */
  case JSONError:
  case JSONNull:
  default:
    return PARSON_TRUE;
  }
}

/* Implementation */
PARSON_API JSON_Value_Type json_type(const JSON_Value *value) {
  return json_value_get_type(value);
}

/* Implementation */
PARSON_API JSON_Object *json_object(const JSON_Value *value) {
  return json_value_get_object(value);
}

/* Implementation */
PARSON_API JSON_Array *json_array(const JSON_Value *value) {
  return json_value_get_array(value);
}

/* Implementation */
PARSON_API const char *json_string(const JSON_Value *value) {
  return json_value_get_string(value);
}

/* Implementation */
PARSON_API size_t json_string_len(const JSON_Value *value) {
  return json_value_get_string_len(value);
}

/* Implementation */
PARSON_API double json_number(const JSON_Value *value) {
  return json_value_get_number(value);
}

/* Implementation */
PARSON_API int json_boolean(const JSON_Value *value) {
  return json_value_get_boolean(value);
}

/* Implementation */
PARSON_API void json_set_allocation_functions(JSON_Malloc_Function malloc_fun,
                                              JSON_Free_Function free_fun) {
  parson_malloc = malloc_fun;
  parson_free = free_fun;
}

/* Implementation */
PARSON_API void json_set_escape_slashes(int escape_slashes) {
  parson_escape_slashes = escape_slashes;
}

/* Implementation */
PARSON_API void json_set_float_serialization_format(const char *format) {
  enum json_result_t rc;
  if (parson_float_format) {
    parson_free(parson_float_format);
    parson_float_format = NULL;
  }
  if (!format) {
    parson_float_format = NULL;
    return;
  }
  rc = parson_strdup(format, &parson_float_format);
  if (rc != JSONSuccess) {
    LOG_DEBUG("Error: %d\n", (int)rc);
  }
}

/* Implementation of json_set_number_serialization_function */
PARSON_API void
json_set_number_serialization_function(JSON_Number_Serialization_Function fun) {
  parson_number_serialization_function = fun;
}

#endif /* PARSON_IMPLEMENTED */
#endif /* PARSON_IMPLEMENTATION */
