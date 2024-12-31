#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "fs.h"

int read_file(const char* filepath, char** content)
{
	int fd = open(filepath, O_RDONLY);
	if (fd == -1)
		return -1;

	struct stat st = {0};

	fstat(fd, &st);
	off_t sz = st.st_size;

	char* fileptr = (char*)mmap(NULL, sz, PROT_READ, MAP_PRIVATE, fd, 0);

	*content = (char*)calloc(sz, sizeof(char));
	memcpy(*content, fileptr, sz);
	munmap(fileptr, sz);
	close(fd);

	return sz;
}

