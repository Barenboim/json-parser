#include <stdlib.h>
#include <stdio.h>
#include "json_parser.h"

#define BUFSIZE		(64 * 1024 * 1024)

void print_json_value(const json_value_t *val, int depth);

void print_json_object(const json_object_t *obj, int depth)
{
	const char *name;
	const json_value_t *val;
	int n = 0;
	int i;

	printf("{\n");
	json_object_for_each(name, val, obj)
	{
		if (n != 0)
			printf(",\n");
		n++;
		for (i = 0; i < depth + 1; i++)
			printf("    ");
		printf("\"%s\": ", name);
		print_json_value(val, depth + 1);
	}

	printf("\n");
	for (i = 0; i < depth; i++)
		printf("    ");
	printf("}");
}

void print_json_array(const json_array_t *arr, int depth)
{
	const json_value_t *val;
	int n = 0;
	int i;

	printf("[\n");
	json_array_for_each(val, arr)
	{
		if (n != 0)
			printf(",\n");
		n++;
		for (i = 0; i < depth + 1; i++)
			printf("    ");
		print_json_value(val, depth + 1);
	}

	printf("\n");
	for (i = 0; i < depth; i++)
		printf("    ");
	printf("]");
}

void print_json_string(const char *str)
{
	printf("\"");
	while (*str)
	{
		switch (*str)
		{
		case '\r':
			printf("\\r");
			break;
		case '\n':
			printf("\\n");
			break;
		case '\f':
			printf("\\f");
			break;
		case '\b':
			printf("\\b");
			break;
		case '\"':
			printf("\\\"");
			break;
		case '\t':
			printf("\\t");
			break;
		case '\\':
			printf("\\\\");
			break;
		default:
			printf("%c", *str);
			break;
		}
		str++;
	}
	printf("\"");
}

void print_json_number(double number)
{
	long long integer = number;

	if (integer == number)
		printf("%lld", integer);
	else
		printf("%lf", number);
}

void print_json_value(const json_value_t *val, int depth)
{
	switch (json_value_type(val))
	{
	case JSON_VALUE_STRING:
		print_json_string(json_value_string(val));
		break;
	case JSON_VALUE_NUMBER:
		print_json_number(json_value_number(val));
		break;
	case JSON_VALUE_OBJECT:
		print_json_object(json_value_object(val), depth);
		break;
	case JSON_VALUE_ARRAY:
		print_json_array(json_value_array(val), depth);
		break;
	case JSON_VALUE_TRUE:
		printf("true");
		break;
	case JSON_VALUE_FALSE:
		printf("false");
		break;
	case JSON_VALUE_NULL:
		printf("null");
		break;
	}
}

json_value_t *json_value_copy(const json_value_t *val);

json_value_t *json_value_copy_object(const json_value_t *val)
{
	json_value_t *dest_val = json_value_create(JSON_VALUE_OBJECT);
	json_object_t *dest_obj = json_value_object(dest_val);
	json_object_t *obj = json_value_object(val);
	const char *name;

	json_object_for_each(name, val, obj)
		json_object_append(dest_obj, name, 0, json_value_copy(val));

	return dest_val;
}

json_value_t *json_value_copy_array(const json_value_t *val)
{
	json_value_t *dest_val = json_value_create(JSON_VALUE_ARRAY);
	json_array_t *dest_arr = json_value_array(dest_val);
	json_array_t *arr = json_value_array(val);

	json_array_for_each(val, arr)
		json_array_append(dest_arr, 0, json_value_copy(val));

	return dest_val;
}

json_value_t *json_value_copy(const json_value_t *val)
{
	switch (json_value_type(val))
	{
	case JSON_VALUE_STRING:
		return json_value_create(JSON_VALUE_STRING, json_value_string(val));
	case JSON_VALUE_NUMBER:
		return json_value_create(JSON_VALUE_NUMBER, json_value_number(val));
	case JSON_VALUE_OBJECT:
		return json_value_copy_object(val);
	case JSON_VALUE_ARRAY:
		return json_value_copy_array(val);
	default:
		return json_value_create(json_value_type(val));
	}
}


int main()
{
	static char buf[BUFSIZE];
	size_t n = fread(buf, 1, BUFSIZE, stdin);

	if (n > 0)
	{
		if (n == BUFSIZE)
		{
			fprintf(stderr, "File too large.\n");
			exit(1);
		}

		buf[n] = '\0';
	}
	else
	{
		perror("fread");
		exit(1);
	}

	json_value_t *val = json_value_parse(buf);
	if (val)
	{
		json_value_t *val1 = json_value_copy(val);
		json_value_destroy(val);
		print_json_value(val1, 0);
		json_value_destroy(val1);
	}
	else
		fprintf(stderr, "Invalid JSON document.\n");

	return 0;
}

