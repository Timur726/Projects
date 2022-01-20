#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>

void exitFunc();
void infoFunc();
void sizeFunc(char* file);
void lsFunc();
void lsFunc2(char* dir);
void cdFunc(char* dir, bool close);
void creatFunc(char* file, bool mkdir);
void mvFunc(char* from, char* to);
void openFunc(char* file, char* mode);
void closeFunc(char* file);
void lseekFunc(char* file, long int offset);
void readFunc(char* file, long int size);
void writeFunc(char* file, long int size, char* string);
void rmFunc(char* file, bool dir);
void cpFunc(char* file, char* to);
void showopenFunc();

FILE* f;	//pointer to image to read from

int BPB_BytsPerSec;	 	//Bytes per Sector
int BPB_SecPerClus; 	//Sectors per Cluster
int BPB_RsvdSecCnt; 	//Reserved Sector Count
int BPB_NumFATs;			//Number of FATs
int BPB_TotSec32;			//Total Sectors
int BPB_FATSz32;			//FATsize
int BPB_RootClus;			//Root Cluster

int FirstFATSector;		//beginning of FAT region
int FirstDataSector;	//beginning of Data region

unsigned int CurrDirClus;			//first cluster (n) of current directory
unsigned int LastClus;					//last cluster of current directory

int findLastClus();			//find last cluster of current directory
void allocateNewClus();	//allocate new cluster for directory

int toInt(unsigned char* buf, int size);	//converts char string to hex, then to int
int hexToInt(unsigned char* d, int size);	//converts hex string to int

struct DIRENTRY {
	unsigned char DIR_Name[12];
	unsigned char DIR_Attributes[1];
	unsigned char DIR_FstClusHI[2];
	unsigned char DIR_FstClusLO[2];
	unsigned char DIR_FileSize[4];
} __attribute__((packed));

struct DIRENTRY directories[1024];	//stores all directories in current directory
int numDirectories;

struct OPENFILE {
	unsigned char Name[11];
	unsigned char FstClus[4];
	unsigned char Mode[3];
	unsigned char FileSize[4];
	long int Offset;
};

struct OPENFILE openfiles[1024];	//stores all currently open files

void initialize();	//initialize directories[]

unsigned int ClusterPath[1024];		//stores first cluster (n) of each directory in path
char DirPath[1024][12];		//stores name of each directory in path
int PathIndex;						//index of current path within ClusterPath[]

typedef struct {
	int size;
	char **items;
} tokenlist;

char *get_input(void);
tokenlist *get_tokens(char *input);
tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);

int main(int argc, char *argv[])
{
	int i = 0;
	unsigned char buf[64];
	f = fopen(argv[1], "r+");

	//Bytes per Sector
	fseek(f, 11, SEEK_SET);
	fread(buf, 1, 2, f);
	BPB_BytsPerSec = toInt(buf, 2);
	strcpy(buf, "");

	//Sectors per Cluster
	fseek(f, 13, SEEK_SET);
	fread(buf, 1, 1, f);
	BPB_SecPerClus = toInt(buf, 1);
	strcpy(buf, "");

	//Reserved Sector Count
	fseek(f, 14, SEEK_SET);
	fread(buf, 1, 2, f);
	BPB_RsvdSecCnt = toInt(buf, 2);
	strcpy(buf, "");

	//Number of FATs
	fseek(f, 16, SEEK_SET);
	fread(buf, 1, 1, f);
	BPB_NumFATs = toInt(buf, 1);
	strcpy(buf, "");

	//Total Sectors
	fseek(f, 32, SEEK_SET);
	fread(buf, 1, 4, f);
	BPB_TotSec32 = toInt(buf, 4);
	strcpy(buf, "");

	//FATsize
	fseek(f, 36, SEEK_SET);
	fread(buf, 1, 4, f);
	BPB_FATSz32 = toInt(buf, 4);
	strcpy(buf, "");

	//Root Cluster
	fseek(f, 44, SEEK_SET);
	fread(buf, 1, 4, f);
	BPB_RootClus = toInt(buf, 4);
	strcpy(buf, "");

	FirstFATSector = BPB_RsvdSecCnt;
	FirstDataSector = BPB_RsvdSecCnt + BPB_NumFATs * BPB_FATSz32;

	CurrDirClus = BPB_RootClus;
	LastClus = findLastClus();
	ClusterPath[0] = BPB_RootClus;
	strcpy(DirPath[0], "Root");
	PathIndex = 0;

	initialize();

	while (1) {
		//take input
		printf("~");
		if(PathIndex < 4)
		{
			for(i = 0; i < PathIndex; i++)
				printf("/%s", DirPath[i+1]);
		}
		else
		{
			printf("/...");
			for(i = PathIndex - 3; i < PathIndex; i++)
				printf("/%s", DirPath[i+1]);
		}
		printf("> ");
		char *input = get_input();
		tokenlist *tokens = get_tokens(input);

		//if statements for commands
		if(tokens->size == 1 && strcmp(tokens->items[0], "exit") == 0)
		{
			exitFunc();
			free(input);
			free_tokens(tokens);
			break;
		}
		else if(tokens->size == 1 && strcmp(tokens->items[0], "info") == 0)
			infoFunc();
		else if(tokens->size == 2 && strcmp(tokens->items[0], "size") == 0)
			sizeFunc(tokens->items[1]);
		else if(tokens->size == 1 && strcmp(tokens->items[0], "ls") == 0)
			lsFunc();
		else if(tokens->size == 2 && strcmp(tokens->items[0], "ls") == 0)
			lsFunc2(tokens->items[1]);
		else if(tokens->size == 2 && strcmp(tokens->items[0], "cd") == 0)
			cdFunc(tokens->items[1], true);
		else if(tokens->size == 2 && strcmp(tokens->items[0], "creat") == 0)
			creatFunc(tokens->items[1], false);
		else if(tokens->size == 2 && strcmp(tokens->items[0], "mkdir") == 0)
			creatFunc(tokens->items[1], true);
		else if(tokens->size == 3 && strcmp(tokens->items[0], "mv") == 0)
			mvFunc(tokens->items[1], tokens->items[2]);
		else if(tokens->size == 3 && strcmp(tokens->items[0], "open") == 0)
			openFunc(tokens->items[1], tokens->items[2]);
		else if(tokens->size == 2 && strcmp(tokens->items[0], "close") == 0)
			closeFunc(tokens->items[1]);
		else if(tokens->size == 3 && strcmp(tokens->items[0], "lseek") == 0)
			lseekFunc(tokens->items[1], atoi(tokens->items[2]));
		else if(tokens->size == 3 && strcmp(tokens->items[0], "read") == 0)
			readFunc(tokens->items[1], atoi(tokens->items[2]));
		else if(tokens->size == 4 && strcmp(tokens->items[0], "write") == 0)
			writeFunc(tokens->items[1], atoi(tokens->items[2]), tokens->items[3]);
		else if(tokens->size == 2 && strcmp(tokens->items[0], "rm") == 0)
			rmFunc(tokens->items[1], false);
		else if(tokens->size == 3 && strcmp(tokens->items[0], "cp") == 0)
			cpFunc(tokens->items[1], tokens->items[2]);
		else if(tokens->size == 2 && strcmp(tokens->items[0], "rmdir") == 0)
			rmFunc(tokens->items[1], true);
		else if(tokens->size == 1 && strcmp(tokens->items[0], "showopen") == 0)
			showopenFunc();
		else if(tokens->size != 0)
			printf("Error: Invalid command.\n");

		free(input);
		free_tokens(tokens);
	}

	return 0;
}

void exitFunc()
{
	close(f);
	free(f);
	return;
}

void infoFunc()
{
	printf("Bytes per Sector: %d\n", BPB_BytsPerSec);
	printf("Sectors per Cluster: %d\n", BPB_SecPerClus);
	printf("Reserved Sector Count: %d\n", BPB_RsvdSecCnt);
	printf("Number of FATS: %d\n", BPB_NumFATs);
	printf("Total Sectors: %d\n", BPB_TotSec32);
	printf("FATsize: %d\n", BPB_FATSz32);
	printf("Root Cluster: %d\n", BPB_RootClus);
	return;
}

void sizeFunc(char* file)
{
	int i = 0;
	for(i = 0; i < numDirectories; i++)
	{
		if(strcmp(directories[i].DIR_Name, file) == 0)
		{
			printf("%d\n", toInt(directories[i].DIR_FileSize, 4));
			return;
		}
	}
	printf("Error: File not found.\n");
	return;
}

void lsFunc()
{
	int i = 0;
	for(i = 0; i < numDirectories; i++)
	{
		if(directories[i].DIR_Attributes[0] == 2)	//hidden directory
			continue;
		if(directories[i].DIR_Attributes[0] != (0x01 | 0x02 | 0x04 | 0x08))
			printf("%s\n", directories[i].DIR_Name);
	}
	return;
}

void lsFunc2(char* dir)
{
	if(strcmp(dir, ".") == 0)
	{
		lsFunc();
		return;
	}

	bool found = false;
	char currentDir[12];
	int i = 0;
	for(i = 0; i < numDirectories; i++)
	{
		if(strcmp(directories[i].DIR_Name, dir) == 0)
		{
			if(directories[i].DIR_Attributes[0] != (char)0x10)
			{
				printf("Error: Not a directory.\n");
				return;
			}
			found = true;
			break;
		}
	}
	if(!found)
	{
		printf("Error: Directory not found.\n");
		return;
	}

	strcpy(currentDir, DirPath[PathIndex]);

	//go to directory, run ls, return to this directory
	cdFunc(dir, false);
	lsFunc();
	if(strcmp(dir, "..") != 0)
		cdFunc("..", false);
	else
		cdFunc(currentDir, false);
	return;
}

void cdFunc(char* dir, bool close)
{
	if(strcmp(dir, ".") == 0)
		return;

	unsigned char buf[11], FstClus[4];
	int i = 0;
	bool found = false;

	if(strcmp(dir, "..") == 0)
	{
		if(PathIndex == 0)
		{
			printf("Error: Already in root directory.\n");
			return;
		}
		found = true;
		ClusterPath[PathIndex] = 0;
		strcpy(DirPath[PathIndex], "");
		PathIndex--;
		CurrDirClus = ClusterPath[PathIndex];
		LastClus = findLastClus();
	}
	else
	{
		for(i = 0; i < numDirectories; i++)
		{
			if(strcmp(directories[i].DIR_Name, dir) == 0)
			{
				if(directories[i].DIR_Attributes[0] != 0x10)
				{
					printf("Error: Not a directory.\n");
					return;
				}
				found = true;
				PathIndex++;
				FstClus[0] = directories[i].DIR_FstClusLO[0];
				FstClus[1] = directories[i].DIR_FstClusLO[1];
				FstClus[2] = directories[i].DIR_FstClusHI[0];
				FstClus[3] = directories[i].DIR_FstClusHI[1];
				ClusterPath[PathIndex] = toInt(FstClus, 4);
				strcpy(DirPath[PathIndex], dir);
				CurrDirClus = ClusterPath[PathIndex];
				LastClus = findLastClus();
				break;
			}
		}
	}
	if(!found)
	{
		printf("Error: Directory not found.\n");
		return;
	}

	//close all open files
	if(close)
	{
		for(i = 0; i < 1024; i++)
		{
			if(strcmp(openfiles[i].Name, "") != 0)
				closeFunc(openfiles[i].Name);
		}
	}

	initialize();
	return;
}

void creatFunc(char* file, bool mkdir)
{
	int i = 0;
	unsigned char write[5], buf[11];
	unsigned long int Offset;

	//check if file already exists
	for(i = 0; i < numDirectories; i++)
	{
		if(strcmp(directories[i].DIR_Name, file) == 0)
		{
			if(!mkdir)
				printf("Error: File already exists.\n");
			else
				printf("Error: Directory already exists.\n");
			return;
		}
	}

	strcpy(directories[numDirectories].DIR_Name, file);
	for(i = 0; i < 4; i++)
		directories[numDirectories].DIR_FileSize[i] = (char)hexToInt("0", 1);

	if(!mkdir)
	{
		//create file
		for(i = 0; i < 2; i++)
			directories[numDirectories].DIR_FstClusLO[i] = (char)hexToInt("0", 1);
		for(i = 0; i < 2; i++)
			directories[numDirectories].DIR_FstClusHI[i] = (char)hexToInt("0", 1);
		directories[numDirectories].DIR_Attributes[0] = (char)hexToInt("20", 2);
	}
	else
	{
		//create directory and allocate cluster for it
		unsigned long int newDirClus = BPB_RootClus;
		do{
			newDirClus++;
			fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + newDirClus * 4, SEEK_SET);
			fread(buf, 1, 4, f);
		}while(toInt(buf, 4) != 0x0);

		//convert newDirClus to FstClusLO and FstClusHI
		unsigned char FstClus[8], FstTemp[2];
		sprintf(FstClus, "%08x", newDirClus);

		for(i = 3; i >= 0; i--)
		{
			FstTemp[0] = FstClus[i*2];
			FstTemp[1] = FstClus[i*2+1];
			if(i > 1)
				directories[numDirectories].DIR_FstClusLO[3-i] = (char)hexToInt(FstTemp, 2);
			else
				directories[numDirectories].DIR_FstClusHI[1-i] = (char)hexToInt(FstTemp, 2);
		}

		//point new directory cluster to empty
		for(i = 0; i < 4; i++)
			write[i] = 255;
		write[4] = '\0';
		fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + newDirClus * 4, SEEK_SET);
		fwrite(write, 1, 4, f);

		//set attributes
		directories[numDirectories].DIR_Attributes[0] = (char)hexToInt("10", 2);
	}
	numDirectories++;

	//if new cluster needs to be allocated (writing to FAT region)
	if(numDirectories%16 == 1 && numDirectories != 1)
		allocateNewClus();

	Offset = (FirstDataSector-BPB_RootClus+LastClus)*BPB_BytsPerSec*BPB_SecPerClus+
					 (32*((numDirectories-1)%(BPB_BytsPerSec*BPB_SecPerClus/32)));

	//write to data region
	fseek(f, Offset, SEEK_SET);
	fwrite(directories[numDirectories-1].DIR_Name, 1, 11, f);
	fseek(f, Offset + 11, SEEK_SET);
	fwrite(directories[numDirectories-1].DIR_Attributes, 1, 1, f);
	fseek(f, Offset + 20, SEEK_SET);
	fwrite(directories[numDirectories-1].DIR_FstClusHI, 1, 2, f);
	fseek(f, Offset + 26, SEEK_SET);
	fwrite(directories[numDirectories-1].DIR_FstClusLO, 1, 2, f);
	fseek(f, Offset + 28, SEEK_SET);
	fwrite(directories[numDirectories-1].DIR_FileSize, 1, 4, f);

	if(mkdir && strcmp(file, ".") != 0 && strcmp(file, "..") != 0)
	{
		cdFunc(file, false);
		creatFunc(".", true);
		creatFunc("..", true);
		cdFunc("..", false);
	}

	return;
}

void mvFunc(char* from, char* to)
{
	if((strcmp(from, "..") == 0 || strcmp(to, "..") == 0) && PathIndex == 0)
	{
		printf("Error: No previous directory before root directory.\n");
		return;
	}
	if(strcmp(from, ".") == 0 && strcmp(to, "..") == 0)
		return;
	if(strcmp(from, to) == 0)
	{
		printf("Error: Cannot move a directory into itself.\n");
		return;
	}
	if(strcmp(from, ".") == 0)
	{
		printf("Error: Cannot move this directory into a directory within itself.\n");
		return;
	}
	if(strcmp(from, "..") == 0)
	{
		printf("Error: Cannot move previous directory into a directory within itself.\n");
		return;
	}

	int i = 0, from_index, to_index;
	bool renaming = true, exists = false;

	//check if FROM exists
	for(i = 0; i < numDirectories; i++)
	{
		if(strcmp(directories[i].DIR_Name, from) == 0)
		{
			exists = true;
			from_index = i;
			break;
		}
	}
	if(!exists)
	{
		printf("Error: File does not exist.\n");
		return;
	}

	if(strcmp(to, ".") == 0)
		return;

	//check if TO exists (if not, then FROM will be renamed to TO)
	for(i = 0; i < numDirectories; i++)
	{
		if(strcmp(directories[i].DIR_Name, to) == 0)
		{
			if(directories[i].DIR_Attributes[0] != (char)0x10 &&
				 directories[from_index].DIR_Attributes[0] != (char)0x10)
			{
				//FROM is a file and TO is an existing file
				printf("Error: The name is already being used by another file.\n");
				return;
			}
			if(directories[i].DIR_Attributes[0] != (char)0x10 &&
				 directories[from_index].DIR_Attributes[0] == (char)0x10)
			{
				//FROM is a directory and TO is an existing file
				printf("Cannot move directory: Invalid destination argument.\n");
				return;
			}
			renaming = false;
			to_index = i;
			break;
		}
	}

	//if moving to or from this directory or previous directory
	if(strcmp(to, "..") == 0)
		renaming = false;

	unsigned char buf[64], buf2[64];

	fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + CurrDirClus * 4, SEEK_SET);

	//find location in data sector
	for(i = 0; i < from_index/(BPB_BytsPerSec*BPB_SecPerClus/32); i++)
	{
		fread(buf2, 1, 4, f);
		fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + toInt(buf2, 4) * 4, SEEK_SET);
	}

	unsigned char dir[12];
	strcpy(dir, DirPath[PathIndex]);

	//check that there isn't already a file in that directory with the same name
	if(!renaming)
	{
		unsigned char dirname[11];
		strcpy(dirname, directories[from_index].DIR_Name);
		cdFunc(to, false);
		for(i = 0; i < numDirectories; i++)
		{
			if(strcmp(directories[i].DIR_Name, dirname) == 0)
			{
				printf("Error: File with this name already exists in that directory.\n");
				if(strcmp(to, "..") != 0)
					cdFunc("..", false);
				else
					cdFunc(dir, false);
				return;
			}
		}
		if(strcmp(to, "..") != 0)
			cdFunc("..", false);
		else
			cdFunc(dir, false);
	}

	if(from_index/(BPB_BytsPerSec*BPB_SecPerClus/32) > 0)
		fseek(f, (FirstDataSector-BPB_RootClus+toInt(buf2, 4))*BPB_BytsPerSec*BPB_SecPerClus
					+(32*((from_index)%(BPB_BytsPerSec*BPB_SecPerClus/32))), SEEK_SET);
	else
		fseek(f, (FirstDataSector-BPB_RootClus+CurrDirClus)*BPB_BytsPerSec*BPB_SecPerClus
					+(32*((from_index)%(BPB_BytsPerSec*BPB_SecPerClus/32))), SEEK_SET);

	if(renaming)	//rename FROM to TO
	{
		strcpy(directories[from_index].DIR_Name, to);
		fwrite(to, 1, 11, f);
		fread(buf, 1, 11, f);	//for some reason it doesn't work without this line
	}
	else	//move FROM to TO
	{
		fread(buf, 1, 32, f);

		//deleting from this directory
		unsigned char temp[32];
		temp[0] = (char)0xE5;
		for(i = 1; i < 32; i++)
			temp[i] = (char)0x0;
		temp[11] = (char)0x02;

		//replace with blank
		if(from_index/(BPB_BytsPerSec*BPB_SecPerClus/32) > 0)
			fseek(f, (FirstDataSector-BPB_RootClus+toInt(buf2, 4))*BPB_BytsPerSec*BPB_SecPerClus
						+(32*((from_index)%(BPB_BytsPerSec*BPB_SecPerClus/32))), SEEK_SET);
		else
			fseek(f, (FirstDataSector-BPB_RootClus+CurrDirClus)*BPB_BytsPerSec*BPB_SecPerClus
						+(32*((from_index)%(BPB_BytsPerSec*BPB_SecPerClus/32))), SEEK_SET);
		fwrite(temp, 1, 32, f);

		cdFunc(to, false);

		if(numDirectories%(BPB_BytsPerSec*BPB_SecPerClus/32) == 0 && numDirectories != 0)
			allocateNewClus();

		//writing to directory
		fseek(f, (FirstDataSector-BPB_RootClus+LastClus)*BPB_BytsPerSec*BPB_SecPerClus
					+(32*(numDirectories%(BPB_BytsPerSec*BPB_SecPerClus/32))), SEEK_SET);
		fwrite(buf, 1, 32, f);

		if(strcmp(to, "..") != 0)
			cdFunc("..", false);
		else
			cdFunc(dir, false);
	}

	return;
}

void openFunc(char* file, char* mode)
{
	//check that mode is valid
	if(strcmp(mode, "r") != 0 && strcmp(mode, "w") != 0 &&
		 strcmp(mode, "rw") != 0 && strcmp(mode, "wr") != 0)
	{
		printf("Error: Invalid mode (must be \"r\", \"w\", \"rw\", or \"wr\").\n");
		return;
	}

	//check that file exists in current directory
	bool exists = false;
	int i = 0, j = 0;
	for(i = 0; i < numDirectories; i++)
	{
		if(strcmp(directories[i].DIR_Name, file) == 0)
		{
			//check that file is not a directory
			if(directories[i].DIR_Attributes[0] == (char)0x10)
			{
				printf("Error: Cannot open a directory.\n");
				return;
			}
			//check that file is not read-only (if writing to file)
			if(directories[i].DIR_Attributes[0] == (char)0x01 && strcmp(mode, "r") != 0)
			{
				printf("Error: Cannot write to read-only file.\n");
				return;
			}
			exists = true;
			break;
		}
	}
	if(!exists)
	{
		printf("Error: File not found in current directory.\n");
		return;
	}

	//check that file is not already in table
	for(j = 0; j < 1024; j++)
	{
		if(strcmp(openfiles[j].Name, file) == 0)
		{
			printf("Error: File is already open.\n");
			return;
		}
	}

	unsigned char FstClus[4];
	FstClus[0] = directories[i].DIR_FstClusLO[0];
	FstClus[1] = directories[i].DIR_FstClusLO[1];
	FstClus[2] = directories[i].DIR_FstClusHI[0];
	FstClus[3] = directories[i].DIR_FstClusHI[1];

	//find first empty spot in table, and add file to it
	for(j = 0; j < 1024; j++)
	{
		if(strcmp(openfiles[j].Name, "") == 0)
		{
			strcpy(openfiles[j].Name, file);
			strcpy(openfiles[j].FstClus, FstClus);
			strcpy(openfiles[j].Mode, mode);
			strcpy(openfiles[j].FileSize, directories[i].DIR_FileSize);
			openfiles[j].Offset = 0;
			break;
		}
	}

	return;
}

void closeFunc(char* file)
{
	//check that file exists in table
	int i = 0;
	for(i = 0; i < 1024; i++)
	{
		if(strcmp(openfiles[i].Name, file) == 0)
		{
			strcpy(openfiles[i].Name, "");
			strcpy(openfiles[i].FstClus, "");
			strcpy(openfiles[i].Mode, "");
			openfiles[i].Mode[2] = '\0';
			strcpy(openfiles[i].FileSize, "");
			openfiles[i].Offset = 0;
			return;
		}
	}
	printf("Error: File not found in the list of opened files.\n");
	return;
}

void lseekFunc(char* file, long int offset)
{
	//check that file exists in table
	int i = 0;
	for(i = 0; i < 1024; i++)
	{
		if(strcmp(openfiles[i].Name, file) == 0)
		{
			if(offset < 0)
				printf("Error: Offset must be at least 0.\n");
			else if(offset > toInt(openfiles[i].FileSize, 4))
				printf("Error: Offset cannot be larger than file size.\n");
			else
				openfiles[i].Offset = offset;
			return;
		}
	}
	printf("Error: File not found in the list of opened files.\n");
	return;
}

void readFunc(char* file, long int size)
{
	int i = 0;
	bool found = false;

	//check that file is open
	for(i = 0; i < 1024; i++)
	{
		if(strcmp(openfiles[i].Name, file) == 0)
		{
			//check that file is open for reading
			if(strcmp(openfiles[i].Mode, "rw") != 0 && strcmp(openfiles[i].Mode, "wr") != 0 &&
				 strcmp(openfiles[i].Mode, "r") != 0)
			{
				printf("Error: File is not open for reading.\n");
				return;
			}
			found = true;
			break;
		}
	}
	if(!found)
	{
		printf("Error: File is not open.\n");
		return;
	}

	if(size > toInt(openfiles[i].FileSize, 4))
		size = toInt(openfiles[i].FileSize, 4);

	int CurrClus = toInt(openfiles[i].FstClus, 4);
	long int Offset = openfiles[i].Offset;
	unsigned char buf[BPB_BytsPerSec*BPB_SecPerClus + 1];

	//find first cluster to read from
	while(Offset > BPB_BytsPerSec*BPB_SecPerClus)
	{
		fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + CurrClus * 4, SEEK_SET);
		fread(buf, 1, 4, f);
		CurrClus = toInt(buf, 4);
		Offset -= BPB_BytsPerSec*BPB_SecPerClus;
	}

	//read from the rest of the clusters
	while(true)
	{
		fseek(f, (FirstDataSector-BPB_RootClus+CurrClus)*BPB_BytsPerSec*BPB_SecPerClus+Offset, SEEK_SET);

		//if this is the last cluster we're reading from
		if(size <= BPB_BytsPerSec*BPB_SecPerClus - Offset)
		{
			fread(buf, 1, size, f);
			buf[size] = '\0';
			printf("%s", buf);
			return;
		}

		//if we are going to need to read from more clusters
		fread(buf, 1, BPB_BytsPerSec*BPB_SecPerClus - Offset, f);
		buf[BPB_BytsPerSec*BPB_SecPerClus] = '\0';
		printf("%s", buf);
		size -= BPB_BytsPerSec*BPB_SecPerClus - Offset;

		Offset = 0;

		fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + CurrClus * 4, SEEK_SET);
		fread(buf, 1, 4, f);
		CurrClus = toInt(buf, 4);
	}
}

void writeFunc(char* file, long int size, char* string)
{
	int i = 0, j = 0, k = 0, index = 0;
	bool found = false;

	//check that file is in current directory
	for(index = 0; index < numDirectories; index++)
	{
		if(strcmp(directories[index].DIR_Name, file) == 0)
		{
			found = true;
			break;
		}
	}
	if(!found)
	{
		printf("Error: File is not in this directory.\n");
		return;
	}
	found = false;

	//check that file is open
	for(i = 0; i < 1024; i++)
	{
		if(strcmp(openfiles[i].Name, file) == 0)
		{
			//check that file is open for writing
			if(strcmp(openfiles[i].Mode, "rw") != 0 && strcmp(openfiles[i].Mode, "wr") != 0 &&
				 strcmp(openfiles[i].Mode, "w") != 0)
			{
				printf("Error: File is not open for writing.\n");
				return;
			}
			found = true;
			break;
		}
	}
	if(!found)
	{
		printf("Error: File is not open.\n");
		return;
	}

	int CurrClus = toInt(openfiles[i].FstClus, 4);
	long int Offset = openfiles[i].Offset;
	unsigned char string2[size], buf[BPB_BytsPerSec*BPB_SecPerClus + 1], temp[1], hex[2];

	//fill up string2 with string (up to size)
	if(size > strlen(string))
		size = strlen(string);
	for(j = 0; j < size; j++)
		string2[j] = string[j];

	//for updating image file later
	unsigned long int writeToClus = CurrDirClus;
	fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + CurrDirClus * 4, SEEK_SET);
	for(k = 0; k < index/(BPB_BytsPerSec*BPB_SecPerClus/32); k++)
	{
		fread(buf, 1, 4, f);
		writeToClus = toInt(buf, 4);
		fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + toInt(buf, 4) * 4, SEEK_SET);
	}

	//if we need to add new clusters
	int current_clusters = toInt(openfiles[i].FileSize, 4)/(BPB_BytsPerSec*BPB_SecPerClus);
	if(toInt(openfiles[i].FileSize, 4)%(BPB_BytsPerSec*BPB_SecPerClus) > 0)
		current_clusters++;
	int final_clusters = (Offset+size)/(BPB_BytsPerSec*BPB_SecPerClus);
	if((Offset+size)%(BPB_BytsPerSec*BPB_SecPerClus) > 0)
		final_clusters++;
	unsigned long int prevClus = toInt(openfiles[i].FstClus, 4);
	for(j = 0; j < final_clusters - current_clusters; j++)
	{
		//allocate cluster
		unsigned long int newDirClus = BPB_RootClus;
		do{
			newDirClus++;
			fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + newDirClus * 4, SEEK_SET);
			fread(buf, 1, 4, f);
		}while(toInt(buf, 4) != 0x0);

		//convert newDirClus to FstClusLO and FstClusHI
		unsigned char FstClus[8], FstTemp[2];
		sprintf(FstClus, "%08x", newDirClus);
		if(j == 0)
		{
			unsigned char FstClus2[4];
			for(k = 3; k >= 0; k--)
			{
				FstTemp[0] = FstClus[k*2];
				FstTemp[1] = FstClus[k*2+1];
				if(k > 1)
					directories[index].DIR_FstClusLO[3-k] = (char)hexToInt(FstTemp, 2);
				else
					directories[index].DIR_FstClusHI[1-k] = (char)hexToInt(FstTemp, 2);
			}

			//update open file table
			FstClus2[0] = directories[index].DIR_FstClusLO[0];
			FstClus2[1] = directories[index].DIR_FstClusLO[1];
			FstClus2[2] = directories[index].DIR_FstClusHI[0];
			FstClus2[3] = directories[index].DIR_FstClusHI[1];
			strcpy(openfiles[i].FstClus, FstClus2);
			CurrClus = newDirClus;

			//update image file
			fseek(f, (FirstDataSector-BPB_RootClus+writeToClus)*BPB_BytsPerSec*BPB_SecPerClus +
						(32*(index%(BPB_BytsPerSec*BPB_SecPerClus/32))) + 20, SEEK_SET);
			fwrite(directories[index].DIR_FstClusHI, 1, 2, f);
			fseek(f, (FirstDataSector-BPB_RootClus+writeToClus)*BPB_BytsPerSec*BPB_SecPerClus +
						(32*(index%(BPB_BytsPerSec*BPB_SecPerClus/32))) + 26, SEEK_SET);
			fwrite(directories[index].DIR_FstClusLO, 1, 2, f);
		}

		//point new cluster to empty
		unsigned char write[5];
		for(k = 0; k < 4; k++)
			write[k] = 255;
		write[4] = '\0';
		fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + newDirClus * 4, SEEK_SET);
		fwrite(write, 1, 4, f);

		//point previous cluster to new cluster
		if(prevClus != 0)
		{
			for(k = 0; k < 8; k++)
			{
				if(FstClus[k] == '\0')
					break;
			}
			int l = k;

			//move leading 0s
			if(l != 8)
			{
				for(; k < 8; k++)
					FstClus[k] = '0';
				FstClus[k] = '\0';

				unsigned char temp[9];

				for(k = 0; k < l; k++)
					temp[8-l+k] = FstClus[k];
				for(k = 0; k < 8 - l; k++)
					temp[k] = '0';
				temp[9] = '\0';
				for(k = 0; k < 9; k++)
					FstClus[k] = temp[k];
			}

			//convert to little endian
			unsigned char temp2[2];
			for(k = 3; k >= 0; k--)
			{
				temp2[0] = FstClus[k*2];
				temp2[1] = FstClus[k*2+1];
				write[3-k] = (char)hexToInt(temp2, 2);
			}
			write[4] = '\0';

			fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + prevClus * 4, SEEK_SET);
			fwrite(write, 1, 4, f);
		}
		prevClus = newDirClus;
	}

	//update file size
	if(Offset + size > toInt(openfiles[i].FileSize, 4))
	{
		unsigned char hex2[8], hextemp[2];
		sprintf(hex2, "%08x", Offset + size);
		for(k = 0; k < 4; k++)
		{
			hextemp[0] = hex2[k*2];
			hextemp[1] = hex2[k*2+1];
			directories[index].DIR_FileSize[3-k] = (char)hexToInt(hextemp, 2);
		}
		fseek(f, (FirstDataSector-BPB_RootClus+writeToClus)*BPB_BytsPerSec*BPB_SecPerClus +
					(32*(index%(BPB_BytsPerSec*BPB_SecPerClus/32))) + 28, SEEK_SET);
		fwrite(directories[index].DIR_FileSize, 1, 4, f);
		strcpy(openfiles[i].FileSize, directories[index].DIR_FileSize);
	}

	//find first cluster to write to
	while(Offset > BPB_BytsPerSec*BPB_SecPerClus)
	{
		fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + CurrClus * 4, SEEK_SET);
		fread(buf, 1, 4, f);
		CurrClus = toInt(buf, 4);
		Offset -= BPB_BytsPerSec*BPB_SecPerClus;
	}

	index = 0;

	//write to the rest of the clusters
	while(true)
	{
		fseek(f, (FirstDataSector-BPB_RootClus+CurrClus)*BPB_BytsPerSec*BPB_SecPerClus+Offset, SEEK_SET);

		//if this is the last cluster we're writing to
		if(size <= BPB_BytsPerSec*BPB_SecPerClus - Offset)
		{
			for(i = 0; i < size; i++)
			{
				fseek(f, (FirstDataSector-BPB_RootClus+CurrClus)*BPB_BytsPerSec*BPB_SecPerClus+Offset+i, SEEK_SET);
				temp[0] = string2[index];
				index++;
				sprintf(hex, "%02x", temp[0]);
				temp[0] = (char)hexToInt(hex, 2);
				fwrite(temp, 1, 1, f);
			}
			return;
		}

		//if we are going to need to write to more clusters
		for(i = 0; i < BPB_BytsPerSec*BPB_SecPerClus - Offset; i++)
		{
			fseek(f, (FirstDataSector-BPB_RootClus+CurrClus)*BPB_BytsPerSec*BPB_SecPerClus+Offset+i, SEEK_SET);
			temp[0] = string2[index];
			index++;
			sprintf(hex, "%02x", temp[0]);
			temp[0] = (char)hexToInt(hex, 2);
			fwrite(temp, 1, 1, f);
		}
		size -= BPB_BytsPerSec*BPB_SecPerClus - Offset;

		Offset = 0;

		fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + CurrClus * 4, SEEK_SET);
		fread(buf, 1, 4, f);
		CurrClus = toInt(buf, 4);
	}
}

void rmFunc(char* file, bool rmdir)
{
	int i = 0, index = 0;
	bool exists = false;

	if(strcmp(file, "..") == 0 && CurrDirClus != BPB_RootClus)
	{
		if(!rmdir)
			printf("Error: Cannot use rm on a directory. Use rmdir instead.\n");
		else
			printf("Error: Cannot perform rmdir on .. directory.\n");
		return;
	}
	if(strcmp(file, ".") == 0)
	{
		if(!rmdir)
			printf("Error: Cannot use rm on a directory. Use rmdir instead.\n");
		else
			printf("Error: Cannot perform rmdir on . directory.\n");
		return;
	}

	//check that file exists in this directory
	for(i = 0; i < numDirectories; i++)
	{
		if(strcmp(directories[i].DIR_Name, file) == 0)
		{
			if(directories[i].DIR_Attributes[0] == (char)0x10 && !rmdir)
			{
				printf("Error: Cannot use rm on a directory. Use rmdir instead.\n");
				return;
			}
			if(rmdir)
			{
				if(directories[i].DIR_Attributes[0] == (char)0x20)
				{
					printf("Error: Cannot use rmdir on a file. Use rm instead.\n");
					return;
				}
			}
			exists = true;
			index = i;
			break;
		}
	}

	if(!exists)
	{
		if(!rmdir)
			printf("Error: File does not exist.\n");
		else
			printf("Error: Directory does not exist.\n");
		return;
	}

	if(rmdir)
	{
		unsigned char cmp[32];
		for(i = 0; i < 32; i++)
			cmp[i] = 0x0;
		cmp[0] = (char)0xE5;
		cmp[11] = (char)0x02;
		cdFunc(file, false);
		for(i = 0; i < numDirectories; i++)
		{
			if(strcmp(directories[i].DIR_Name, ".") != 0 && strcmp(directories[i].DIR_Name, "..") != 0
				 && strcmp(directories[i].DIR_Name, cmp) != 0)
			{
				printf("Error: Directory must be empty.\n");
				cdFunc("..", false);
				return;
			}
		}
		cdFunc("..", false);
	}

	//find location in data sector
	fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + CurrDirClus * 4, SEEK_SET);
	unsigned char buf[32];

	for(i = 0; i < index/(BPB_BytsPerSec*BPB_SecPerClus/32); i++)
	{
		fread(buf, 1, 4, f);
		fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + toInt(buf, 4) * 4, SEEK_SET);
	}

	//wipe file data from directory (in data sector)
	unsigned char temp[1];
	for(i = 0; i < 32; i++)
	{
		if(index/(BPB_BytsPerSec*BPB_SecPerClus/32) > 0)
			fseek(f, (FirstDataSector-BPB_RootClus+toInt(buf, 4))*BPB_BytsPerSec*BPB_SecPerClus
						+(32*((index)%(BPB_BytsPerSec*BPB_SecPerClus/32)))+i, SEEK_SET);
		else
			fseek(f, (FirstDataSector-BPB_RootClus+CurrDirClus)*BPB_BytsPerSec*BPB_SecPerClus
						+(32*((index)%(BPB_BytsPerSec*BPB_SecPerClus/32)))+i, SEEK_SET);
		temp[0] = (char)0x0;
		if(i == 0)
			temp[0] = (char)0xE5;
		if(i == 11)
			temp[0] = (char)0x02;
		fwrite(temp, 1, 1, f);
	}

	//wipe file contents from data sector

	buf[0] = directories[index].DIR_FstClusLO[0];
	buf[1] = directories[index].DIR_FstClusLO[1];
	buf[2] = directories[index].DIR_FstClusHI[0];
	buf[3] = directories[index].DIR_FstClusHI[1];

	unsigned char temp2[4];

	while(toInt(buf, 4) != 0x0 && toInt(buf, 4) != 0xFFFFFFFF && toInt(buf, 4) != 0x0FFFFFF8 &&
				toInt(buf, 4) != 0x0FFFFFFE && toInt(buf, 4) != 0x0FFFFFFF)
	{
		for(i = 0; i < BPB_BytsPerSec*BPB_SecPerClus; i++)
		{
			fseek(f, (FirstDataSector-BPB_RootClus+toInt(buf, 4))*BPB_BytsPerSec*BPB_SecPerClus+i, SEEK_SET);
			fwrite(temp, 1, 1, f);
		}

		for(i = 0; i < 4; i++)
			temp2[i] = buf[i];

		//find next cluster
		fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + toInt(buf, 4) * 4, SEEK_SET);
		fread(buf, 1, 4, f);

		//wipe file clusters from FAT sector
		for(i = 0; i < 4; i++)
		{
			fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + toInt(temp2, 4) * 4 + i, SEEK_SET);
			fwrite(temp, 1, 1, f);
		}
	}

	//wipe file data from directories[]
	strcpy(directories[index].DIR_Name, "");
	strcpy(directories[index].DIR_FstClusHI, "");
	strcpy(directories[index].DIR_FstClusLO, "");
	strcpy(directories[index].DIR_FileSize, "");

	//wipe from open table
	for(i = 0; i < 1024; i++)
	{
		if(strcmp(openfiles[i].Name, file) == 0)
		{
			closeFunc(openfiles[i].Name);
			break;
		}
	}

	initialize();

	return;
}

void cpFunc(char* file, char* to)
{
	int i = 0, index = 0;
	bool exists = false;
	bool cpToDir = false;

	//check that file exists and find to
	for(i = 0; i < numDirectories; i++)
	{
		if(strcmp(directories[i].DIR_Name, file) == 0 && !exists)
		{
			if(directories[i].DIR_Attributes[0] == (char)0x10)
			{
				printf("Error: Cannot use first argument on a directory.\n");
				return;
			}
			exists = true;
			index = i;
		}
		if(strcmp(directories[i].DIR_Name, to) == 0 && !cpToDir)
		{
			if(directories[i].DIR_Attributes[0] == (char)0x20)
			{
				printf("Error: Cannot use second argument on a file.\n");
				return;
			}
			cpToDir = true;
		}
		if(exists && cpToDir)
			break;
	}

	if(cpToDir)
	{
		cdFunc(to, false);
		for(i = 0; i < numDirectories; i++)
		{
			if(strcmp(directories[i].DIR_Name, file) == 0)
			{
				printf("Error: File with this name already exists in that directory.\n");
				cdFunc("..", false);
				return;
			}
		}
		cdFunc("..", false);
	}

	if(!exists)
	{
		printf("Error: First argument does not exist.\n");
		return;
	}

	for(i = 0; i < 1024; i++)
	{
		if((!cpToDir&&strcmp(openfiles[i].Name,to)==0) || (cpToDir&&strcmp(openfiles[i].Name,file)==0))
		{
			printf("Error: A file with the name of the new file name is open. Please close it.\n");
			return;
		}
	}

	long int size = toInt(directories[index].DIR_FileSize, 4);
	unsigned char fileContents[size+1], buf[BPB_BytsPerSec*BPB_SecPerClus], FstClus[4];

	FstClus[0] = directories[index].DIR_FstClusLO[0];
	FstClus[1] = directories[index].DIR_FstClusLO[1];
	FstClus[2] = directories[index].DIR_FstClusHI[0];
	FstClus[3] = directories[index].DIR_FstClusHI[1];

	int CurrClus = toInt(FstClus, 4), contentsIndex = 0;

	//read file into fileContents
	while(true)
	{
		fseek(f, (FirstDataSector-BPB_RootClus+CurrClus)*BPB_BytsPerSec*BPB_SecPerClus, SEEK_SET);

		//if this is the last cluster we're reading from
		if(size <= BPB_BytsPerSec*BPB_SecPerClus)
		{
			fread(buf, 1, size, f);
			for(i = 0; i < size; i++)
			{
				fileContents[contentsIndex] = buf[i];
				contentsIndex++;
			}
			fileContents[toInt(directories[index].DIR_FileSize, 4)] = '\0';
			break;
		}

		//if we are going to need to read from more clusters
		fread(buf, 1, BPB_BytsPerSec*BPB_SecPerClus, f);
		for(i = 0; i < BPB_BytsPerSec*BPB_SecPerClus; i++)
		{
			fileContents[contentsIndex] = buf[i];
			contentsIndex++;
		}
		size -= BPB_BytsPerSec*BPB_SecPerClus;

		fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + CurrClus * 4, SEEK_SET);
		fread(buf, 1, 4, f);
		CurrClus = toInt(buf, 4);
	}

	//copy file contents to new file
	if(cpToDir)
	{
		cdFunc(to, false);
		creatFunc(file, false);
		openFunc(file, "rw");
		writeFunc(file, contentsIndex, fileContents);
		closeFunc(file);
		cdFunc("..", false);
	}
	else
	{
		creatFunc(to, false);
		openFunc(to, "rw");
		writeFunc(to, contentsIndex, fileContents);
		closeFunc(to);
	}

	return;
}

void showopenFunc()
{
	int i = 0;
	for(i = 0; i < 1024; i++)
	{
		if(strcmp(openfiles[i].Name, "") != 0)
			printf("%s | %s\n", openfiles[i].Name, openfiles[i].Mode);
	}

	return;
}

int findLastClus()
{
	//finds last cluster of this directory
	unsigned char buf[8];
	LastClus = CurrDirClus;
	while(1)
	{
		fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + LastClus * 4, SEEK_SET);
		fread(buf, 1, 4, f);
		if(toInt(buf, 4) == 0x0 || toInt(buf, 4) == 0xFFFFFFFF ||
			 toInt(buf, 4) == 0x0FFFFFF8 || toInt(buf, 4) == 0x0FFFFFFE ||
			 toInt(buf, 4) == 0x0FFFFFFF)
			return LastClus;
		LastClus = toInt(buf, 4);
	}
}

void allocateNewClus()
{
	int i = 0;
	unsigned long int newClus = LastClus;
	unsigned char hex[9], write[5], buf[11];

	//find first empty cluster to be newly allocated cluster
	do{
		newClus++;
		fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + newClus * 4, SEEK_SET);
		fread(buf, 1, 4, f);
	}while(toInt(buf, 4) != 0x0);

	//link previous "last" cluster to new cluster
	sprintf(hex, "%x", newClus);

	for(i = 0; i < 8; i++)
	{
		if(hex[i] == '\0')
			break;
	}
	int j = i;

	//move leading 0s
	if(j != 8)
	{
		for(; i < 8; i++)
			hex[i] = '0';
		hex[8] = '\0';

		unsigned char hex2[9];

		for(i = 0; i < j; i++)
			hex2[8-j+i] = hex[i];
		for(i = 0; i < 8 - j; i++)
			hex2[i] = '0';
		hex2[8] = '\0';
		for(i = 0; i < 9; i++)
			hex[i] = hex2[i];
	}

	//convert to little endian
	unsigned char temp[2];
	for(i = 3; i >= 0; i--)
	{
		temp[0] = hex[i*2];
		temp[1] = hex[i*2+1];
		write[3-i] = (char)hexToInt(temp, 2);
	}
	write[4] = '\0';

	fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + LastClus * 4, SEEK_SET);
	fwrite(write, 1, 4, f);
	for(i = 0; i < 4; i++)
		write[i] = 255;
	fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + newClus * 4, SEEK_SET);
	fwrite(write, 1, 4, f);

	LastClus = newClus;
}

int toInt(unsigned char* buf, int size)
{
	int i = 0, j = 0;
	unsigned char d[size*2 + 1];	//will contain hex string e.g. "3AF1"

	//fills up d[] with hex string from buf
	for(i = size - 1; i >= 0; i--)
	{
		sprintf((unsigned char*)(d+j), "%02x", buf[i]);
		j += 2;
	}
	d[size*2] = '\0';
	return hexToInt(d, size*2);
}

int hexToInt(unsigned char* d, int size)
{
	//converts hex string in d[] to int using ASCII values
	int ans = 0, i = 0, j = 0, ex = 0, power, temp;

	for(i = size - 1; i >= 0; i--)
	{
		power = 1;
		if(d[i] >= 48 && d[i] <= 57)
			temp = d[i] - 48;
		else
			temp = d[i] - 87;
		for(j = 0; j < ex; j++)
			power *= 16;
		ex++;
		ans = ans + temp*power;
	}

	return ans;
}

void initialize()
{
	//initializes directories[] with all entries in root directory
	int i = 0, n = BPB_RootClus, dirnumber = 0;
	unsigned char buf[64];
	unsigned int Offset = FirstDataSector, ClusterOffset = 0;
	bool end = false;

	n = CurrDirClus;
	Offset = FirstDataSector + (n - BPB_RootClus) * BPB_SecPerClus;

	unsigned int Byte_Offset = Offset * BPB_BytsPerSec;

	while(true)
	{
		while(ClusterOffset < BPB_BytsPerSec * BPB_SecPerClus)
		{
			fseek(f, Byte_Offset + ClusterOffset, SEEK_SET);
			fread(buf, 1, 11, f);
			if(buf[0] == (char)0x0)
			{
				end = true;
				break;
			}
			for(i = 0; i < 11; i++)
			{
				if(buf[i] == ' ')
				{
					buf[i] = '\0';
					break;
				}
			}
			strcpy(directories[dirnumber].DIR_Name, buf);
			directories[dirnumber].DIR_Name[11] = '\0';
			ClusterOffset += 11;
			fseek(f, Byte_Offset + ClusterOffset, SEEK_SET);
			fread(directories[dirnumber].DIR_Attributes, 1, 1, f);
			ClusterOffset += 9;
			fseek(f, Byte_Offset + ClusterOffset, SEEK_SET);
			fread(directories[dirnumber].DIR_FstClusHI, 1, 2, f);
			ClusterOffset += 6;
			fseek(f, Byte_Offset + ClusterOffset, SEEK_SET);
			fread(directories[dirnumber].DIR_FstClusLO, 1, 2, f);
			ClusterOffset += 2;
			fseek(f, Byte_Offset + ClusterOffset, SEEK_SET);
			fread(directories[dirnumber].DIR_FileSize, 1, 4, f);
			ClusterOffset += 4;
			dirnumber++;
		}
		if(end)
			break;
		fseek(f, FirstFATSector*BPB_BytsPerSec*BPB_SecPerClus + n * 4, SEEK_SET);
		fread(buf, 1, 4, f);
		n = toInt(buf, 4);
		strcpy(buf, "");
		if(n == 0x0 || n == 0xFFFFFFFF || n == 0x0FFFFFF8 || n == 0x0FFFFFFE || n == 0x0FFFFFFF)
			break;
		Offset = FirstDataSector + (n - BPB_RootClus) * BPB_SecPerClus;
		Byte_Offset = Offset * BPB_BytsPerSec;
		ClusterOffset = 0;
	}
	numDirectories = dirnumber;

	return;
}

tokenlist *new_tokenlist(void)
{
	tokenlist *tokens = (tokenlist *) malloc(sizeof(tokenlist));
	tokens->size = 0;
	tokens->items = (char **) malloc(sizeof(char *));
	tokens->items[0] = NULL; /* make NULL terminated */
	return tokens;
}

void add_token(tokenlist *tokens, char *item)
{
	int i = tokens->size;
	tokens->items = (char **) realloc(tokens->items, (i + 2) * sizeof(char *));
	tokens->items[i] = (char *) malloc(strlen(item) + 1);
	tokens->items[i + 1] = NULL;
	strcpy(tokens->items[i], item);
	tokens->size += 1;
}

char *get_input(void)
{
	char *buffer = NULL;
	int bufsize = 0;
	char line[5];
	while (fgets(line, 5, stdin) != NULL) {
		int addby = 0;
		char *newln = strchr(line, '\n');
		if (newln != NULL)
			addby = newln - line;
		else
			addby = 5 - 1;
		buffer = (char *) realloc(buffer, bufsize + addby);
		memcpy(&buffer[bufsize], line, addby);
		bufsize += addby;
		if (newln != NULL)
			break;
	}
	buffer = (char *) realloc(buffer, bufsize + 1);
	buffer[bufsize] = 0;
	return buffer;
}

tokenlist *get_tokens(char *input)
{
	char *buf = (char *) malloc(strlen(input) + 1);
	strcpy(buf, input);
	tokenlist *tokens = new_tokenlist();

	int i = 0, j = 0, size = 0;
	char c[1];
	char* tok;

	//tokens are delimited by space unless they are between quotes
	for(i = 0; i < strlen(input); i++)
	{
		size = 0;
		if(input[i] == '\"')
		{
			i++;
			while(size + i < strlen(input) && input[size + i] != '\"')
				size++;
		}
		else
		{
			while(size + i < strlen(input) && input[size + i] != ' ')
				size++;
		}

		char tok[size + 1];
		tok[size] = '\0';
		for(j = 0; j < size; j++)
		{
			tok[j] = input[i];
			i++;
		}

		add_token(tokens, tok);
		strcpy(tok, "");

		if(input[i] == '\"')
			i++;
	}

	return tokens;
}

void free_tokens(tokenlist *tokens)
{
	int i;
	for (i = 0; i < tokens->size; i++)
		free(tokens->items[i]);
	free(tokens->items);
	free(tokens);
}
