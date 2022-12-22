#ifndef LIST_H
#define LIST_H

struct _Node
{
    int data;
    struct _Node *next;
};

typedef struct _Node Node;

typedef struct _List
{
    Node *head;
    int size;
} List;

List *list_new();

void list_free(List *list);

int list_insert(List *list, int data);

#endif