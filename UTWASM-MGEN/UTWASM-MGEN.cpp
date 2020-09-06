// UT99 WebAssembly manifest.json generator
// Preps the game for new data.

#include "stdafx.h"
#include <stdlib.h>
#include <string.h>

#define DEF_FILELIST_NAME "tree_list.txt"
#define DEF_PATCHED_FILELIST_NAME "patched_tree_list.txt"
#define DEF_GAMEDATA_DIR_NAME "gamedata"

char ReadLine[1024];
char FinalOutName[1024];
char TempBuffer[1024];

unsigned int GetFileSize(const char* FileName)
{
	struct stat st;
	stat(FileName, &st);
	return st.st_size;
}

unsigned int CountLinesInFile(const char* FileName)
{
	FILE *finput = fopen(FileName, "r");

	if (!finput)
	{
		printf("ERROR: Can't open file %s for reading!\n", FileName);
		perror("ERROR");
		return -1;
	}

	
	unsigned int FileSize = GetFileSize(FileName);
	unsigned int LineCount = 0;
	char ReadCh;

	while (ftell(finput) < FileSize)
	{
		ReadCh = fgetc(finput);
		if (ReadCh == '\n')
			LineCount++;
	}

	fclose(finput);
	
	return LineCount + 1;
}

int ReplaceCharRecursively(char* input, char oldchar, char newchar)
{
	for (unsigned int i = 0; i <= strlen(input); i++)
	{
		if (input[i] == oldchar)
		{
			input[i] = newchar;
		}
	}
	return 0;
}

int GenerateFileList(const char* InDir, const char* OutList) // relies on GNU tree binary
{
	int ListCount = 0;

	FILE *f = fopen("tree.exe", "rb");
	if (!f)
	{
		printf("ERROR: Can't find tree.exe\n");
		perror("ERROR");
		return -1;
	}
	fclose(f);

	sprintf(TempBuffer, "tree --noreport -o \"%s\" -i -f \"%s\"", OutList, InDir);
	printf("Creating file list with: %s\n", TempBuffer);
	system(TempBuffer);

	printf("Patching file list %s...\n", OutList);

	ListCount = CountLinesInFile(OutList);

	f = fopen(OutList, "r");
	if (!f)
	{
		printf("ERROR: Can't open file %s for reading!\n", OutList);
		perror("ERROR");
		return -1;
	}

	strcpy(TempBuffer, "patched_");
	strcat(TempBuffer, OutList);
	FILE *flist = fopen(TempBuffer, "w");
	if (!flist)
	{
		printf("ERROR: Can't open file %s for writing!\n", TempBuffer);
		perror("ERROR");
		return -1;
	}

	fgets(ReadLine, 1024, f); // skipping the first line by using gets

	for (unsigned int i = 1; i < ListCount; i++)
	{
		fgets(ReadLine, 1024, f);
		if (ReadLine[strlen(ReadLine) - 1] == '\n')
			ReadLine[strlen(ReadLine) - 1] = 0;

		if (strrchr(ReadLine, '.'))
		{
			if (strcmp(ReadLine + strlen(InDir) + 1, "index.html") && strcmp(ReadLine + strlen(InDir) + 1, "manifest.json"))
			{
				fputs(ReadLine + strlen(InDir) + 1, flist);
				fputc('\n', flist);
			}
		}
	}

	fclose(flist);
	fclose(f);
	return 0;
}

int GenerateJSON(const char* InListFilename, const char* GameDataDir, const char* OutFilename)
{
	FILE *f = fopen(InListFilename, "r");
	if (!f)
	{
		printf("ERROR: Can't open file %s for reading!\n", InListFilename);
		perror("ERROR");
		return -1;
	}

	FILE *fjson = fopen(OutFilename, "w");
	if (!fjson)
	{
		printf("ERROR: Can't open file %s for writing!\n", OutFilename);
		perror("ERROR");
		return -1;
	}

	int FileCount = CountLinesInFile(InListFilename) - 1;

	printf("{\n");
	fprintf(fjson, "{\n");

	for (unsigned int i = 0; i < FileCount; i++)
	{
		fgets(ReadLine, 1024, f);
		if (ReadLine[strlen(ReadLine) - 1] == '\n')
			ReadLine[strlen(ReadLine) - 1] = 0;

		sprintf(TempBuffer, "%s\\%s", GameDataDir, ReadLine);
		ReplaceCharRecursively(TempBuffer, '/', '\\');

		printf("\t\"%s\" : { \"filesize\" : %d, \"filetime\" : 1493314151 }\n", ReadLine, GetFileSize(TempBuffer));
		fprintf(fjson, "\t\"%s\" : { \"filesize\" : %d, \"filetime\" : 1493314151 }", ReadLine, GetFileSize(TempBuffer));
		if (i == FileCount - 1)
		{
			printf("\n");
			fprintf(fjson, "\n");
		}
		else
		{
			printf(",\n");
			fprintf(fjson, ",\n");
		}
	}
	printf("}\n");
	fprintf(fjson, "}\n");

	fclose(fjson);
	fclose(f);

	return 0;
}

int main(int argc, char *argv[])
{
	printf("UT WASM Manifest JSON Generator\n\n");
	if (argc < 2)
	{
		printf("ERROR: Too few arguments.\nUSAGE: %s GameDataDir [OutJson.json]\n", argv[0]);
		return -1;
	}

	GenerateFileList(argv[1], DEF_FILELIST_NAME);

	if (argc == 2)
	{
		strcpy(FinalOutName, argv[1]);
		strcat(FinalOutName, "\\");
		strcat(FinalOutName, "manifest.json");
	}
	else
		strcpy(FinalOutName, argv[2]);

	GenerateJSON(DEF_PATCHED_FILELIST_NAME, argv[1], FinalOutName);
	//printf("%d files\n", CountLinesInFile(argv[1]));

    return 0;
}

