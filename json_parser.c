#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include "list.h"
#include "rbtree.h"
#include "json_parser.h"

#define JSON_DEPTH_LIMIT	1024

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

typedef struct __json_member json_member_t;
typedef struct __json_element json_element_t;

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
	const char *p = cursor;

	if (*p == '-')
		p++;

	if (*p == '0' && (tolower(p[1]) == 'x' || isdigit(p[1])))
		return -2;

	*number = strtod(cursor, (char **)end);
	if (*end == cursor)
		return -2;

	return 0;
}

static int __parse_json_value(const char *cursor, const char **end,
							  int depth, json_value_t *val);

static void __destroy_json_value(json_value_t *val);

static int __parse_json_object(const char *cursor, const char **end,
							   int depth, json_object_t *obj);

static int __parse_json_elements(const char *cursor, const char **end,
								 int depth, json_array_t *arr)
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

		ret = __parse_json_value(cursor, &cursor, depth, &elem->value);
		if (ret < 0)
		{
			free(elem);
			return -2;
		}

		list_add_tail(&elem->list, &arr->head);
		cnt++;

		while (isspace(*cursor))
			cursor++;

		if (*cursor == ',')
		{
			cursor++;
			while (isspace(*cursor))
				cursor++;
		}
		else if (*cursor == ']')
			break;
		else
			return -2;
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
							  int depth, json_array_t *arr)
{
	int ret;

	if (depth >= JSON_DEPTH_LIMIT)
		return -3;

	INIT_LIST_HEAD(&arr->head);
	ret = __parse_json_elements(cursor, end, depth, arr);
	if (ret < 0)
	{
		__destroy_json_elements(arr);
		return ret;
	}

	arr->count = ret;
	return 0;
}

static int __parse_json_value(const char *cursor, const char **end,
							  int depth, json_value_t *val)
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
		ret = __parse_json_object(cursor, end, depth + 1, &val->value.object);
		if (ret < 0)
			return ret;

		val->type = JSON_VALUE_OBJECT;
		break;

	case '[':
		cursor++;
		ret = __parse_json_array(cursor, end, depth + 1, &val->value.array);
		if (ret < 0)
			return ret;

		val->type = JSON_VALUE_ARRAY;
		break;

	case 't':
		if (strncmp(cursor, "true", 4) != 0)
			return -2;

		*end = cursor + 4;
		val->type = JSON_VALUE_TRUE;
		break;

	case 'f':
		if (strncmp(cursor, "false", 5) != 0)
			return -2;

		*end = cursor + 5;
		val->type = JSON_VALUE_FALSE;
		break;

	case 'n':
		if (strncmp(cursor, "null", 4) != 0)
			return -2;

		*end = cursor + 4;
		val->type = JSON_VALUE_NULL;
		break;

	default:
		return -2;
	}

	return 0;
}

static int __parse_json_members(const char *cursor, const char **end,
								int depth, json_object_t *obj)
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

		ret = __parse_json_value(cursor, &cursor, depth, &memb->value);
		if (ret < 0)
		{
			free(memb);
			return ret;
		}

		__copy_json_string(memb->name, name, len);
		__insert_json_member(memb, obj);
		cnt++;

		while (isspace(*cursor))
			cursor++;

		if (*cursor == ',')
		{
			cursor++;
			while (isspace(*cursor))
				cursor++;
		}
		else if (*cursor == '}')
			break;
		else
			return -2;
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
							   int depth, json_object_t *obj)
{
	int ret;

	if (depth >= JSON_DEPTH_LIMIT)
		return -3;

	INIT_LIST_HEAD(&obj->head);
	obj->root.rb_node = NULL;
	ret = __parse_json_members(cursor, end, depth, obj);
	if (ret < 0)
	{
		__destroy_json_members(obj);
		return ret;
	}

	obj->count = ret;
	return 0;
}

json_value_t *json_value_create(const char *doc)
{
	json_value_t *val;
	int ret;

	while (isspace(*doc))
		doc++;

	val = (json_value_t *)malloc(sizeof (json_value_t));
	if (!val)
		return NULL;

	ret = __parse_json_value(doc, &doc, 0, val);
	if (ret >= 0)
	{
		while (isspace(*doc))
			doc++;

		if (*doc)
		{
			__destroy_json_value(val);
			ret = -2;
		}
	}

	if (ret < 0)
	{
		free(val);
		return NULL;
	}

	return val;
}

void json_value_destroy(json_value_t *val)
{
	__destroy_json_value(val);
	free(val);
}

const char *json_value_string(const json_value_t *val)
{
	if (val->type != JSON_VALUE_STRING)
		return NULL;

	return val->value.string;
}

const double json_value_number(const json_value_t *val)
{
	if (val->type != JSON_VALUE_NUMBER)
		return NAN;

	return val->value.number;
}

const json_object_t *json_value_object(const json_value_t *val)
{
	if (val->type != JSON_VALUE_OBJECT)
		return NULL;

	return &val->value.object;
}

const json_array_t *json_value_array(const json_value_t *val)
{
	if (val->type != JSON_VALUE_ARRAY)
		return NULL;

	return &val->value.array;
}

const json_value_t *json_object_find(const char *name,
									 const json_object_t *obj)
{
	struct rb_node *p = obj->root.rb_node;
	json_member_t *memb;
	int n;

	while (p)
	{
		memb = rb_entry(p, json_member_t, rb);
		n = strcmp(name, memb->name);
		if (n < 0)
			p = p->rb_left;
		else if (n > 0)
			p = p->rb_right;
		else
			return &memb->value;
	}

	return NULL;
}

int json_object_count(const json_object_t *obj)
{
	return obj->count;
}

int json_object_read(const char *name[], const json_value_t *val[], int n,
					 const json_object_t *obj)
{
	struct list_head *pos = obj->head.next;
	json_member_t *memb;
	int i;

	for (i = 0; i < n; i++)
	{
		if (pos == &obj->head)
			break;

		memb = list_entry(pos, json_member_t, list);
		name[i] = memb->name;
		val[i] = &memb->value;
		pos = pos->next;
	}

	return i;
}

int json_array_count(const json_array_t *arr)
{
	return arr->count;
}

int json_array_read(const json_value_t *val[], int n,
					const json_array_t *arr)
{
	struct list_head *pos = arr->head.next;
	json_element_t *elem;
	int i;

	for (i = 0; i < n; i++)
	{
		if (pos == &arr->head)
			break;

		elem = list_entry(pos, json_element_t, list);
		val[i] = &elem->value;
		pos = pos->next;
	}

	return i;
}

