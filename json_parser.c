#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "list.h"
#include "rbtree.h"

typedef struct __json_object json_object_t;
typedef struct __json_array json_array_t;
typedef struct __json_value json_value_t;
typedef struct __json_member json_member_t;
typedef struct __json_element json_element_t;

#define JSON_VALUE_STRING	1
#define JSON_VALUE_NUMBER	2
#define JSON_VALUE_OBJECT	3
#define JSON_VALUE_ARRAY	4
#define JSON_VALUE_TRUE		5
#define JSON_VALUE_FALSE	6
#define JSON_VALUE_NULL		7

struct __json_object
{
	struct list_head head;
	struct rb_root root;
	int count;
};

struct __json_array
{
	struct list_head head;
	int count;
};

struct __json_value
{
	int type;
	union
	{
		char *string;
		double number;
		json_object_t object;
		json_array_t array;
	} value;
};

struct __json_member
{
	struct list_head list;
	struct rb_node rb;
	json_value_t value;
	char name[1];
};

struct __json_element
{
	struct list_head list;
	json_value_t value;
};

static void __insert_json_member(json_member_t *memb, json_object_t *obj)
{
	struct rb_node **p = &obj->root.rb_node;
	struct rb_node *parent = NULL;
	json_member_t *entry;
	int n;

	while (*p)
	{
		parent = *p;
		entry = rb_entry(*p, json_member_t, rb);
		n = strcmp(memb->name, entry->name);
		if (n < 0)
			p = &(*p)->rb_left;
		else
			p = &(*p)->rb_right;
	}

	rb_link_node(&memb->rb, parent, p);
	rb_insert_color(&memb->rb, &obj->root);
	list_add_tail(&memb->list, &obj->head);
}

static void __copy_json_string(char *dest, const char *src, int len)
{
	int i;

	for (i = 0; i < len; i++)
	{
		if (*src == '\\')
		{
			src++;
			switch (*src)
			{
			case '\"':
				*dest = '\"';
				break;
			case '\\':
				*dest = '\\';
				break;
			case '/':
				*dest = '/';
				break;
			case 'b':
				*dest = '\b';
				break;
			case 'f':
				*dest = '\f';
				break;
			case 'n':
				*dest = '\n';
				break;
			case 'r':
				*dest = '\r';
				break;
			case 't':
				*dest = '\t';
				break;
			default:
				assert(0);
				break;
			}
		}
		else
			*dest = *src;

		src++;
		dest++;
	}

	*dest = '\0';
}

static int __parse_json_string(const char *cursor, const char **end)
{
	int len = 0;
	int ret;

	while (1)
	{
		if (*cursor == '\"')
			break;

		if (!*cursor)
			return -2;

		cursor++;
		if (cursor[-1] == '\\')
		{
			switch (*cursor)
			{
			case '\"':
			case '\\':
			case '/':
			case 'b':
			case 'f':
			case 'n':
			case 'r':
			case 't':
				cursor++;
				break;
			case 'u':	/* TODO: support unicode */
			default:
				return -2;
			}
		}

		len++;
	}

	*end = cursor + 1;
	return len;
}

static int __parse_json_number(const char *cursor, const char **end,
							   double *number)
{
	*number = strtod(cursor, (char **)end);
	if (*end == cursor)
		return -2;

	return 0;
}

static int __parse_json_value(const char *cursor, const char **end,
							  json_value_t *val);

static void __destroy_json_value(json_value_t *val);

static int __parse_json_object(const char *cursor, const char **end,
							   json_object_t *obj);

static int __parse_json_elements(const char *cursor, const char **end,
								 json_array_t *arr)
{
	json_element_t *elem;
	int cnt = 0;
	int ret;

	while (isspace(*cursor))
		cursor++;

	if (*cursor == ']')
	{
		*end = cursor + 1;
		return 0;
	}

	while (1)
	{
		elem = (json_element_t *)malloc(sizeof (json_element_t));
		if (!elem)
			return -1;

		ret = __parse_json_value(cursor, &cursor, &elem->value);
		if (ret < 0)
		{
			free(elem);
			return -2;
		}

		list_add_tail(&elem->list, &arr->head);
		cnt++;

		if (*cursor != ',')
		{
			while (isspace(*cursor))
				cursor++;

			if (*cursor != ']')
				return -2;

			break;
		}

		cursor++;
		while (isspace(*cursor))
			cursor++;
	}

	*end = cursor + 1;
	return cnt;
}

static void __destroy_json_elements(json_array_t *arr)
{
	struct list_head *pos, *tmp;
	json_element_t *elem;

	list_for_each_safe(pos, tmp, &arr->head)
	{
		elem = list_entry(pos, json_element_t, list);
		__destroy_json_value(&elem->value);
		free(elem);
	}
}

static int __parse_json_array(const char *cursor, const char **end,
							  json_array_t *arr)
{
	int ret;

	INIT_LIST_HEAD(&arr->head);
	ret = __parse_json_elements(cursor, end, arr);
	if (ret < 0)
	{
		__destroy_json_elements(arr);
		return ret;
	}

	arr->count = ret;
	return 0;
}

static int __parse_json_value(const char *cursor, const char **end,
							  json_value_t *val)
{
	int ret;

	switch (*cursor)
	{
	case '\"':
		cursor++;
		ret = __parse_json_string(cursor, end);
		if (ret < 0)
			return ret;

		val->value.string = (char *)malloc(ret + 1);
		if (!val->value.string)
			return -1;

		__copy_json_string(val->value.string, cursor, ret);
		val->type = JSON_VALUE_STRING;
		break;

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
		ret = __parse_json_number(cursor, end, &val->value.number);
		val->type = JSON_VALUE_NUMBER;
		break;

	case '{':
		cursor++;
		ret = __parse_json_object(cursor, end, &val->value.object);
		if (ret < 0)
			return ret;

		val->type = JSON_VALUE_OBJECT;
		break;

	case '[':
		cursor++;
		ret = __parse_json_array(cursor, end, &val->value.array);
		if (ret < 0)
			return ret;

		val->type = JSON_VALUE_ARRAY;
		break;

	defaut:
		return -2;
	}

	return 0;
}

static int __parse_json_members(const char *cursor, const char **end,
								json_object_t *obj)
{
	json_member_t *memb;
	const char *name;
	int cnt = 0;
	size_t size;
	int ret;
	int len;

	while (isspace(*cursor))
		cursor++;

	if (*cursor == '}')
	{
		*end = cursor + 1;
		return 0;
	}

	while (1)
	{
		if (*cursor != '\"')
			return -2;

		cursor++;
		name = cursor;
		ret = __parse_json_string(cursor, &cursor);
		if (ret < 0)
			return ret;

		len = ret;
		while (isspace(*cursor))
			cursor++;

		if (*cursor != ':')
			return -2;

		size = offsetof(json_member_t, name) + len + 1;
		memb = (json_member_t *)malloc(size);
		if (!memb)
			return -1;

		cursor++;
		while (isspace(*cursor))
			cursor++;

		ret = __parse_json_value(cursor, &cursor, &memb->value);
		if (ret < 0)
		{
			free(memb);
			return ret;
		}

		__copy_json_string(memb->name, name, len);
		__insert_json_member(memb, obj);
		cnt++;

		if (*cursor != ',')
		{
			while (isspace(*cursor))
				cursor++;

			if (*cursor != '}')
				return -2;

			break;
		}

		cursor++;
		while (isspace(*cursor))
			cursor++;
	}

	*end = cursor + 1;
	return cnt;
}

static void __destroy_json_members(json_object_t *obj)
{
	struct list_head *pos, *tmp;
	json_member_t *memb;

	list_for_each_safe(pos, tmp, &obj->head)
	{
		memb = list_entry(pos, json_member_t, list);
		list_del(pos);
		rb_erase(&memb->rb, &obj->root);
		__destroy_json_value(&memb->value);
		free(memb);
	}
}

static void __destroy_json_value(json_value_t *val)
{
	switch (val->type)
	{
	case JSON_VALUE_STRING:
		free(val->value.string);
		break;
	case JSON_VALUE_OBJECT:
		__destroy_json_members(&val->value.object);
		break;
	case JSON_VALUE_ARRAY:
		__destroy_json_elements(&val->value.array);
		break;
	}
}

static int __parse_json_object(const char *cursor, const char **end,
							   json_object_t *obj)
{
	int ret;

	INIT_LIST_HEAD(&obj->head);
	obj->root.rb_node = NULL;
	ret = __parse_json_members(cursor, end, obj);
	if (ret < 0)
	{
		__destroy_json_members(obj);
		return ret;
	}

	obj->count = ret;
	return 0;
}

json_object_t *parse_json_document(const char *doc)
{
	json_object_t *obj;
	int ret;

	while (isspace(*doc))
		doc++;

	if (*doc != '{')
		return NULL;

	obj = (json_object_t *)malloc(sizeof (json_object_t));
	if (!obj)
		return NULL;

	doc++;
	ret = __parse_json_object(doc, &doc, obj);
	if (ret < 0)
	{
		free(obj);
		return NULL;
	}

	return obj;
}

void destroy_json_object(json_object_t *obj)
{
	__destroy_json_members(obj);
	free(obj);
}

#include <stdio.h>

int main()
{
	char *buf = NULL;
	size_t size = 0;
	const char *end;
	json_object_t *obj;
	const char *aaa[] = {
		"{ \"name\\\\\\\\\" : \"value\\t\\t\\t\", \n"
		" \"\\t\\/name1\" : \"value2\" }",
		"{ \"name\"	:	[	[	[	{	\"\\t\\t\\t\" : { } 	}, \"value\"	]]] }",
		"{ \"name\"	:			[		{ }, \"value\"	] }",
		"{ \"name\" :		[-1,   0.001, 0, 0.5e10 ] }"
	};
	int n = sizeof aaa / sizeof *aaa;
	int i;
	for (i = 0; i < n; i++)
	{
		printf("%s\n", aaa[i]);
		obj = parse_json_document(aaa[i]);
		if (obj)
			destroy_json_object(obj);
	}

	return 0;
}
