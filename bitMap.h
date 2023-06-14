/**************************************************************
* Class:  CSC-415-02  Fall 2022
* Names: Raschid Al-Kurdi, Abbas Mahdavi, Kevin Aziz, Vincent Marcinkowski 
* Student IDs: 921234870, 918345420, 920477087, 920508768 
* GitHub Names: ralkurdi, AbbasMahdavi021, kevinaziz11, SpaceManV3
* Group Name: Chutney
* Project: Basic File System
*
* File: bitmap.h

* Description: headerfile for bitmap initialization.
*
***************************************************************/


#ifndef BITMAP_H_
#define BITMAP_H_

extern  char * bitMap_struct;
extern int bitmapBlockSize;
int Initialization(int blocksInTotal, int size);
int blockAllocation(int allocate,  char* bitMap, int mapSize);
void deallocateBlocks(int Free, int position);
//loads the bitmap into memory 
//1 means success and -1 is failure
int bitProcessing();
#endif