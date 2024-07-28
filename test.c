#include <stdlib.h>
#include <stdio.h>
#include "json_parser.h"

#define BUFSIZE		(64 * 1024 * 1024)

void print_json_value(const json_value_t *val, int depth);

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
			if ((unsigned char)*str < 0x20)
				printf("\\u00%02x", *str);
			else
				printf("%c", *str);
			break;
		}
		str++;
	}
	printf("\"");
}

void print_json_object(const json_object_t *obj, int depth)
{
	const char *name;
	const json_value_t *val;
	int n = 0;
	int i;

	if (json_object_size(obj) == 0)
	{
		printf("{}");
		return;
	}

	printf("{\n");
	json_object_for_each(name, val, obj)
	{
		if (n != 0)
			printf(",\n");
		n++;
		for (i = 0; i < depth + 1; i++)
			printf("    ");
		print_json_string(name);
		printf(": ");
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

	if (json_array_size(arr) == 0)
	{
		printf("[]");
		return;
	}

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

