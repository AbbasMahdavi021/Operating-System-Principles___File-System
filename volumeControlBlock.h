/**************************************************************
* Class:  CSC-415-02  Fall 2022
* Names: Raschid Al-Kurdi, Abbas Mahdavi, Kevin Aziz, Vincent Marcinkowski 
* Student IDs: 921234870, 918345420, 920477087, 920508768 
* GitHub Names: ralkurdi, AbbasMahdavi021, kevinaziz11, SpaceManV3
* Group Name: Chutney
* Project: Basic File System
*
* File: volumeControlBlock.h
*
* Description: vcb header file, Init VCB values.
*
**************************************************************/

#ifndef VCB_H_
#define VCB_H_

typedef struct VCB {
	//signature
    int magic;
	//starting block of bitmap on the disk
	int bitMapIndex;
	//starting block of the root on disk
	int rootIndex;
	//total number of blocks partitioned 
	int numBlocks;
	//size of one block on disk 
	int blockSize;
	//the number of remaining free blocks within the partitioned region of the disk
	int freeBlockCount;
} VCB;

extern VCB* vcb;
#endif