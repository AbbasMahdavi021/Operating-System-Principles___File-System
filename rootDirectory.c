/**************************************************************
* Class:  CSC-415-02  Fall 2022
* Names: Raschid Al-Kurdi, Abbas Mahdavi, Kevin Aziz, Vincent Marcinkowski 
* Student IDs: 921234870, 918345420, 920477087, 920508768 
* GitHub Names: ralkurdi, AbbasMahdavi021, kevinaziz11, SpaceManV3
* Group Name: Chutney
* Project: Basic File System
*
* File: mfs.h
*
* Description: This file is required to start a root directory
* entry. This also contains a function that creates any given
* directory entry.
*
**************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "bitMap.h"
#include "rootDirectory.h"
#include "volumeControlBlock.h"


//When complete, this returns the first block of the new created dir
int initDirectory(DE* parent, int blockSize){
//hold the number of bytes in the bitmap
int bitByteSize = ( vcb->numBlocks + 7) / 8; 
//allocate space memory for the new dir
DE* dir = malloc(BUFFER_SIZE * sizeof(DE) + blockSize-1);

if(dir==NULL){
    perror("malloc Error: initDirectory\n");
    return -1;
}


// Write "." and ".." directoryEntry values into the buffer. 
// They go at indexPos 0 and 1.
// initialize the "." DirectoryEntry to itself.

//initialize all directory items
for (int i = 0; i < BUFFER_SIZE; i++){
    //all dir entries get NULL as name
    dir[i].name[0] = '\0';
}

if(bitProcessing() == -1){
    perror("Error: initDirectory Failed! \n");
    return -1;
}


// Allocate space on disk
// set blockNumber to the value that gets returned
 int blockNumber = blockAllocation(( BUFFER_SIZE * sizeof(DE) + blockSize-1) / blockSize , bitMap_struct, bitByteSize);

DE obj;
//initialize the directory
obj.name[0] = '.';
obj.name[1] = '\0';
obj.size = BUFFER_SIZE * sizeof(DE) + blockSize-1;
obj.location = blockNumber;
obj.isDir = 1;
dir[0] = obj;

//".." gets initialized to pointer to itself, when parent NULL
// Other wise, initialize the ".." entry to  the parent dir.
if(parent == NULL){
    dir[1] = obj;
    dir[1].name[0] = '.';
    dir[1].name[1] = '.';
    dir[1].name[2] = '\0';
}
else{
    dir[1] = parent[0];
    dir[1].name[0] = '.';
    dir[1].name[1] = '.';
    dir[1].name[2] = '\0'; 
}

int dirNumBlocks = (BUFFER_SIZE * sizeof(DE) + blockSize-1) / blockSize;
// The buffer gets written to the disk, at its block number

if(LBAwrite(dir, dirNumBlocks, blockNumber) != dirNumBlocks){
    perror("LBAwrite Error: initDirectory\n");
    return -1;
}

return blockNumber;
}
