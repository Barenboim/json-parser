# Json Parser in Standard C (C99)
This json parser was created for the project of [C++ Workflow](https://github.com/sogou/workflow).  
# Build tests
~~~bash
$ make
~~~
# Run tests
### Parse and print json document:
~~~bash
$ ./parse_json < xxx.json
~~~
### Test parsing speed:
~~~bash
$ time ./test_speed <repeat times> < xxx.json
~~~
# Main Interfaces

### JSON value related
~~~c
/* Parse JSON document and create a JSON value. Returns NULL on parsing
   failures (Invalid JSON, nesting too deep, memory allocation failure).
   @doc JSON document */
json_value_t *json_value_parse(const char *doc);

/* Destroy the JSON value
   @val JSON value. Typically created by the parsing function. */
void json_value_destroy(json_value_t *val);

/* Get JSON value's type
     Return values:
     JSON_VALUE_STRING: string
     JSON_VALUE_NUMBER: number
     JSON_VALUE_OBJECT: JSON object
     JSON_VALUE_ARRAY: JSON array
     JSON_VALUE_TRUE: true
     JSON_VALUE_FALSE: false
     JSON_VALUE_NULL: null
   @val: JSON value */
int json_value_type(const json_value_t *val);

/* Obtain the JSON string. The function returns the string or
   returns NULL if the type of @val is not JSON_VALUE_STRING.
   @val: JSON value */
const char *json_value_string(const json_value_t *val);

/* Obtain the JSON number. The function returns the number or
   returns NAN if the type of @val is not JSON_VALUE_NUMBER.
   @val: JSON value */
double json_value_number(const json_value_t *val);

/* Obtain JSON object. The function returns the JSON object or
   returns NULL if the type of @val is not JSON_VALUE_OBJECT.
   @val: JSON value
   Note: The returned pointer to JSON object is not const.
         You may extend the object using buiding functions. */
json_object_t *json_value_object(const json_value_t *val);

/* Obtain JSON arary. The function returns the JSON array or
   returns NULL if the type of @val is not JSON_VALUE_ARRAY.
   @val: JSON value
   Note: The returned pointer to the JSON array is not const and
         can been extended by using building functions. */
json_array_t *json_value_array(const json_value_t *val);

~~~
### JSON object related
~~~c
/* Get the size of the JSON object.
   @obj: JSON object */
int json_object_size(const json_object_t *obj);

/* Find the JSON value under the key @name. Returns NULL if @name
   can not be found. The time complexity of this function is
   O(log(n)), where n is the size of the JSON object.
   @name: The key to find
   @obj: JSON object
   Note: The returned pointer to JSON value is const. */
const json_value_t *json_object_find(const char *name, const json_object_t *obj);

/* Traversing the JSON object
   @name: Temporary (const char *) pointer for each key
   @val: Temporary (const json_value_t *) pointer for each JSON value
   @obj: JSON object
   NOTE: This is not a function. It's a macro of a loop. */
json_object_for_each(name, val, obj)

~~~

### JSON array related
~~~c
/* Get the size of the JSON array.
   @arr：JSON array */
int json_array_size(const json_array_t *arr);

/* Traversing the JSON array
   @val: Temporary (const json_value_t *) pointer for each JSON value
   @arr: JSON array
   NOTE: This is not a function. It's a macro of a loop. */
json_array_for_each(val, arr)
~~~

### Building JSON
All the following functions return NULL on failure of allocating memory.
~~~c
/* Create a JSON value.
   @type: JSON value's type.
   Note: Variable argument. Example:
     v_str = json_value_create(JSON_VALUE_STRING, "hello");
     v_num = json_value_create(JSON_VALUE_NUMBER, 3.14);
     v_int = json_value_create(JSON_VALUE_NUMBER, (double)100);
     v_obj = json_value_create(JSON_VALUE_OBJECT);
     v_arr = json_value_create(JSON_VALUE_ARRAY);
     v_true = json_value_create(JSON_VALUE_TRUE);
   The returned value is not const and has to be destroyed. */
json_value_t *json_value_create(int type, ...);

/* Extend the JSON object. Returns the value that's added.
   @obj: JSON object
   @name: New member's name
   @type: New member's value type or zero which means another JSON value
   Note: Variable argument. Example:
     v_num = json_object_append(obj, "pi", JSON_VALUE_NUMBER, 3.14);
     v_obj = json_object_append(obj, "user", JSON_VALUE_OBJECT);
     v_from_doc = json_object_append(obj, "doc", 0, json_value_parse("{\"data\" : [1, 2, 3]}")); */
const json_value_t *json_object_append(json_object_t *obj, const char *name,
                                       int type, ...);

/* Remove a JSON value from a JSON object and return the value.
   @val: JSON value to be removed.
   @obj: JSON object
   Note: The returned value is not const and need to be destroyed,
         or appended to another JSON object or JSON array. */
json_value_t *json_object_remove(const json_value_t *val,
                                 json_object_t *obj);

/* Extend the JSON array. Return the value that's added.
   @arr: JSON array
   @type: New member's value type or zero which means another JSON value
   Note: Variable argument. Example:
     v_str = json_array_append(arr, JSON_VALUE_STRING, "hello");
     equal to:
     v_str = json_array_append(arr, 0, json_value_create(JSON_VALUE_STRING, "hello")); */
const json_value_t *json_array_append(json_array_t *arr, int type, ...);

/* Remove a JSON value from a JSON array and return the value.
   @val: JSON value to be removed
   @arr: JSON array
   Note: The returned value is not const and need to be destroyed,
         or appended to another JSON object or JSON array. */
json_value_t *json_array_remove(const json_value_t *val,
                                json_object_t *arr);

~~~
