#include <stdio.h>
#include <stdlib.h>
#include "json_parser.h"

#define BUFSIZE		(64 * 1024 * 1024)

int main(int argc, char *argv[])
{
	static char buf[BUFSIZE];
	size_t n;

	if (argc != 2)
	{
		fprintf(stderr, "USAGE: %s <repeat times>\n", argv[0]);
		exit(1);
	}

	n = fread(buf, 1, BUFSIZE, stdin);
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

	int rep = atoi(argv[1]);
	int i;

	for (i = 0; i < rep; i++)
	{
		json_value_t *val = json_value_parse(buf);
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

