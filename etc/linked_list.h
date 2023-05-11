#include <stdio.h>
#include <stdlib.h>

#include "stats.c"

struct Node {
    struct CPU_Stats data;
    struct Node *next;
};

struct LinkedList {
    struct Node *head;
    size_t size;
};

struct Node* createNode(struct CPU_Stats);
void initLinkedList(struct LinkedList*);
void pushBack(struct LinkedList*, struct CPU_Stats);
void deleteAtPosition(struct LinkedList*, size_t);
struct Node* getAtPosition(struct LinkedList*, size_t);
size_t getSize(struct LinkedList*);
void printLinkedList(struct LinkedList*);
void freeLinkedList(struct LinkedList*);