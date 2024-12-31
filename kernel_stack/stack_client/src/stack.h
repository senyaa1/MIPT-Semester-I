#include <stdlib.h>
#include "kernel_stack.h"

stack_status_t stack_get_and_print(stack_num num);
stack_status_t stack_print(stack_data_t* stack, void* buf);
stack_status_t stack_get_buf(stack_num st_num, void* buf_ptr);
stack_status_t stack_get(stack_num st_num, stack_data_t* stack_data);
stack_status_t stack_pop(stack_num st_num, void* resulting_data);
stack_status_t stack_push(stack_num st_num, const void* input_data);
stack_status_t stack_destroy(stack_num st_num);
stack_num stack_init(size_t elem_size, size_t initial_size);
stack_status_t stack_print_err(stack_status_t status);
