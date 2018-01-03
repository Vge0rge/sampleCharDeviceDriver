
#include <linux/slab.h>
#include "linkedList.h"

struct linkedList* initLinkedList(void){
	struct linkedList *myLinkedList = kmalloc( sizeof(struct linkedList), GFP_KERNEL);

	if(myLinkedList == NULL){
		return NULL;
	}

	myLinkedList->head = NULL;
	myLinkedList->tail = NULL;

	return myLinkedList;
}

int isLinkedListEmpty(struct linkedList *myLinkedList){
	if ( myLinkedList->head == NULL && myLinkedList->tail == NULL) {
		return 1;
	}else{
		return 0;
	}
}

int isLinkedListValid(struct linkedList *myLinkedList){
	if ( myLinkedList == NULL){
		//fprintf( stderr, "The linked list is NULL and I am sad :( \n" );
		return 0;
	}else if ( (myLinkedList->head == NULL || myLinkedList->tail == NULL) && ! isLinkedListEmpty(myLinkedList)) {
		//fprintf( stderr, "The linked  list is broken and I am sad :( \n" );
		return 0;
	}
	return 1;
}


unsigned long countItems(struct linkedList *myLinkedList){
	struct linkedListNode *myNode;
	unsigned long counter = 0;
	if ( ! isLinkedListValid(myLinkedList) ){
		return -1;
	}else if ( isLinkedListEmpty(myLinkedList)){
		return 0;
	}

	myNode = myLinkedList->head;
	do{
		myNode = myNode->nextNode;
		counter++;
	}while(myNode != NULL);

	return counter;
}

struct linkedList* insertNode( struct linkedList *myLinkedList, struct nodeDataStruct *myNodeData){
	struct linkedListNode *newNode;
	if ( ! isLinkedListValid(myLinkedList) ){
		return myLinkedList;
	}

	newNode =  kmalloc(sizeof(struct linkedListNode), GFP_KERNEL);

	if ( newNode == NULL ){
		//fprintf( stderr, "I cannot malloc a new element, I cannot add the element to the list  and I am sad :( \n" );
		return myLinkedList;
	}

	newNode->nextNode = NULL;
	newNode->nodeData = myNodeData;
	// If the linked list is empty, the newnode is the both the tail and the head
	if ( isLinkedListEmpty(myLinkedList) ){
		myLinkedList->head = newNode;
		myLinkedList->tail = newNode;
	}else{
		myLinkedList->tail->nextNode = newNode;
		myLinkedList->tail = newNode;
	}

	return myLinkedList;

}



struct nodeDataStruct* removeNode(struct linkedList *myLinkedList){
	struct nodeDataStruct *myNodeData;
	struct linkedListNode *oldHead;
	
	if ( !isLinkedListValid(myLinkedList) || isLinkedListEmpty(myLinkedList) ){
		return NULL;
	}

	// If there is only one element
	myNodeData = myLinkedList->head->nodeData;

	if ( countItems(myLinkedList) == 1 ){
		kfree(myLinkedList->head);
		myLinkedList->tail = NULL;
		myLinkedList->head = NULL;
	}else {
		oldHead = myLinkedList->head;
		myLinkedList->head = myLinkedList->head->nextNode;
		kfree(oldHead);
	}

	return myNodeData;
}


