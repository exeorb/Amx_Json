#if defined _json_included
	#endinput
#endif
#define _json_included


#if AMXX_VERSION_NUM >= 175
 #pragma reqlib json
 #if !defined AMXMODX_NOAUTOLOAD
  #pragma loadlib json
 #endif
#else
 #pragma library json
#endif

/**
 * Originaly docs tooked from http://jansson.readthedocs.org/en/latest/apiref.html
 */

enum JsonHandle
{
	INVALID_JSON = INVALID_HANDLE,
};

enum JsonType
{
	JSON_OBJECT = 0,
	JSON_ARRAY,
	JSON_STRING,
	JSON_INTEGER,
	JSON_REAL,
	JSON_TRUE,
	JSON_FALSE,
	JSON_NULL
};

enum JsonIter
{
	INVALID_ITER = INVALID_HANDLE,
};

/** 
 * Destroy handle. Return 0 on error or 1 on success
 */
native destroy_handle(any:handle);


/** 
 * destroy_handle result has to be used. I don't have good idea how to do this
 */
stock destroy_json(&JsonHandle:handle)
{
	if (handle == INVALID_JSON)
	{
		return
	}
	destroy_handle(handle)
	handle = INVALID_JSON
}

stock destroy_iter(&JsonIter:iter)
{
	if (iter == INVALID_ITER)
	{
		return
	}
	destroy_handle(iter)
	iter = INVALID_ITER
}

/**
 * Return the type of the JSON value (a json_type cast to int). 
 * json MUST NOT be INVALID_JSON.
 */
native JsonType:json_typeof(JsonHandle:handle);

/**
 * Functions below are actually implemented as a macro for speed
 */

#define json_is_object(%1)   (%1 != INVALID_JSON && json_typeof(%1) == JSON_OBJECT)
#define json_is_array(%1)    (%1 != INVALID_JSON && json_typeof(%1) == JSON_ARRAY)
#define json_is_string(%1)   (%1 != INVALID_JSON && json_typeof(%1) == JSON_STRING)
#define json_is_integer(%1)  (%1 != INVALID_JSON && json_typeof(%1) == JSON_INTEGER)
#define json_is_real(%1)     (%1 != INVALID_JSON && json_typeof(%1) == JSON_REAL)
#define json_is_number(%1)   (json_is_integer(%1) || json_is_real(%1))
#define json_is_true(%1)     (%1 != INVALID_JSON && json_typeof(%1) == JSON_TRUE)
#define json_is_false(%1)    (%1 != INVALID_JSON && json_typeof(%1) == JSON_FALSE)
#define json_is_boolean(%1)  (json_is_true(%1) || json_is_false(%1))
#define json_is_null(%1)     (%1 != INVALID_JSON && json_typeof(%1) == JSON_NULL)


/* true, false or INVALID_JSON */

/** 
 * Return value: New reference.
 * Returns the JSON true value.
 */
native JsonHandle:json_true();

/** 
 * Return value: New reference.
 * Returns the JSON false value.
 */
native JsonHandle:json_false();

/** 
 * Return value: New reference.
 * Returns JSON false if val is zero, and JSON true otherwise. 
 * val ? json_true() : json_false().
 */
#define json_boolean(%1)	%1 ? json_true() : json_false()

/** 
 * Return value: New reference.
 * Returns the JSON null value.
 */
native JsonHandle:json_null();


/* number */


/** 
 * Return value: New reference.
 * Returns a new JSON integer, or INVALID_JSON on error.
 */
native JsonHandle:json_integer(value);


/**
 * Returns the associated value of integer, or 0 if json is not a JSON integer.
 */
native json_integer_value(JsonHandle:handle);


/**
 * Sets the associated value of integer to value. Returns 1 on success and 0 if integer is not a JSON integer.
 */
native json_integer_set(JsonHandle:handle, value);


/** 
 * Return value: New reference.
 * Returns a new JSON real, or INVALID_JSON on error.
 */
native JsonHandle:json_real(Float:value);


/** 
 * Returns the associated value of real, or 0.0 if real is not a JSON real.
 */
native Float:json_real_value(JsonHandle:handle);


/** 
 * Sets the associated value of real to value. Returns 1 on success and 0 if real is not a JSON real.
 */
native json_real_set(JsonHandle:handle, Float:value);



/* array */


/**
 * Return value: New reference.
 * Returns a new JSON array, or INVALID_JSON on error. Initially, the array is empty.
 */
native JsonHandle:json_array();


/**
 * Returns the number of elements in array, or 0 if array is INVALID_JSON or not a JSON array.
 */
native json_array_size(JsonHandle:handle);


/**
 * Returns the element in array at position index. 
 * The valid range for index is from 0 to the return value of json_array_size() minus 1. 
 * If array is not a JSON array, if array is INVALID_JSON, or if index is out of range, INVALID_JSON is returned.
 * ! Returns new handle with borrowed reference on json value and it has to be closed by `destroy_json`
 */
native JsonHandle:json_array_get(JsonHandle:handle, index);


/**
 * Replaces the element in array at position index with value. 
 * The valid range for index is from 0 to the return value of json_array_size() minus 1. 
 * Returns 1 on success and 0 on error.
 */
native json_array_set(JsonHandle:handle, index, JsonHandle:value);

/**
 * Like json_array_set() but steals the reference to value. 
 * This is useful when value is newly created and not used after the call.
 */
native json_array_set_new(JsonHandle:handle, index, JsonHandle:value);


/**
 * Appends value to the end of array, growing the size of array by 1. Returns 1 on success and 0 on error.
 */
native json_array_append(JsonHandle:handle, JsonHandle:value);


/**
 * Like json_array_append() but steals the reference to value. 
 * This is useful when value is newly created and not used after the call.
 */
native json_array_append_new(JsonHandle:handle, JsonHandle:value);


/**
 * Inserts value to array at position index, 
 * shifting the elements at index and after it one position towards the end of the array. 
 * Returns 1 on success and 0 on error.
 */
native json_array_insert(JsonHandle:handle, index, JsonHandle:value);


/**
 * Like json_array_insert() but steals the reference to value. 
 * This is useful when value is newly created and not used after the call.
 */
native json_array_insert_new(JsonHandle:handle, index, JsonHandle:value);


/**
 * Removes the element in array at position index, shifting the elements after index 
 * one position towards the start of the array. Returns 1 on success and 0 on error. 
 * The reference count of the removed value is decremented.
 */
native json_array_remove(JsonHandle:handle, index);


/**
 * Removes all elements from array. Returns 1 on suсcess and 0 on error. 
 * The reference count of all removed values are decremented.
 */
native json_array_clear(JsonHandle:handle);


/**
 * Iterate over every element of array, running the block of code that follows each time 
 * with the proper values set to variables index and value, of types cell and JsonHandle respectively.
 * ! Attension: each valid value has to be destroy in cycle block (see `json_array_get` )
 *
 * json_array_foreach(array, index, value) 
 */
#define json_array_foreach(%1,%2,%3) \
	for(%2 = 0; \
		%2 < json_array_size(%1) && (%3 = json_array_get(%1, %2)); \
		%2++)


/**
 *
 * Appends all elements in other_array to the end of array. Returns 1 on success and 0 on error.
 */
native json_array_extend(JsonHandle:handle, JsonHandle:other);


/* string */


/**
 * Return value: New reference.
 * Returns a new JSON string, or INVALID_JSON on error. value must be a valid UTF-8 encoded Unicode string.
 */
native JsonHandle:json_string(const value[]);


/**
 * Return value: New reference.
 * Like json_string(), but with explicit length, so value may contain NULL characters or not be NULL terminated.
 */
native JsonHandle:json_stringn(const value[], len);


/**
 * to do description
 */
native json_string_value(JsonHandle:handle, buffer[], len);


/**
 * Returns the length of string in its UTF-8 presentation, 
 * or zero if string is not a JSON string.
 */
native json_string_length(JsonHandle:handle);

/**
 * Sets the associated value of string to value. value must be a valid UTF-8 encoded Unicode string. 
 * Returns 1 on success and 0 on error.
 */
native json_string_set(JsonHandle:handle, const value[]);


/**
 * Not implt yet
 * Like json_string_set(), but with explicit length, so value may contain INVALID_JSON characters or not be INVALID_JSON terminated.
 * native json_string_setn(JsonHandle:handle, const value[], len);
 */

/* object */


/**
 * Return value: New reference.
 * Returns a new JSON object, or INVALID_JSON on error. Initially, the object is empty.
 */
native JsonHandle:json_object();


/**
 * Returns the number of elements in object, or 0 if object is not a JSON object.
 */
native json_object_size(JsonHandle:handle);


/**
 * Delete key from object if it exists. Returns 1 on success, or 0 if key was not found. 
 * The reference count of the removed value is decremented. 
 */
native json_object_del(JsonHandle:handle, const key[]);


/**
 * Get a value corresponding to key from object. Returns INVALID_JSON if key is not found and on error.
 * Return new handle with borrowed reference on json value and it has to be closed by `destroy_json`
 */
native JsonHandle:json_object_get(JsonHandle:handle, const key[]);


/**
 * Set the value of key to value in object. key must be a valid INVALID_JSON terminated UTF-8 encoded Unicode string. 
 * If there already is a value for key, it is replaced by the new value. Returns 1 on success and 0 on error.
 */
native json_object_set(JsonHandle:handle, const key[], JsonHandle:value);


/**
 * Like json_object_set() but steals the reference to value. 
 * This is useful when value is newly created and not used after the call.
 */
native json_object_set_new(JsonHandle:handle, const key[], JsonHandle:value);

/**
 * Remove all elements from object. Returns 1 on success and 0 if object is not a JSON object. 
 * The reference count of all removed values are decremented.
 */
native json_object_clear(JsonHandle:handle);


/**
 * Returns an opaque iterator which can be used to iterate over all key-value pairs in object, 
 * or INVALID_ITER if object is empty.
 */
native JsonIter:json_object_iter(JsonHandle:handle);


/**
 * Like json_object_iter(), but returns an iterator to the key-value pair in object whose key is equal to key, 
 * or INVALID_ITER if key is not found in object. 
 */
native JsonIter:json_object_iter_at(JsonHandle:handle, const key[])


/**
 * Returns an iterator pointing to the next key-value pair in object after iter, 
 * or INVALID_ITER if the whole object has been iterated through.
 */ 
native JsonIter:json_object_iter_next(JsonHandle:handle, JsonIter:iter);


/**
 * Extract the associated key from iter.
 */ 
native json_object_iter_key(JsonIter:iter, buf[], len);


/**
 * ! Return value: new JsonHandle with borrowed reference on json object
 * Extract the associated value from iter.
 */ 
native JsonHandle:json_object_iter_value(JsonIter:iter);


/**
 * Set the value of the key-value pair in object, that is pointed to by iter, to value.
 */ 
native json_object_iter_set(JsonHandle:handle, JsonIter:iter, JsonHandle:value);


/**
 * Like json_object_iter_set(), but steals the reference to value. This is useful when value is newly created and not used after the call.
 */ 
native json_object_iter_set_new(JsonHandle:handle, JsonIter:iter, JsonHandle:value);


/**
 * Like json_object_iter_at(), but much faster. Only works for values returned by json_object_iter_key(). 
 * Using other keys will lead to segfaults. This function is used internally to implement json_object_foreach().
 */ 
native JsonIter:json_object_key_to_iter(const key[]);



/**
 * Iterate over every key-value pair of object, running the block of code that follows 
 * each time with the proper values set to variables key and value, of types  
 * const String[] and JsonHandle respectively.
 *  
 * define json_object_foreach(object, key, value)
 * not impl yet
 */


/* encoding */

#define JSON_INDENT(%1)     (%1 & 0x1F)
#define JSON_COMPACT        0x20
#define JSON_ENSURE_ASCII   0x40
#define JSON_SORT_KEYS      0x80
#define JSON_PRESERVE_ORDER 0x100
#define JSON_ENCODE_ANY     0x200


/**
 * Returns 1 on success and 0 on error
 * JSON representation of root as a string. 
 * flags is described above
 */
native json_dumps(JsonHandle:handle, flags, buffer[], len);


/**
 * Write the JSON representation of root to the file path. If path already exists, it is overwritten. 
 * flags is described above. Returns 1 on success and 0 on error.
 */
native json_dump_file(JsonHandle:handle, const path[], flags);



/* decoding */

#define JSON_REJECT_DUPLICATES 0x1
#define JSON_DISABLE_EOF_CHECK 0x2


/**
 * Return value: New reference.
 * Decodes the JSON string input and returns the array or object it contains, 
 * or INVALID_JSON on error, in which case error is filled with information about the error. 
 * flags is described above.
 */
native JsonHandle:json_loads(const input[], flags, error[], len);


/**
 * Return value: New reference.
 * Decodes the JSON text in file path and returns the array or object it contains, 
 * or INVALID_JSON on error, in which case error is filled with information about the error. 
 * flags is described above.
 */
native JsonHandle:json_load_file(const path[], flags, error[], len);
