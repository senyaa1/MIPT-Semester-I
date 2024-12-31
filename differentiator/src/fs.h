#pragma once

#include <stdint.h>
#include <stdlib.h>

const size_t INPUT_BUF_SZ = 1024;

size_t read_file(const char* filepath, char** content);
int write_file(const char* filepath, char* content, size_t size);
