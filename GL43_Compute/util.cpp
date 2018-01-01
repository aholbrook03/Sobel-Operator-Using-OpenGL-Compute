#include "util.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

int readFile(char *filename, char **output)
{
	FILE *fp = fopen(filename, "r");
	int rv = 0;
	if (fp != NULL)
	{
		char buf[4096];
		int count = 0;
		int read_chars = fread(buf, 1, 4095, fp);
		while (read_chars != 0)
		{
			buf[read_chars] = '\0';
			*output = (char *)realloc(*output, count + read_chars + 1);
			strncpy(*output + count, buf, read_chars + 1);
			count += read_chars;
			read_chars = fread(buf, 1, 4095, fp);
		}

		rv = 1;
	}

	fclose(fp);
	return rv;
}
