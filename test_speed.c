#include <stdio.h>
#include <stdlib.h>
#include "json_parser.h"

#define BUFSIZE		(8 * 1024 * 1024)

int main(int argc, char *argv[])
{
	static char buf[BUFSIZE];

	if (argc != 2)
	{
		fprintf(stderr, "USAGE: %s <repeat times>\n", argv[0]);
		exit(1);
	}

	size_t n = fread(buf, 1, BUFSIZE - 1, stdin);

	if (n > 0)
		buf[n] = '\0';
	else
	{
		perror("fread");
		exit(1);
	}

	int rep = atoi(argv[1]);
	int i;

	for (i = 0; i < rep; i++)
	{
		json_value_t *val = json_value_create(buf);
		if (val)
		{
			json_value_destroy(val);
		}
		else
		{
			fprintf(stderr, "Invalid JSON document.\n");
			exit(1);
		}
	}

	return 0;
}

