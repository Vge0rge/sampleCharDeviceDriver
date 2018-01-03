
typedef struct nodeDataStruct{
	char *charData;
	unsigned long charSize;	
}nodeDataStruct;

typedef struct linkedListNode{
	struct nodeDataStruct *nodeData;
	struct linkedListNode *nextNode;
}linkedListNode;


typedef struct linkedList{
	struct linkedListNode *head;
	struct linkedListNode *tail;

}linkedList;

struct linkedList* initLinkedList(void);
int isLinkedListEmpty(struct linkedList *myLinkedList);
int isLinkedListValid(struct linkedList *myLinkedList);
unsigned long countItems(struct linkedList *myLinkedList);
struct linkedList* insertNode( struct linkedList *myLinkedList, struct nodeDataStruct *myNodeData);
struct nodeDataStruct* removeNode(struct linkedList *myLinkedList);



