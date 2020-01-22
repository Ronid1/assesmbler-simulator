/*This file contains definitions and structures used across all files in the project*/

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/*--------------------------------Libraries-----------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*--------------------------------Definitions-----------------------------------*/

#define MEMORY_CELL_SIZE 14
#define MAX_LINE_LENGTH 82 /*80chars + '\0' + '\n'*/
#define WORDS_IN_LINE 40 /*max of 40 words in a line'*/
#define LABLE_LENGTH 32 /*31 chars + '\0'*/
#define FIRST_ADDRESS 100
#define FILE_NAME_LENGTH 83 /*I'm assuming a file name will be no longet then 80char + .as ending*/
#define NUM_OF_OPCODES 16
#define NUM_OF_INSTRUCTIONS 4
#define NUM_OF_REGISTERES 8
#define MACRO_STATMENT ".define"
#define NONE -1
#define SAVE_ENTRY -1
#define OUTPUT_WORD 7


typedef enum {FALSE, TRUE} boolean;
enum addressingMethod {imidiate, direct, constent, registers};
enum TYPE { INSTUCTION, OPCODE, MACRO, LABLE };
enum INSTRUCTION { DATA, STRING, ENTRY, EXTERN };
enum { mov, cmp, add, sub, not, clr, lea, inc, dec, jmp, bne, red, prn, jsr, rts, stop };
enum tableType { CODET, DATAT, SYMBOLT };

/*Arry of opcods, define for each code: name, code, num of operands*/
typedef struct {
	char* codeName;
	int codeSymble;
	int NumOfOperands;
}operationCodes;

/*-------------------------------Structs for managing data---------------------------------*/
/*Memory cell of 14 bits to save data converted to binery*/
typedef struct dataMemoryCell {
	unsigned int value : MEMORY_CELL_SIZE;
}dataMemoryCell;


/*A 'word' as defined in the beginning of the project, each bit represents something else from input*/
typedef struct wordMemoryCell {
	unsigned int ARE : 2;
	unsigned int destination : 2;
	unsigned int source : 2;
	unsigned int opcode : 4;
	unsigned int unused : 4;
}wordMemoryCell;

/*An addressing value that contains a,r,e bits and 12 bit to represent address / value*/
typedef struct addressingMemoryCell {
	unsigned int ARE : 2;
	unsigned int value : MEMORY_CELL_SIZE-2;
}addressingMemoryCell;

/*An addressing value that contains a,r,e bits, 3 bits for each register*/
typedef struct registerMemoryCell {
	unsigned int ARE : 2;
	unsigned int des : 3;
	unsigned int source : 3;
	unsigned int rest : MEMORY_CELL_SIZE-8;
}registerMemoryCell;

/*Code row can either contain a word or an addressing value*/
typedef union info {
	wordMemoryCell word;
	addressingMemoryCell addressing;
	unsigned value: MEMORY_CELL_SIZE; /*TO DO: WAS MEMORY_CELL_SIZE - 2;*/
	registerMemoryCell registercell;
}info;

/*-------------------------------------Data for linked lists-------------------------------------------------*/

/*Definition of node in linked list*/
typedef struct node* ptr;

typedef struct node {
	void* data;
	ptr next;
}list;

/*Data stored in each row in code table*/
typedef struct codeTableRow {
	int IC;
	info codeRow;
}codeTableRow;

/*Data stored in each row in data table*/
typedef struct dataTableRow {
	int DC;
	dataMemoryCell dataRow;
}dataTableRow;

/*Data stored in each row in symbole table*/
typedef struct symbolTableRow {
	char name[LABLE_LENGTH];
	int addressOrValue;
	int type; /*CHECK WHAT KIND OF VARIABLE NEEDED*/
	boolean external;
}symbolTableRow;

/*-------------------------------------Data for Queue-------------------------------------------------*/
/*Linked list for queue application*/
typedef struct qNode* qPtr;

typedef struct qNode {
	int line;
	int CTadress;
	char str1[LABLE_LENGTH];
	char str2[LABLE_LENGTH];
	qPtr next;
}qNode;

/*First and last nodes in queue*/
typedef struct queue {
	qNode *front, *rear;
}queue;

/*-------------------------External variables-------------------------------------*/
extern int numOfErrors;
extern ptr symbolHead;
extern ptr dataHead;
extern ptr codeHead;
extern list codeTable;
extern list dataTable;
extern list symbolTable;
extern queue* missingInfo;
extern char* registerNames[NUM_OF_REGISTERES];
extern int ic;
extern int dc;
extern char* fileName;
extern operationCodes opcodes[NUM_OF_OPCODES];
/*-----------------------------------------------------------------------------------*/

#endif