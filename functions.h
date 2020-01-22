/*This file contains functions used in multipul files*/

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

/*------------------First Pass------------------*/
extern void firstPass(FILE* fd);
extern boolean legalLableMacro(char* name);
int opType(char* operand);
boolean legalNum(char* num);

/*-------------------utility----------------------*/
void add2Table(ptr* head, void* lineData, int type);
boolean searchTable(ptr head, char* name);
int macroValue(ptr head, char* name);
ptr* findNode(ptr* head, int address);
boolean isExternal(ptr head, char* name);
void deleteTable(ptr* head);
void reverseList(ptr* head);
qNode* newNode(int line, int CTadress, char* str1, char* str2);
queue* createQueue();
void enQueue(queue* q, int line, int CTadress, char* str1, char* str2);
qNode* deQueue(queue* q);
void deleteQueue(queue* q);

/*------------------Second Pass------------------*/
extern void secondPass(FILE* fd);

#endif