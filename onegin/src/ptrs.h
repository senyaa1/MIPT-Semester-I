#include <stdint.h>

#define PTR_SHIFT 8 * 6
#define PTR_PACK(ptr, length) 	(void*)(((uint64_t)length << (PTR_SHIFT)) | (uint64_t)ptr)
#define PTR_STRIP(ptr) 		(void*)((uint64_t)ptr & (~(0xFFFFull << (6 * 8))))
#define PTR_GETLEN(ptr) 	(uint16_t)(((0xFFFFull << (PTR_SHIFT)) & (uint64_t)ptr) >> (PTR_SHIFT))
