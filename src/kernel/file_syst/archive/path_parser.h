#ifndef PATH_PARSER_H
#define PATH_PARSER_H
#include "./virtual_file.h"
struct path_head
{
	struct mount_point_ent* mnt_pnt;
	struct path_body* first;
	
};
struct path_body
{
	char * path_content;
	struct path_body* next;
};
struct path_head* path_llist_gen(const char* path);
struct path_body* path_get_first(struct path_head* head);
struct path_body* path_get_next(struct path_body* body);
void path_llist_free(struct path_head* head);
#endif
