#include "json_parser.h"
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE		(8 * 1024 * 1024)

int main()
{
	static char buf[BUFSIZE];
	size_t n = fread(buf, 1, BUFSIZE - 1, stdin);

	if (n > 0)
		buf[n] = '\0';
	else
	{
		perror("fread");
		exit(1);
	}

	json_value_t *val = json_value_create(buf);
	if (val)
	{
		printf("success!\n");
		json_value_destroy(val);
	}
	else
	{
		printf("failed!\n");
	}
	return 0;
}

