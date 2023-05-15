#include <stdio.h>
#include <assert.h>

#include "etc/linked_list.c"

void test_createNode() {
    struct CPU_Stats Data = { "Test", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    struct Node *Node = createNode(Data);
    assert(Node != NULL);
    assert(Node->data.user == 1);
    assert(Node->next == NULL);
    free(Node);
}

void test_pushBack() {
    struct LinkedList *List = (struct LinkedList*)malloc(sizeof(struct LinkedList));
    initLinkedList(List);

    struct CPU_Stats Data_1 = { "Test1", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    struct CPU_Stats Data_2 = { "Test2", 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };

    pushBack(List, Data_1);
    assert(List->size == 1);
    assert(List->head != NULL);
    assert(List->head->data.user == 1);

    pushBack(List, Data_2);
    assert(List->size == 2);
    assert(List->head->data.user == 1);
    assert(List->head->next != NULL);
    assert(List->head->next->data.user == 11);
    freeLinkedList(List);
}

void test_deleteAtPosition() {
    struct LinkedList *List = (struct LinkedList*)malloc(sizeof(struct LinkedList));
    initLinkedList(List);

    struct CPU_Stats Data_1 = { "Test1", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    struct CPU_Stats Data_2 = { "Test2", 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
    struct CPU_Stats Data_3 = { "Test3", 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 };
    pushBack(List, Data_1);
    pushBack(List, Data_2);
    pushBack(List, Data_3);

    deleteAtPosition(List, 1);
    assert(List->size == 2);
    assert(List->head->data.user == 1);
    assert(List->head->next != NULL);
    assert(List->head->next->data.user == 21);

    deleteAtPosition(List, 0);
    assert(List->size == 1);
    assert(List->head->data.user == 21);

    deleteAtPosition(List, 0);
    assert(List->size == 0);
    assert(List->head == NULL);

    deleteAtPosition(List, 0);

    freeLinkedList(List);
}

int main() {
    test_createNode();
    test_pushBack();
    test_deleteAtPosition();
    
    printf("Tests passed successfully.\n");
    return 0;
}