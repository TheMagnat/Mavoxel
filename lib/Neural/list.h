#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C"{
#endif 


struct node{

	int value;
	struct node* next;

};
typedef struct node node;


struct list{

	unsigned int size;

	node* start;
	node* end;

};
typedef struct list list;

void createList(list *l);

void deleteList(list *l);

void pushFront(list* l, int val);

int getList(list* l, unsigned int index);

void clearList(list* l);

void printList(list* l);


#ifdef __cplusplus
}
#endif 


#endif /* LIST_H */
