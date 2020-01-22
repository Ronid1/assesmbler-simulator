/*This file containes all functions needed for first pass - reading and phrasing info from file, printing errors if found,
adding each line to the relevent table (symbol, data or code) and saving information that cannot be proccessed at this point
in a queue for easy access in the second pass*/

#include "definitions.h"
#include "functions.h"

#define KIND	statement == MACRO? "macro" : "lable"

/*----------------global variables------------------*/
int lineNum;
int ic;
int dc;
operationCodes opcodes [NUM_OF_OPCODES] = {
	{"mov", 0, 2},
	{"cmp", 1, 2},
	{"add", 2, 2},
	{"sub", 3, 2},
	{"not", 4, 1},
	{"clr", 5, 1},
	{"lea", 6, 2},
	{"inc", 7, 1},
	{"dec", 8, 1},
	{"jmp", 9, 1},
	{"bne", 10, 1},
	{"red", 11, 1},
	{"prn", 12, 1},
	{"jsr", 13, 1},
	{"rts", 14, 0},
	{"stop", 15, 0}
};
char* instructions[NUM_OF_INSTRUCTIONS] = { ".data", ".string", ".entry", ".extern" }; /*arry of pointers to instructin types*/
char* registerNames[NUM_OF_REGISTERES] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7" }; /*arry of register names*/

/*----------------external variables------------------*/
int statement;
int numOfErrors;
ptr symbolHead;
ptr dataHead;
ptr codeHead;
list codeTable;
list dataTable;
list symbolTable;
queue* missingInfo;

/*------------Function declerations----------------------*/
void phraseLine(char line[MAX_LINE_LENGTH], int lineNum);
void sortInstruction(char* line, char* statement, char* lableName);
void sortMacro(char* line);
void sortOpcode(char* line, char* statement, char* lableName);
int statementType(char* statement);
boolean legalComma(char* words);
boolean legalLableMacro(char* name);
int findCommand(char* command);
boolean legalAmountOperands(int operands, int code);
int directOrConst(char* operand);
boolean legalOperands(int origin, int des, int code);
void updateInstructionAddress(ptr* head);
/*----------------------------------------------------------------------------------------------------------------------------*/

/*This is the main function for maneging the first pass. here the file is red line by line, and send to phrasing*/
void firstPass(FILE* fd)
{
	/*Initiate tables*/
	symbolHead = NULL, dataHead = NULL, codeHead = NULL;
	missingInfo = createQueue();
	numOfErrors = 0;
	ic = FIRST_ADDRESS, dc = 0;
	lineNum = 0;

	while (!feof(fd))
	{
		char line[MAX_LINE_LENGTH];

		/*insert first line in an arry*/
		if (fgets(line, MAX_LINE_LENGTH, fd) == NULL)
			break;

		/*start counting line number from 1*/
		lineNum++;
		phraseLine(line, lineNum);
	}

	reverseList(&symbolHead);
	reverseList(&dataHead);
	reverseList(&codeHead);

	updateInstructionAddress(&symbolHead);
}

/*------------------------Functions for checking line by statment type and sending data to relevent tables--------------------------*/

/*This function phrases each line red from file by seperating line into words (ignoring white spaces),
determing statement type and checking all syntax is legal. The line will be sent to a function for further proccessing according to statment type*/
void phraseLine(char line[MAX_LINE_LENGTH], int lineNum)
{
	char *statementName, *firstWord, *secondWord, *lableName;
	int i = 0;
	statement = -1;
	lableName = NULL;

	/*find statment and lable, ignoring white spaces*/
	firstWord = strtok(line, " \t");

	/*if empty or comment line - ignore*/
	if (*firstWord == ' ' || *firstWord == '\t' || *firstWord == '\n' || *firstWord == ';')
		return;

	/*if first word in sentence isn't a statement, save as lable name, second word will define statement type*/
	if (statementType(firstWord) == -1)
	{
		lableName = firstWord;
		secondWord = strtok(NULL, " \t");
	
		while ((lableName[i] != '\0') && (lableName[i] != '\n') && (lableName[i] != '\t') && (lableName[i] != ' '))
		{
			if (lableName[i + 1] == '\0')
			{
				if (lableName[i] != ':')
				{
					numOfErrors++;
					fprintf(stderr, "ERROR: Lable name must end with ':'.\tline: %d\n", lineNum);
					return;
				}

				else if (lableName[i] == ':')
				{
					lableName[i] = '\0';
					break;
				}
			}

			i++;
		}

		/*check if lable is legal*/
		if (legalLableMacro(lableName) == FALSE)
		{
			numOfErrors++;
			return;
		}

		i = 0;

		/*clean white spaces from statment, make sure no access text*/
		while (secondWord[i] != '\0')
		{

			if (secondWord[i] == ' ' || secondWord[i] == '\t' || secondWord[i] == '\n')
				secondWord[i] = '\0';

			else if (i != 0 && secondWord[i - 1] == '\0')
			{
				fprintf(stderr, "ERROR: Illegal statment name.\tline: %d\n", lineNum);
				numOfErrors++;
				return;
			}

			i++;
		}

		statement = statementType(secondWord);
		statementName = secondWord;

		/*if second word is ".define"*/
		if (statement == MACRO)
		{
			fprintf(stderr, "ERROR: Illegal macro definition.\tline: %d\n", lineNum);
			numOfErrors++;
			return;
		}
	}

	else
	{
		statement = statementType(firstWord);
		statementName = firstWord;
	}

	switch (statement)
	{
		case INSTUCTION: sortInstruction(line, statementName, lableName); return;
		case  OPCODE: sortOpcode(line, statementName, lableName); return;
		case MACRO: sortMacro(line); return;
		default: {
			numOfErrors++;
			fprintf(stderr, "ERROR: Unrecognized statement type.\tline: %d\n", lineNum);
			return;
			}
	}
}
 
/*function for dealing with instructions by instruction type: data, string, extern or entry*/
void sortInstruction(char* line, char* statement, char* lableName)
{
	symbolTableRow sr; dataTableRow dr;
	int i, type = -1;

	/*find instruction type*/
	for (i = 0; i < NUM_OF_INSTRUCTIONS; i++)
	{
		if (strcmp(statement, instructions[i]) == 0)
			type = i;
	}

	switch (type)
		{
		case DATA: {
			int number, numIndex = 0; char* nums[WORDS_IN_LINE];
			char* resOfLine = strtok(NULL, "\n\0");

			/*check legal commas*/
			if (!legalComma(resOfLine))
			{
				numOfErrors++;
				return;
			}

			/*clean commas*/
			nums[numIndex] = strtok(resOfLine, " \t,\n\0");
			while (nums[numIndex] != NULL)
			{
				numIndex++;
				nums[numIndex] = strtok(NULL," \t,\n\0");
			}

			numIndex = 0;

			/* if there is a lable add it to symbol table, address is current DC*/
			if (lableName != NULL)
			{
				/*add info to sybmol table*/
				strcpy(sr.name, lableName);
				sr.addressOrValue = dc;
				sr.type = INSTUCTION;
				sr.external = FALSE;
				
				add2Table(&symbolHead, &sr, SYMBOLT);
			}

			/*go over the line, number by number*/
			while (nums[numIndex] != NULL)
			{
				number = 0;
				/*check legal nums*/
				if (!legalNum(nums[numIndex]))
				{
					/*if its a macro, look for it in symbol table*/
					if (searchTable(symbolHead, nums[numIndex]) == FALSE)
					{
						fprintf(stderr, "ERROR: Illegal number or macro name %s.\tline: %d\n", nums[numIndex], lineNum);
						numOfErrors++;
						break;
					}
					else
						number = macroValue(symbolHead, nums[numIndex]);
				}

				/*add info to data table*/
				if (!number)
					number = atoi(nums[numIndex]);

				dr.DC = dc;
				dr.dataRow.value = number;
				add2Table(&dataHead, &dr, DATAT);

				numIndex++;
				dc++;
			}
			return;
		}

		case STRING: {

			char* string = strtok(NULL, "\0");
			int cr, k, i = 0, j = strlen(string) - 2; /*j is string legnth without end of string sign*/

			/* if there is a lable add it to symbol table, address is current DC*/
			if (lableName != NULL)
			{
				/*add info to sybmol table*/
				strcpy(sr.name, lableName);
				sr.addressOrValue = dc;
				sr.type = INSTUCTION;
				sr.external = FALSE;

				add2Table(&symbolHead, &sr, SYMBOLT);
			}

			/*check for legal string, starting and ending with " */
			while (((string[i] == ' ') || (string[i] == '\t')) && (string[i] != '\0'))
				i++;

			while (((string[j] == ' ') || (string[j] == '\t')) && j > i)
				j--;

			if (string[i] != '"' || string[j] != '"')
			{
				fprintf(stderr, "ERROR: Illegal string.\tline: %d\n", lineNum);
				numOfErrors++;
				return;
			}
			
			i++;

			/*add info to data table*/
			for (k = i; k < j; k++)
			{
				cr = string[k];

				dr.DC = dc;
				dr.dataRow.value = cr;
				add2Table(&dataHead, &dr, DATAT);

				dc++;
			}

			/*add end of string sign to data table*/
			cr = 0;
			dr.DC = dc;
			dr.dataRow.value = cr;
			add2Table(&dataHead, &dr, DATAT);
			dc++;

			return;
		}

		case ENTRY: { 
			char* name = strtok(NULL, "/0"); int i = 0;

			/*clean white spaces from entry name and make sure no access text*/
			while (name[i] != '\0')
			{
				if (name[i] == ' ' || name[i] == '\t' || name[i] == '\n')
					name[i] = '\0';

				else if (i != 0 && name[i - 1] == '\0')
				{
					fprintf(stderr, "ERROR: Access text in entry name.\tline: %d\n", lineNum);
					numOfErrors++;
					return;
				}
				i++;
			}

			/*add to queue - will need to come back to this line in second pass*/
			enQueue(missingInfo, lineNum, SAVE_ENTRY, name, NULL);
			
			return;
		} 

		case EXTERN: {
			char* name = strtok(NULL, "/0"); int i = 0;

			/*clean white spaces, make sure no access next*/
			while (name[i] != '\0')
			{
				if (name[i] == ' ' || name[i] == '\t' || name[i] == '\n')
					name[i] = '\0';

				else if (i != 0 && name[i - 1] == '\0')
				{
					fprintf(stderr, "ERROR: Access text in extern name.\tline: %d\n", lineNum);
					numOfErrors++;
					return;
				}

				i++;
			}

			/*check lable is legal*/
			if (!legalLableMacro(name))
			{
				numOfErrors++;
				return;
			}

			/*add to symbole table with value 0*/
			strcpy(sr.name, name);
			sr.addressOrValue = 0;
			sr.type = EXTERN;
			sr.external = TRUE;
			add2Table(&symbolHead, &sr, SYMBOLT);

			return;
		}
		
	}
}

/*function for dealing with macro statments: check legal name, check legal veriable, fill tables*/
void sortMacro(char* line)
{
	symbolTableRow sr;
	int value, i = 0;
	char* valueStr;

	/*find macro name and value*/
	char* macroName = strtok(NULL, "=");
	valueStr = strtok(NULL, "\n\0");

	/*cleam spaces. check for access text*/	
	while (macroName[i] != '\0')
	{
		if (macroName[i] == ' ' || macroName[i] == '\t' || macroName[i] == '\n')
			macroName[i] = '\0';

		else if (i != 0 && macroName[i - 1] == '\0')
		{
			fprintf(stderr, "ERROR: Access text in macro name.\tline: %d\n", lineNum);
			numOfErrors++;
			return;
		}

		i++;
	}

	/*make sure legal macro name*/
	if (legalLableMacro(macroName) == FALSE)
	{
		numOfErrors++;
		return; 
	}

	/*check value is a legal number*/
	if (!legalNum(valueStr))
	{
		fprintf(stderr, "ERROR: Illegal number.\tline: %d\n", lineNum);
		numOfErrors++;
		return;
	}
	value = atoi(valueStr);

	/*add info to sybmol table*/
	strcpy(sr.name, macroName);
	sr.addressOrValue = value;
	sr.type = MACRO;
	sr.external = FALSE;

	add2Table(&symbolHead, &sr, SYMBOLT);
}

/*function for dealing with opcode statments: find operation, chek right amount of paramater and increase DC*/
void sortOpcode(char* line, char* statement, char* lableName)
{
	symbolTableRow sr; codeTableRow cr;
	char* resOfLine = strtok(NULL, "\0");
	char* origin, * des;
	int numOfOperands = 0, memoryWords = 0, originType, desType, code, legalOp, i;

	/* if there is a lable add it to symbol table, address is current IC*/
	if (lableName != NULL)
	{
		/*add info to sybmol table*/
		strcpy(sr.name, lableName);
		sr.addressOrValue = ic;
		sr.type = OPCODE;
		sr.external = FALSE;

		add2Table(&symbolHead, &sr, SYMBOLT);
	}

	/*find opcode*/
	code = findCommand(statement);

	if (code == -1)
	{
		numOfErrors++;
		return;
	}

	/*make sure legal amount of commas (one or zero)*/
	if (!legalComma(resOfLine))
	{
		numOfErrors++;
		return;
	}

	/*clean commas*/
	origin = strtok(resOfLine, ",\n\0");
	if (resOfLine != NULL)
		des = strtok(NULL, "\n\0");

	/*find num of operands in statment*/
	if (origin != NULL)
	{
		if (des == NULL)
			numOfOperands = 1;
		else
			numOfOperands = 2;
	}

	/*check right amount of operands*/
	if (!legalAmountOperands(numOfOperands, code))
	{
		numOfErrors++;
		return;
	}

	/*find operand type*/
	originType = opType(origin);
	desType = opType(des);

	/*if name is not a legal operand*/
	if ((origin != NULL && originType == -1) || (des != NULL && desType == -1))
	{
		numOfErrors++;
		fprintf(stderr, "ERROR: Unrecognized operand.\tline: %d\n", lineNum);
		return;
	}

	/*check legal operand types for oparation*/
	legalOp = legalOperands(originType, desType, code);
	if (!legalOp)
	{
		fprintf(stderr, "ERROR: Illegal type of operand.\tline: %d\n", lineNum);
		numOfErrors++;
		return;
	}

	/*find amount of memory lines needed in code table*/
	if (originType == imidiate || originType == direct || originType == registers)
		memoryWords += 1;

	else if (originType == constent)
		memoryWords += 2;

	if (desType == imidiate || desType == direct)
			memoryWords += 1;

	if (desType == registers)
	{
		if (originType != registers)
			memoryWords += 1;
	}


	else if (desType == constent)
		memoryWords += 2;

	if (originType == -1)
		originType = 0;

	/*if theres only one addressing word store it in des*/
	if (desType == -1)
	{
		cr.codeRow.word.destination = originType;
		cr.codeRow.word.source = 0;
	}

	else
	{
		cr.codeRow.word.destination = desType;
		cr.codeRow.word.source = originType;
	}

	/*bild first word in binery*/
	cr.IC = ic;
	cr.codeRow.word.ARE = 0;
	cr.codeRow.word.opcode = code;
	cr.codeRow.word.unused = 0;

	add2Table(&codeHead, &cr, CODET);

	ic++;

	/*save information to queue - will need to continue proccessing in second pass*/
	if(memoryWords != 0)
		enQueue(missingInfo, lineNum, ic, origin, des);

	/*save space for rest of words*/
	for (i = 0; i < memoryWords; i++)
	{
		cr.IC = ic++;
	
		cr.codeRow.word.ARE = 0;
		cr.codeRow.word.destination = 0;
		cr.codeRow.word.source = 0;
		cr.codeRow.word.opcode = 0;
		cr.codeRow.word.unused = 0;
		add2Table(&codeHead, &cr, CODET);
	}

}

/*---------------------------------------------Utility functions for phrasing---------------------------------------------------*/

/*This function determins statement type: instruction, opcode or macro*/
int statementType(char* statement)
{
	int i;

	for (i = 0; i < NUM_OF_INSTRUCTIONS; i++)
		if (strcmp(statement, instructions[i]) == 0)
			return INSTUCTION;

	for (i = 0; i < NUM_OF_OPCODES; i++)
		if (strcmp(statement, opcodes[i].codeName) == 0)
			return OPCODE;

	if (strcmp(statement, MACRO_STATMENT) == 0)
		return MACRO;

	return -1;
}

/*This function checks all numbers providad are legal, positive or negative integers, may start with + or - sign*/
boolean legalNum(char* num)
{
	int i;
	num = strtok(num, " \t");

	if (!(num[0] == '+' || num[0] == '-' || isdigit(num[0])))
		return FALSE;

	for (i = 1; i < strlen(num); i++)
	{
		if(!isdigit(num[i]))
			return FALSE;
	}

	return TRUE;
}


/*This function returns TRUE if all commas in sentence are legal and FALSE otherwise*/
boolean legalComma(char* words)
{
	int i = 0, commas = 0, nums = 0, commaCnt = 0, numCnt = 0;


	if (words == NULL)
		return TRUE;

	/*count comas vs letters*/
	while ((words[i] != '\n') && (words[i] != '\0'))
	{
		if (words[i] == ',')
		{
			commas++; commaCnt++;
			
			/*if first word is a comma*/
			if (commaCnt > 0 && numCnt == 0)
			{
				fprintf(stderr, "ERROR: Too many commas.\tline: %d\n", lineNum);
		    		return FALSE;
			}

			nums = 0;
			i++;
		}

		else if ((words[i] != ',') && (words[i] != ' ') && (words[i] != '\t'))
		{
			commas = 0;
			nums++; numCnt++;
			while ((words[++i] != '\0') && (words[i] != '\n') && (words[i] != ',') && (words[i] != ' ') && (words[i] != '\t'));
		}

		else
			i++;

		/*two commas in a row*/
		if (commas == 2)
		{
			fprintf(stderr, "ERROR: Too many commas.\tline: %d\n", lineNum);
			return FALSE;
		}

		/*two words in a row*/
		if (nums == 2)
		{
			fprintf(stderr, "ERROR: Missing comma.\tline: %d\n", lineNum);
			return FALSE;
		}

	}

	/*last char in input is a comma*/
	if (commas > 0)
	{
		fprintf(stderr, "ERROR: Too many commas.\tline: %d\n", lineNum);
		return FALSE;
	}

	return TRUE;
}

/* This function checks a lable or macro name is legal*/
boolean legalLableMacro(char *name)
{
	int i = 0, nameLen = strlen(name);
	
	/*string is empty*/
	if (strcmp(name, "\0") == 0)
		return TRUE;

	/*a macro/ lable name must start with a letter*/
	if (!isalpha(name[i]))
	{
		fprintf(stderr,"ERROR: %s name must start with a letter.\tline: %d\n", KIND, lineNum);
		return FALSE;
	}

	i++;
	/*check legal legnth*/
	if (nameLen > LABLE_LENGTH)
	{
		fprintf(stderr, "ERROR: %s name too long.\tline: %d\n", KIND, lineNum);
		return FALSE;
	}
	/*check valid chars*/
	while ((name[i] != '\0') && (name[i] != '\n') && (name[i] != '\t') && (name[i] != ' '))
	{
		if (!isalnum(name[i]))
		{
			fprintf(stderr, "ERROR: %s name contains invalid character.\tline: %d\n", KIND, lineNum);
			return FALSE;
		}

		i++;
	}

	/*check if lable/macro name is a saved word*/
	for (i = 0; i < NUM_OF_OPCODES; i++)
	{
		if (strcmp(name, opcodes[i].codeName) == 0 ) 
		{
			fprintf(stderr, "ERROR: %s name can not be the same as an opcode name.\tline: %d\n", KIND, lineNum);
				return FALSE;
		}
	}

	for (i = 0; i < NUM_OF_REGISTERES; i++)
	{
		if (strcmp(name, registerNames[i]) == 0)
		{
			fprintf(stderr, "ERROR: %s name can not be the same as a register name.\tline: %d\n", KIND, lineNum);
			return FALSE;
		}
	}

	/*search symbol table, if macro/lable name already exists - print error and return false*/
	if (searchTable(symbolHead, name) == TRUE)
	{
		fprintf(stderr, "ERROR: %s %s already defined.\tline: %d\n", KIND, name ,lineNum);
		return FALSE;
	}
	
	return TRUE;
}


/*This function return a command name from opcod struct, or "no" if the string isnt an opcod*/
int findCommand(char* command)
{
	int i;
	
	for (i = 0; i < NUM_OF_OPCODES; i++)
	{
		if (strcmp(command, opcodes[i].codeName) == 0)
			return opcodes[i].codeSymble;
	}
	return -1;
}


/*This function returns true if amount of operands is accuret for opcode*/
boolean legalAmountOperands(int operands, int code)
{
	int i;

	for (i = 0; i < NUM_OF_OPCODES; i++)
	{
		if (code == opcodes[i].codeSymble)
		{
			if (operands != opcodes[i].NumOfOperands)
			{
				fprintf(stderr, "ERROR: Illegal amount of operands.\tline: %d\n", lineNum);
				return FALSE;
			}

			break;
		}
	}

	return TRUE;
}

/*This function return an operand type or -1 if illegal operand*/
int opType(char* name)
{
	int i = 0, type, opLegnth;
	char num[WORDS_IN_LINE], *tagName, *operand;
	char nameCpy[WORDS_IN_LINE];

	if (name == NULL)
		return -1;

	strcpy(nameCpy, name);
	operand = strtok(nameCpy, " \t");
	opLegnth = strlen(operand);

	/*if name starts with #, operand is imidiate or error*/
	if (operand[i] == '#')
	{
		/*clean white spaces*/
		for (i = 1; i < opLegnth; i++)
		{
			if (operand[i] != '\n' && operand[i] != '\t' && operand[i] != '\0')
				num[i - 1] = operand[i];

			else
				num[i - 1] = '\0';
		}

		for (i = opLegnth - 1; i < WORDS_IN_LINE; i++)
			num[i] = '\0';


		if (legalNum(num))
			return imidiate;

		else if (searchTable(symbolHead, num) == TRUE)
			return imidiate;

		else return NONE;
	}

	/*if not imidiate - check if register*/
	for (i = 0; i < NUM_OF_REGISTERES; i++)
	{
		if (strcmp(operand, registerNames[i]) == 0)
			return registers;
	}

	/*if not imidiate or register define as direct or const*/
	type = directOrConst(operand);
	if (type == NONE)
		return NONE;

	if (type == 0)
	{
		tagName = strtok(operand, "[");
	}

	else
		tagName = operand;

	/*check if legal tag name*/
	i = 0;

	if (!isalpha(tagName[i]) || (strlen(tagName) > LABLE_LENGTH))
		return NONE;

	while (tagName[i] != '\0')
	{
		if (!isalnum(tagName[i]))
			return -1;
		i++;
	}

	if (type == 0)
		return constent;

	return direct;
}

/*This function returns 0 if char is a constant operand and 1 if its a direct. if it's neither it returns -1*/
int directOrConst(char* operand)
{
	int i, k = 0, legnth = strlen(operand), j = legnth;
	char num[MAX_LINE_LENGTH];
	for (i = 0; i < legnth; i++)
	{
		if (operand[i] == '[')
		{
			i++;

			while ((operand[j] == ' ') || (operand[j] == '\t') || (operand[j] == '\0') || (operand[j] == '\n'))
				j--;
			/*if no closing braked found - error*/
			if (operand[j] != ']')
				return NONE;
			/*if brakets are empty - error*/
			if (i >= j)
				return NONE;
			/*find value in brakets*/
			while (i < j)
			{
				num[k] = operand[i];
				k++; i++;
			}
			num[k] = '\0';

			k = 0;
			/*check if legal macro or number*/
			if (searchTable(symbolHead, num) == TRUE)
			{
				return 0;
			}

			if (num[k] == '-' || num[k] == '+')
				k++;

			while (num[k] != '\0')
			{
				if (!isdigit(num[k]))
					return NONE;
				k++;
			}
			return 0;
		}
	}

	return 1;
}

/*This function returns TRUE if operand are legal type for origin and destantion of opcode*/
boolean legalOperands(int origin, int des, int code)
{
	switch (code) {
	case mov: case add: case sub: {
		if (des != -1 && origin != -1)
		{
			if (des != imidiate)
				return TRUE;
		}
	}

	case cmp: {
		if (des != -1 && origin != -1)
			return TRUE;
	}

	case not: case clr: case inc: case dec: case red: {
		if ((des == -1) && (origin != imidiate))
			return TRUE;
	}

	case lea: {
		if (((origin == direct) || (origin == constent)) && (des != imidiate))
			return TRUE;
	}

	case jmp: case bne: case jsr: {
		if ((des == -1) && ((origin == direct) || (origin == registers)))
			return TRUE;
	}
	case prn: {
		if ((des == -1) && (origin != -1))
			return TRUE;
	}

	case rts: case stop:
		if ((des == -1) && (origin == -1))
			return TRUE;
	}

	return FALSE;
}

/*This function uptades address value of instructions in symbol table at the end of first pass*/
void updateInstructionAddress(ptr* head)
{
	ptr temp = *head;

	while (temp)
	{
		if (((symbolTableRow*)(temp)->data)->type == INSTUCTION)
			((symbolTableRow*)(temp)->data)->addressOrValue += ic;

		temp = (temp)->next;
	}
}