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
		json_object_t object;
		json_array_t array;
		char *string;
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
			case '"':
				*dest = '"';
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
		if (*cursor == '"')
			break;

		if (!*cursor)
			return -2;

		cursor++;
		if (cursor[-1] == '\\')
		{
			switch (*cursor)
			{
			case '"':
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

	defaut:
		return -2;
	}

	return 0;
}

static int __parse_json_object(const char *cursor, const char **end,
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
		return 0;

	while (1)
	{
		if (*cursor != '"')
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

void destroy_json_object(json_object_t *obj)
{
	struct list_head *pos, *tmp;
	json_member_t *memb;

	list_for_each_safe(pos, tmp, &obj->head)
	{
		memb = list_entry(pos, json_member_t, list);
		list_del(pos);
		rb_erase(&memb->rb, &obj->root);
	//	__destroy_json_member(memb);
	}

	free(obj);
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

	INIT_LIST_HEAD(&obj->head);
	obj->root.rb_node = NULL;
	ret = __parse_json_object(doc + 1, &doc, obj);
	if (ret < 0)
	{
		destroy_json_object(obj);
		return NULL;
	}

	obj->count = ret;
	return obj;
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
		" \"\\t\\/name1\" : \"value2\" }"
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
