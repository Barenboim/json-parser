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
	int n;
};

struct __json_array
{
	struct list_head head;
	int n;
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

static int __parse_json_members(const char *cursor, const char **end,
								json_object_t *obj)
{
	json_member_t *memb;
	const char *name;
	int cnt = 0;
	size_t size;
	int ret;
	int len;

	while (1)
	{
		while (isspace(*cursor))
			cursor++;

		if (*cursor == '}')
			break;

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
			return -2;

		cursor++;
	}

	obj->n = cnt;
	return cnt;
}

static int __parse_json_object(const char *cursor, const char **end,
							   json_object_t *obj)
{
	int ret;

	ret = __parse_json_members(cursor, end, obj);
	if (ret < 0 || *end)
	{
		if (*end)
			ret = -2;

	//	destroy_json_object(obj);
		return ret;
	}

	return 0;
}

#include <stdio.h>

int main()
{
	static char buf[4096];
	const char *end;
	json_object_t obj;
	while (1)
	{
		scanf("%s", buf);
		if (*buf == '{')
			__parse_json_object(buf + 1, &end, &obj);
	}

	return 0;
}
