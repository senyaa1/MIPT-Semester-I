#include <stdbool.h>
#include <stdio.h>

#include <jason/json.h>
#include <jason/serializer.h>
#include <jason/deserializer.h>

#include "fs.h"
#include "graph.h"
#include "akinator.h"
#include "color.h"


int main(int argc, char** argv)
{
	json_object_t* root = 0;
	json_value_t val = (json_value_t){0};
	char* json_text = 0;

	size_t len = read_file(JSON_DB_PATH, &json_text);

	if(!len)
	{
		root = (json_object_t*)calloc(1, sizeof(json_object_t));
		val = (json_value_t){.type = JSON_OBJECT, .value.obj = root};
	}
	else
	{
		val = json_deserialize(json_text);
		root = val.value.obj;
	}

	while(1)
	{
		printf(WHITE "[g]uess     [d]efinition    [c]ompare    [r]ender graph    [q]uit\n" RESET);
		char res = getchar();

		switch(res)
		{
			case 'g':
				guess(&val);
				break;
			case 'd':
				definition(root);
				break;
			case 'c':
				compare(root);
				break;
			case 'r':
				render_graph(&val, GRAPH_IMG_PATH);
				system("imv " GRAPH_IMG_PATH);
				break;
			case 'q':
				goto ex;
				break;
			default:
				fprintf(stderr, RED "Unknown command\n" RESET);
				break;
		}

		while ( getchar() != '\n' && getchar() != EOF )  
			;
	}

ex:
	char* serialized = json_serialize(&val);
	write_file(JSON_DB_PATH, serialized, strlen(serialized));

	return 0;
}

