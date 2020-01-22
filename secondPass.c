/*This file contains all functions for second pass - adding missing information to code table,
 printing entrys to .ent file, printing extenals to .ext file.
Translating code table to special 4 base and printing in .ob file*/

#include "definitions.h"
#include "functions.h"
#define ORIGIN 0
#define DES 1

/*----------------Global variables------------------*/
int lineNumber;
FILE* ob, *ent, *ext;
ptr headCpy;

/*----------------External variables------------------*/
int statement;
char* instructions[NUM_OF_INSTRUCTIONS];
int ic;
int dc;
int numOfErrors;
char* fileName;
queue* missingInfo;
operationCodes opcodes[NUM_OF_OPCODES];

/*------------Function declerations----------------------*/
void addEntry(char* name);
void addCode(int address, char* origin, char* des);
void addImidiate(char* name, ptr* location);
void addImidiate(char* name, ptr* location);
void addRegister(char* name, ptr* location, int address);
void addDirect(char* name, ptr* location, int address);
void addConst(char* name, ptr* location, int address);
void creatObFile(ptr cHead, ptr dHead);
char* translate2Binery (unsigned num, char buffer[MEMORY_CELL_SIZE]);
char* specialBase(char bineryNum[MEMORY_CELL_SIZE], char num[OUTPUT_WORD]);
boolean legalEntry(char* name);
/*--------------------------------------------------------*/

/* Main function for handeling information in second pass. All information needing attention in second pass was stored in a queue in the first pass.
Each line marked it the queue is sent to a spesipic function according to its type. after the queue is empty, if there are'nt any addition errors
.ob file is created and .ext & .ent are deleted if not in use */
void secondPass(FILE* fd)
{
	char fileOb[FILE_NAME_LENGTH]; char fileEnt[FILE_NAME_LENGTH]; char fileExt[FILE_NAME_LENGTH];
	int size = 1;
	numOfErrors = 0; lineNumber = 0;

	/*copy head pointer from code table*/
	headCpy = codeHead;

	/*creat files*/;
	strcpy(fileOb, fileName);
	strcat(fileOb, ".ob");

	strcpy(fileEnt, fileName);
	strcat(fileEnt, ".ent");

	strcpy(fileExt, fileName);
	strcat(fileExt, ".ext");

	ob = fopen(fileOb, "w+");
	ent = fopen(fileEnt, "w+");
	ext = fopen(fileExt, "w+");

	while (!feof(fd) && missingInfo->front != NULL)
	{
		char line[MAX_LINE_LENGTH];

		/*insert first line in an arry*/
		if (fgets(line, MAX_LINE_LENGTH, fd) == NULL)
			break;

		lineNumber++;

		/*if line isnt in queue - it dosent need attention in second pass*/
		if (missingInfo->front->line != lineNumber)
			continue;

		/*if its an entry*/
		if (missingInfo->front->CTadress == SAVE_ENTRY)
			addEntry(missingInfo->front->str1);

		/*its an opcode*/
		else
			addCode(missingInfo->front->CTadress, missingInfo->front->str1, missingInfo->front->str2);

		/*done with info from this node, delete it*/
		free(deQueue(missingInfo));

	}

	/*if errors found in second pass - delete all files*/
	if (numOfErrors != 0)
	{
		remove(fileEnt);
		remove(fileExt);
		remove(fileOb);
		return;
	}

	/*creat code file, based on code and data tables*/
	creatObFile(codeHead, dataHead);

	/*if entry file is empty - delete it*/
	if (ent)
	{
		fseek(ent, 0, SEEK_END);
		size = ftell (ent);
	}

	if (size == 0)
		remove(fileEnt);

	size = 1;

	/*if external file is empty - delete it*/
	if (ext)
	{
		fseek(ext, 0, SEEK_END);
		size = ftell (ext);
	}

	if (size == 0)
		remove(fileExt);

	/*close files*/
	fclose(ob);
	fclose(ent);
	fclose(ext);
}

/*This function addes an entrys information to .ent file*/
void addEntry(char* name)
{
	int value;

	/*check enty is legal*/
	if (!legalEntry(name))
	{
		numOfErrors++;
		return;
	}

	/*check if entry was intialized, if not - error*/
	if (!searchTable(symbolHead, name))
	{
		numOfErrors++;
		fprintf(stderr, "ERROR: Entry %s not found in symbol table.\tline: %d\n", name, lineNumber);
		return;
	}

	value = macroValue(symbolHead, name);

	/*add info to entry file*/
	fprintf(ent, "%s %04d\n", name, value);
}

/*This function addes missing information to code table regarding opcodes*/
void addCode(int address, char* origin, char* des)
{
	int originType, desType; ptr* addHere;

	/*find first address in code table to fill info regarding this opcode*/
	addHere = findNode(&headCpy, address);

	/*find addressing types*/
	originType = opType(origin);
	desType = opType(des);

	switch (originType) {
	case imidiate: addImidiate(origin, addHere); break;
	case registers: addRegister(origin, addHere, ORIGIN); break;
	case constent: addConst(origin, addHere, address); break;
	case direct: addDirect(origin, addHere, address); break;
	default: return;
	}
	
	/*find starting point for second word according to addressing type*/
	if (originType != registers || (originType == registers && desType != registers))
		addHere = findNode(&headCpy, ++address);

	if (originType == constent)
		addHere = findNode(&headCpy, ++address);

		
	switch (desType) {
	case imidiate: addImidiate(des, addHere); return;
	case registers:	addRegister(des, addHere, DES); return;
	case constent: addConst(des, addHere, address); return;
	case direct: addDirect(des, addHere, address); return;
	default: return;
	}

}

/*This function addes an imidiate addressing value to code table*/
void addImidiate(char* name, ptr* location)
{
	int number; char* numStr;

	numStr = strtok(name, " \t#");

	/*if its not a number, its a macro*/
	if (!legalNum(numStr))
		number = macroValue(symbolHead, numStr);

	else
		number = atoi(numStr);

	/*add info to code table*/
	((codeTableRow*)((*location)->data))->codeRow.addressing.ARE = 0;
	((codeTableRow*)((*location)->data))->codeRow.addressing.value = number;
}

/*This function addes a direct addressing value to code table*/
void addDirect(char* name, ptr* location, int CTaddress)
{
	boolean extr; int address;

	name = strtok(name, " \t\n");
	address = macroValue(symbolHead, name);
	extr = isExternal(symbolHead, name);

	/*if name wasnt found in symbol table*/
	if (address == NONE)
	{
		fprintf(stderr, "ERROR: Unrecognized variable %s.\tline: %d\n", name, lineNumber);
		numOfErrors++;
		return;
	}

	if (extr == TRUE)
	{
		/*add to external file with current CTadress address*/
		fprintf(ext, "%s %04d\n", name, CTaddress);

		((codeTableRow*)((*location)->data))->codeRow.addressing.ARE = 1;
		((codeTableRow*)((*location)->data))->codeRow.addressing.value = address;
	}

	else
	{
		/*add to code table*/
		((codeTableRow*)((*location)->data))->codeRow.addressing.ARE = 2;
		((codeTableRow*)((*location)->data))->codeRow.addressing.value = address;
	}
}

/*This function addes a constent addressing value to code table*/
void addConst(char* arr, ptr* location, int CTaddress)
{
	boolean extr; int address, index; char *indexStr, *name;

	/*find name and index*/
	name = strtok(arr, " \t[");
	indexStr = strtok(NULL, "]");

	address = macroValue(symbolHead, name);
	extr = isExternal(symbolHead, name);

	/*if name wasnt found in symbol table*/
	if (address == NONE)
	{
		fprintf(stderr, "ERROR: Unrecognized variable %s.\tline: %d\n", name, lineNumber);
		numOfErrors++;
		return;
	}

	/*if its not a number, its a macro*/
	if (!legalNum(indexStr))
		index = macroValue(symbolHead, indexStr);

	else
		index = atoi(indexStr);

	if (extr == TRUE)
	{
		/*add to external file with current CTadress address*/
		fprintf(ext, "%s %04d\n", name, CTaddress);

		((codeTableRow*)((*location)->data))->codeRow.addressing.ARE = 1;
		((codeTableRow*)((*location)->data))->codeRow.addressing.value = address;
	}

	else
	{
		((codeTableRow*)((*location)->data))->codeRow.addressing.ARE = 2;
		((codeTableRow*)((*location)->data))->codeRow.addressing.value = address;
	}

	/*add second word*/
	location = findNode(&headCpy, ++CTaddress);

	((codeTableRow*)((*location)->data))->codeRow.addressing.ARE = 0;
	((codeTableRow*)((*location)->data))->codeRow.addressing.value = index;
}

/*This function addes a register addressing value to code table*/
void addRegister(char* name, ptr* location, int address)
{
	int regist, i;

	for (i = 0; i < NUM_OF_REGISTERES; i++)
		if (strcmp(name, registerNames[i]) == 0)
			regist = i;


	if (address == ORIGIN)
	{
		((codeTableRow*)((*location)->data))->codeRow.registercell.ARE = 0;
		((codeTableRow*)((*location)->data))->codeRow.registercell.source = regist;
	}

	else
	{
		((codeTableRow*)((*location)->data))->codeRow.word.ARE = 0;
		((codeTableRow*)((*location)->data))->codeRow.registercell.des = regist;
	}

}

/*This function writes info to object file*/
void creatObFile(ptr cHead, ptr dHead)
{
	char newNum[OUTPUT_WORD], bineryNum[MEMORY_CELL_SIZE]; char *bNum, *sNum;

	fprintf(ob, "   %d %d\n", ic - 100, dc);

	while (cHead)
	{
		bNum = translate2Binery (((codeTableRow*)cHead->data)->codeRow.value, bineryNum);
		sNum = specialBase(bNum, newNum);

		fprintf(ob, "%04d %s\n", (((codeTableRow*)cHead->data)->IC), sNum);
		cHead = cHead->next;
	}

	while (dHead)
	{
		bNum = translate2Binery (((dataTableRow*)dHead->data)->dataRow.value, bineryNum);
		sNum = specialBase(bNum, newNum);

		fprintf(ob, "%04d %s\n", (((dataTableRow*)dHead->data)->DC) + ic, sNum);
		dHead = dHead->next;
	}
}

/*This function translets a memory word to a binery string*/
char* translate2Binery (unsigned num, char buffer[MEMORY_CELL_SIZE])
{
	int i;

	for (i = MEMORY_CELL_SIZE-1; i>=0; i--)
	{
		buffer[i]= (num & 1) + '0';
		num >>=1;
	}

	buffer[MEMORY_CELL_SIZE] = '\0';
	
	return buffer;
	
}

/*function for translating a binery string into spacial base 4*/
char* specialBase(char* bineryNum, char num[OUTPUT_WORD])
{
	int i, k = 0; char digit[2];

	for (i = 0; i <MEMORY_CELL_SIZE; i += 2)
	{
		digit[0] = bineryNum[i]; digit[1] = bineryNum[i+1];

		if (strcmp(digit, "00") == 0)
			num[k] = '*';

		else if (strcmp(digit, "01") == 0)
			num[k] = '#';

		else if (strcmp(digit, "10") == 0)
			num[k] = '%';

		else if (strcmp(digit, "11") == 0)
			num[k] = '!';

		k++;
	}

	num[OUTPUT_WORD] = '\0';
	return num;
}

/*This function returns true if a string is a legal entry name*/
boolean legalEntry(char* name)
{
	int i = 0, nameLen = strlen(name);

	if (strcmp(name, "\0") == 0)
		return TRUE;

	if (!isalpha(name[i]))
	{
		fprintf(stderr, "ERROR: Enty name must start with a letter.\tline: %d\n", lineNumber);
		return FALSE;
	}

	i++;

	if (nameLen > LABLE_LENGTH)
	{
		fprintf(stderr, "ERROR: Entry name too long.\tline: %d\n", lineNumber);
		return FALSE;
	}

	while ((name[i] != '\0') && (name[i] != '\n') && (name[i] != '\t') && (name[i] != ' '))
	{
		if (!isalnum(name[i]))
		{
			fprintf(stderr, "ERROR: Entry name contains invalid character.\tline: %d\n", lineNumber);
			return FALSE;
		}

		i++;
	}

	/*check if lable/macro name is a saved word*/
	for (i = 0; i < NUM_OF_OPCODES; i++)
	{
		if (strcmp(name, opcodes[i].codeName) == 0)
		{
			fprintf(stderr, "ERROR: Entry name can not be the same as an opcode name.\tline: %d\n", lineNumber);
			return FALSE;
		}
	}

	for (i = 0; i < NUM_OF_REGISTERES; i++)
	{
		if (strcmp(name, registerNames[i]) == 0)
		{
			fprintf(stderr, "ERROR: Entry name can not be the same as a register name.\tline: %d\n", lineNumber);
			return FALSE;
		}
	}

	/*search symbol table if entry name already saved as extern retrun false*/
	if (isExternal(symbolHead, name) == TRUE)
	{
		fprintf(stderr, "ERROR: Same name %s for entry and extern.\tline: %d\n", name, lineNumber);
		return FALSE;
	}

	return TRUE;
}