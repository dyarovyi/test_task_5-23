#include "linked_list.h"

struct Node* createNode(struct CPU_Stats data) {
    struct Node *NewNode = (struct Node*)malloc(sizeof(struct Node));
    if (NewNode == NULL) {
        printf("Memory allocation failed.\n");
        return NULL;
    }
    NewNode->data = data;
    NewNode->next = NULL;
    return NewNode;
}

void initLinkedList(struct LinkedList *list) {
    list->head = NULL;
    list->size = 0;
}

struct LinkedList* copyLinkedList(const struct LinkedList *OriginalList) {
    struct LinkedList *NewList = (struct LinkedList*)malloc(sizeof(struct LinkedList));
    if (NewList == NULL) {
        return NULL;
    }

    initLinkedList(NewList);

    struct Node *CurrentNode = OriginalList->head;
    struct Node *PreviousNode = NULL;

    while (CurrentNode != NULL) {
        struct Node *NewNode = (struct Node*)malloc(sizeof(struct Node));
        if (NewNode == NULL) {
            freeLinkedList(NewList);
            return NULL;
        }

        NewNode->data = CurrentNode->data;
        NewNode->next = NULL;

        if (PreviousNode == NULL) {
            NewList->head = NewNode;
        } else {
            PreviousNode->next = NewNode;
        }

        NewList->size++;

        PreviousNode = NewNode;
        CurrentNode = CurrentNode->next;
    }

    return NewList;
}

void pushBack(struct LinkedList *list, struct CPU_Stats data) {
    struct Node *NewNode = createNode(data);
    if (NewNode == NULL) {
        return;
    }
    if (list->head == NULL) {
        list->head = NewNode;
    } else {
        struct Node *currentNode = list->head;
        while (currentNode->next != NULL) {
            currentNode = currentNode->next;
        }
        currentNode->next = NewNode;
    }
    list->size++;
}

void deleteAtPosition(struct LinkedList *list, size_t position) {
    if (list->head == NULL) {
        printf("Linked list is empty.\n");
        return;
    }
    if (position >= list->size) {
        printf("Invalid position.\n");
        return;
    }
    struct Node *CurrentNode = list->head;
    struct Node *PreviousNode = NULL;
    size_t currentPosition = 0;
    while (currentPosition < position) {
        PreviousNode = CurrentNode;
        CurrentNode = CurrentNode->next;
        currentPosition++;
    }
    if (PreviousNode == NULL) {
        // Deleting the head node
        list->head = CurrentNode->next;
    } else {
        PreviousNode->next = CurrentNode->next;
    }
    free(CurrentNode);
    list->size--;
}

struct Node* getAtPosition(struct LinkedList *list, size_t position) {
    if (list->head == NULL) {
        printf("Linked list is empty.\n");
        return NULL;
    }
    if (position >= list->size) {
        printf("Invalid position.\n");
        return NULL;
    }
    struct Node *CurrentNode = list->head;
    size_t currentPosition = 0;
    while (currentPosition < position) {
        CurrentNode = CurrentNode->next;
        currentPosition++;
    }
    return CurrentNode;
}

size_t getSize(struct LinkedList *list) {
    return list->size;
}

void printLinkedList(struct LinkedList *list) {
    if (list->head == NULL) {
        printf("Linked list is empty.\n");
        return;
    }
    struct Node *CurrentNode = list->head;
    printf("Elements in the linked list:\n");
    while (CurrentNode != NULL) {
        struct CPU_Stats stats = CurrentNode->data;
        printf("Name: %s ", stats.name);
        printf("User: %llu ", stats.user);
        printf("Nice: %llu ", stats.nice);
        printf("System: %llu ", stats.system);
        printf("Idle: %llu ", stats.idle);
        printf("IOWait: %llu ", stats.iowait);
        printf("IRQ: %llu ", stats.irq);
        printf("SoftIRQ: %llu ", stats.softirq);
        printf("Steal: %llu ", stats.steal);
        printf("Guest: %llu ", stats.guest);
        printf("Guest nice: %llu ", stats.guest_nice);
        printf("\n");
        CurrentNode = CurrentNode->next;
    }
    printf("\n");
}

void freeLinkedList(struct LinkedList *list) {
    struct Node *CurrentNode = list->head;
    while (CurrentNode != NULL) {
        struct Node *NextNode = CurrentNode->next;
        free(CurrentNode);
        CurrentNode = NextNode;
    }
    list->head = NULL;
    list->size = 0;
}