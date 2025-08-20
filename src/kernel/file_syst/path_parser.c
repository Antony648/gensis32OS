#include "path_parser.h"
#include  "../ctype/ctype.h"
#include "../heap/heap_cream.h"
uintptr_t karray[5];

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
static char* gen_str(char*target,int start,int end)
{
	if(end<start)			//if end is less than start it will cause a large unsigned 
		return NULL;
	size_t size=(size_t)(end-start+1);
	char *str=heap_cream_malloc(karray,size);
	for(int i=0;i<size-1;i++)
		str[i]=target[start+i];
	str[size-1]='\0';
	return str;
}
struct path_head* path_llist_gen(const char* target)
{
	heap_cream_init(karray);
	if(!isdigit(target[0]))
		return NULL;
	if(target[1]!=':' || target[2] !='/')
		return NULL;
	int j=0;
	//we can create a head at this point
	struct path_head* head=(struct path_head*)heap_cream_malloc(sizeof(struct path_head));
	head->disk_id=char_to_int(target[0]);
	if( target[3]=='\0')
	{
		head->first=NULL;
		return head;
	}
	
	struct path_body* pb=(struct path_body*)heap_cream_malloc(sizeof(struct path_body));
	head->first=pb;
	
	struct path_body* cur=pb;struct path_body* nex=NULL;
	pb->next=NULL;
	int start_index,end_index;char* temp;
	
	for(int i=2;target[i];i++)
	{
		if(target[i]=='/')
		{
			start_index=i+1;
			int j=i+1;	//i++
			while(target[j]!='/' && target[j]!='\0') //replace j with i
				j++;		//i++;
			end_index=j-1;	//i-- ; end_index=i;
			i =end_index;
			
			if(start_index-1 == end_index)	// prevents "//' from allocatin a blank
				continue;
			if(start_index ==end_index)		//prevents extra path body for a path that ends in /
				continue;	
			
			temp=gen_str(target,start_index,end_index);
			if(temp)
				cur->path_content=temp;
			if(target[j]!='\0')
			{
				//not end yet
				nex=(struct path_body*)heap_cream_malloc(sizeof(struct path_body));
				cur->next=nex;
				cur=nex; nex=NULL;
			}
			else
				cur->next=NULL;
		}
		if(head->first->path_content==NULL)
		{
			//only for a special condition where we have only a disk no
			heap_cream_free(karray,(void*)(head->first));
			head->first=NULL;
		}
	}
	return head;
	
}
static void path_body_free(struct path_body* body)
{
	if(body->next !=NULL)
		path_body_free(body->next);
	heap_cream_free(body->path_content);
	heap_cream_free(body);
	return;
}
void path_llist_free(struct path_head* head)
{
		if(head->first!=NULL)
			path_body_free(head->first);
		heap_cream_free(head);	
		return;
}

