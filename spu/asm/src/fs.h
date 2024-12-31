#pragma once

#include <stdint.h>
#include <stdlib.h>

int read_file(const char* filepath, char** content);
int write_file(const char* filepath, uint8_t* content, size_t size);
