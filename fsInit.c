/**************************************************************
* Class:  CSC-415-02  Fall 2022
* Names: Raschid Al-Kurdi, Abbas Mahdavi, Kevin Aziz, Vincent Marcinkowski 
* Student IDs: 921234870, 918345420, 920477087, 920508768 
* GitHub Names: ralkurdi, AbbasMahdavi021, kevinaziz11, SpaceManV3
* Group Name: Chutney
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "rootDirectory.h"
#include "fsLow.h"
#include "mfs.h"
#include "bitMap.h"
#include "volumeControlBlock.h"

// signature of file system checked to see if fs is mounted
#define MAGIC_NUMBER 221822
// maximum number of chars for a file path in our fs
#define MAX_PATH_LEN 400

// declaring needed vars
VCB *vcb;
int bitmapBlockSize;
char *bitMap_struct;

//DE *cwdPointer;
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{

	// get the vcb from the disk for usage
	vcb = malloc(blockSize);
	if (vcb == NULL)
	{
		printf("malloc Error: VCB");
		return -1;
	}
	if (LBAread(vcb, 1, 0) != 1)
	{
		printf("LBAread Error: vcb, initFileSystem\n");
		return -1;
	}

	// magic number must match
	if (vcb->magic != MAGIC_NUMBER)
	{
		// We must initilize the file System!

		printf("File System Initializing! \n With %ld number of blocks, and block size of %ld\n", numberOfBlocks, blockSize);

		// initialize the vcb
		vcb->magic = MAGIC_NUMBER;
		vcb->blockSize = blockSize;
		vcb->freeBlockCount = numberOfBlocks;

		// freespace
		int indexOfBitmap = Initialization(numberOfBlocks, blockSize);
		// rootDirectory
		int indexOfRoot = initDirectory(NULL, blockSize);

		vcb->bitMapIndex = indexOfBitmap;
		vcb->rootIndex = indexOfRoot;
		vcb->numBlocks = numberOfBlocks;

		// write the volume control block to disc

		if (LBAwrite(vcb, 1, 0) != 1)
		{
			printf("LBAwrite Error: vcb could not be written to disk!.\n");
			return -1;
		}
	}
	else
	{
		printf("File system has been initialized!\n");
	}
	// allocate memory to cwdPointer
	cwdPointer = malloc(sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1);
	if (cwdPointer == NULL)
	{
		printf("malloc Error: fsInit!\n");
		return -1;
	}
	// define the extern cwdPointer 
	//It must start pointing at the root directory array
	LBAread(cwdPointer, (sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1) / vcb->blockSize, vcb->rootIndex);
	return 0;
}

void exitFileSystem()
{
	printf("Exiting The FileSystem!\n");
}
