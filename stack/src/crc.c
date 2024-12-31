#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "crc.h"

static uint32_t crc_table[256];
static bool is_table_initialized = false;

static void init_crc32_table() 
{
	uint32_t polynomial = 0xEDB88320;
	for (uint32_t i = 0; i < 256; i++) 
	{
		uint32_t crc = i;
		for (uint8_t j = 0; j < 8; j++) 
		{
			if (crc & 1)
				crc = (crc >> 1) ^ polynomial;
			else
				crc = crc >> 1;
		}
	        crc_table[i] = crc;
	}

	is_table_initialized = true;
}

uint32_t crc32(const char *data, size_t length) 
{
	if(!is_table_initialized)
		init_crc32_table();
	
	uint32_t crc = 0xFFFFFFFF;  
	for (size_t i = 0; i < length; i++) 
	{
		uint8_t index = (crc ^ data[i]) & 0xFF;
		crc = (crc >> 8) ^ crc_table[index];
	}
	return crc ^ 0xFFFFFFFF;  
}
