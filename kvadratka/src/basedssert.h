#include <stdio.h>
#include <stdlib.h>


#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#ifdef RELEASE
#define basedssert(expr);
#else
#define basedssert(...) if(__VA_ARGS__) { \
	printf(RED "\nwhat the hell is that???? %s" YEL "\n" RESET WHT "\t\t\t   ^^^^ this happened in %s on line #%d\n" RESET, (#__VA_ARGS__), __FILE__, __LINE__); abort(); }
#endif
