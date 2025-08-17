#include "path_parser.h"
#include  "../ctype/ctype.h"

/*struct path_head
{
	int disk_id;
	struct path_body* first;
	
};
struct path_body
{
	char * path_content;
	struct path_body* next;
};*/
struct path_head* path_llist_gen(const char** full_path)
{
	if(!isdigit(**path))
		return NULL;
	
	if(*path[1]!= ':' || *path[2]!= '/') //we have donot have :/ followingthe id
	{
		print("path_parser: no :/ following disk id");
		return NULL;
	}
	//at this point we have checked  digit:/
	
	struct path_head ph=kzalloc(sizeof(struct path_head));
	ph.disk_id=char_to_int(*path);
	ph.first=NULL;
	//going to implement 32 block kalloc becuase i think we are wasting space 
	//trying to allocate 4kb for a struct
	
}
void path_llist_free(struct path_head* head)
{
	
}

