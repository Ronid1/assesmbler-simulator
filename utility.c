/*
code for linked lists
*/
#include "definitions.h"
#include "functions.h"

#define SYMBOL_ROW ((symbolTableRow*)newLine->data)
#define DATA_ROW ((dataTableRow*)newLine->data)
#define CODE_ROW ((codeTableRow*)newLine->data)

#define ERROR {numOfErrors++; fprintf(stderr, "ERROR: can not allocate memmory.\n"); exit(0);}

/*-----------------functions for linked lists---------------*/

/*This functions adds data from a generic pointer to a linked list, by list type*/
void add2Table(ptr* head, void* lineData, int type)
{
	ptr newLine; int rowSize;

	/*allocate memory according to table type*/
	if (type == CODET)
		rowSize = sizeof(codeTableRow);


	else if (type == DATAT)
		rowSize = sizeof(dataTableRow);

	else
		rowSize = sizeof(symbolTableRow);


	newLine = (ptr)malloc(sizeof(list));
	if (!newLine) ERROR

		newLine->data = malloc(rowSize + sizeof(list));
	if (!(newLine->data)) ERROR

		if (!*head)
			(newLine)->next = NULL;

		else
			newLine->next = *head;

	/*Add data by table type*/
	switch (type) {
	case CODET: {
		CODE_ROW->IC = ((codeTableRow*)lineData)->IC;
		CODE_ROW->codeRow = ((codeTableRow*)lineData)->codeRow;
		break;
	}
	case DATAT: {
		DATA_ROW->DC = ((dataTableRow*)lineData)->DC;
		DATA_ROW->dataRow = ((dataTableRow*)lineData)->dataRow;
		break;
	}

	case SYMBOLT: {
		strcpy(SYMBOL_ROW->name, ((symbolTableRow*)lineData)->name);
		SYMBOL_ROW->addressOrValue = ((symbolTableRow*)lineData)->addressOrValue;
		SYMBOL_ROW->type = ((symbolTableRow*)lineData)->type;
		SYMBOL_ROW->external = ((symbolTableRow*)lineData)->external;
		break;
	}
	}

	*head = newLine;
}

/*This function searches symbol list by value "name". If the item was not found the function returns false*/
boolean searchTable(ptr head, char* name)
{
	while (head)
	{
		if (strcmp(((symbolTableRow*)head->data)->name, name) == 0)
			return TRUE;

		head = head->next;
	}
	return FALSE;
}

/*This function returns the value from symbol list by name, will be used after confirming value existd with searchTable*/
int macroValue(ptr head, char* name)
{
	while (head)
	{
		if (strcmp(((symbolTableRow*)head->data)->name, name) == 0)
			return ((symbolTableRow*)head->data)->addressOrValue;

		head = head->next;
	}
	return NONE;
}

/*This function returns true if the variable "name" is marked as external in the symbol table*/
boolean isExternal(ptr head, char* name)
{
	while (head)
	{
		if (strcmp(((symbolTableRow*)head->data)->name, name) == 0)
			return ((symbolTableRow*)head->data)->external;

		head = head->next;
	}

	return FALSE;
}

/*This function returns a pointer to the node with the given address in code table*/
ptr* findNode(ptr* head, int address)
{
	while (*head)
	{
		if (((codeTableRow*)(*head)->data)->IC == address)
			return head;

		*head = (*head)->next;
	}

	return NULL;
}

/*This function delets a linked list and frees memory*/
void deleteTable(ptr* head)
{
	ptr temp;

	while (*head)
	{
		temp = *head;
		*head = (*head)->next;
		free(temp);
	}
}

/*This function reverses the direction of a one-way linked list*/
void reverseList(ptr* head)
{
	ptr prev = NULL, current = *head, nextNode = NULL;

	while (current != NULL)
	{

		nextNode = current->next;
		current->next = prev;

		prev = current;
		current = nextNode;
	}

	*head = prev;
}

/*---------------------Functions for Queue-------------------*/

/*This function adds info to a new node*/
qNode* newNode(int line, int CTadress, char* str1, char* str2)
{
	qNode* temp = (qNode*)malloc(sizeof(qNode));
	if (!temp)	ERROR;
	temp->line = line;
	temp->CTadress = CTadress;

	if (str1 != NULL)
		strcpy(temp->str1, str1);
	else
		strcpy(temp->str2, "-1");
	if (str2 != NULL)
		strcpy(temp->str2, str2);
	else
		strcpy(temp->str2, "-1");

	temp->next = NULL;

	return temp;
}

/*This function creats a new empty queue*/
queue* createQueue()
{
	queue* q = (queue*)malloc(sizeof(queue));
	if (!q) ERROR;

	q->front = NULL;
	q->rear = NULL;
	return q;
}

/*This functions adds a new node to end of a queue*/
void enQueue(queue* q, int line, int CTadress, char* str1, char* str2)
{
	qNode* temp = newNode(line, CTadress, str1, str2);

	/* If queue is empty, then new node is both front and rear*/
	if (q->rear == NULL) {
		q->front = q->rear = temp;
		return;
	}

	/*Add the new node at the end of queue and change rear*/
	q->rear->next = temp;
	q->rear = temp;
}

/*remove first node from queue and return a pointer to it*/
qNode* deQueue(queue* q)
{
	qNode* temp;
	/*If queue is empty, return NULL*/
	if (q->front == NULL)
		return NULL;

	/*Store previous front and move front one node ahead*/
	temp = q->front;
	q->front = q->front->next;

	/* If front becomes NULL, then change rear also as NULL*/
	if (q->front == NULL)
		q->rear = NULL;
	return temp;
}

/*This function deletes a queue and frees memory*/
void deleteQueue(queue* q)
{
	while (q->front)
		free(deQueue(q));
}