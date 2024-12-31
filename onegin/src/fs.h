#include <unistd.h>

// globals are justified. 
// there is no reason to pass 3 buffers in and out of load_poem, 
// there is only one poem in the program context.
extern size_t poem_lines;
extern wchar_t** poem;

bool load_poem(const char* poem_path);
bool write_poem(const char* poem_path);
void unload_poem();

