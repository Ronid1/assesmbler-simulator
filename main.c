/*This is the main file of the assembler. here files will be red and sent to first pass, if no errors were found, it will countinue
to the second pass. After proccessing is finished all utility tables will be deleted*/
#include "definitions.h"
#include "functions.h"

void cleanMemory();

int main(int argc, char* argv[])
{
	FILE* fd;
	int i;
	char fileFullName[FILE_NAME_LENGTH];

	/*no file provided*/
	if (argc < 2)
	{
		printf("\nERROR: NO file provided\n");
		return 1;
	}

	/*go over each file provided by user*/
	for (i = 1; i < argc; i++)
	{
		fileName = argv[i];
		/*add .as to end of file name provided*/
		strcpy(fileFullName, argv[i]);
		strcat(fileFullName, ".as");

		/*open file, if not foud, print error and move to next file*/
		if (!(fd = fopen(fileFullName, "r")))
		{
			fprintf(stderr, "ERROR: Can not find %s\n", fileFullName);
			continue;
		}
		
		numOfErrors = 0;

		firstPass(fd);

		/*If no errors found in first pass - continue to second pass*/
		if (numOfErrors == 0)
		{
			rewind(fd);
			secondPass(fd);
		}

			fclose(fd);
			cleanMemory();
	}
	return 0;
}

/*delete all utility linked lists and queue*/
void cleanMemory()
{
	deleteTable(&symbolHead);
	deleteTable(&codeHead);
	deleteTable(&dataHead);
	if((missingInfo->front) != NULL)
		deleteQueue(missingInfo);
}