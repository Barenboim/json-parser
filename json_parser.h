#ifndef _JSON_PARSER_H_
#define _JSON_PARSER_H_

#define JSON_VALUE_STRING	1
#define JSON_VALUE_NUMBER	2
#define JSON_VALUE_OBJECT	3
#define JSON_VALUE_ARRAY	4
#define JSON_VALUE_TRUE		5
#define JSON_VALUE_FALSE	6
#define JSON_VALUE_NULL		7

typedef struct __json_value json_value_t;
typedef struct __json_object json_object_t;
typedef struct __json_array json_array_t;

#ifdef __cplusplus
extern "C"
{
#endif

json_value_t *json_value_create(const char *doc);
void json_value_destroy(json_value_t *val);

int json_value_type(const json_value_t *val);
const char *json_value_string(const json_value_t *val);
const double *json_value_number(const json_value_t *val);
const json_object_t *json_value_object(const json_value_t *val);
const json_array_t *json_value_array(const json_value_t *val);

const json_value_t *json_object_find(const char *name,
									 const json_object_t *obj);
int json_object_count(const json_object_t *obj);
int json_object_read(const char *name[], const json_value_t *val[], int n,
					 const json_object_t *obj);

int json_array_count(const json_array_t *arr);
int json_array_read(const json_value_t *val[], int n,
					const json_array_t *arr);

#ifdef __cplusplus
}
#endif

#endif

