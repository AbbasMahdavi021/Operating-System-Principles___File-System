/**************************************************************
* Class:  CSC-415-02  Fall 2022
* Names: Raschid Al-Kurdi, Abbas Mahdavi, Kevin Aziz, Vincent Marcinkowski 
* Student IDs: 921234870, 918345420, 920477087, 920508768 
* GitHub Names: ralkurdi, AbbasMahdavi021, kevinaziz11, SpaceManV3
* Group Name: Chutney
* Project: Basic File System
*
* File: rootDirectory.h
*
* Description: Root Directory HeaderFile. Init rootDir values
*
**************************************************************/

#ifndef ROOTDIRECTORY_H_
#define ROOTDIRECTORY_H_
#define BUFFER_SIZE 50
#include <time.h>
#include "Extent.h"

//max number of directory items in a directory 

typedef struct DE{

    char name[20];
   //entry in bytes, checks size
     long int size;
    //location and sector num
    int location;
    time_t Touched;
    //date creation
    time_t Created;
    //time at which file was adjusted
    time_t Changed;
     //file property extension
     char fileProp;
    //extent array that can hold a max of 8 extents
    Extent extents[8];
    //if directory entry points to folder then isDir equals 1
   //else 0
    int isDir;
} DE;

int initDirectory(DE* Head, int Size);

#endif
