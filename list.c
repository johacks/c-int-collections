#include <stdlib.h>
#include <stdio.h>
#include "list.h"

List *list_new()
{
    List *list;

    list = (List *)malloc(sizeof(List));

    if (!list)
        return NULL;

    list->head = NULL;
    list->size = 0;
}

void list_free(List *list)
{
    Node *current, *next;

    if (!list)
        return;

    current = list->head;

    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }

    free(list);
}

int list_insert(List *list, int data)
{
    Node *node;

    node = (Node *)malloc(sizeof(Node));
    if (!node)
        return 0;

    node->data = data;
    node->next = list->head;
    list->head = node;
    list->size += 1;
    
    return 1;
}