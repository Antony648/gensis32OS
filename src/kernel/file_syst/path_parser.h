#ifndef PATH_PARSER_H
#define PATH_PARSER_H
#include "../disk/disk.h"
struct path_head
{sth
	int disk_id;
	struct path_body* first;
	
};
struct path_body
{
	char * path_content;
	struct path_body* next;
};
struct path_head* path_llist_gen(const char** path);
void path_llist_free(struct path_head* head);
#endif
