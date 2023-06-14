/**************************************************************
* Class:  CSC-415-02  Fall 2022
* Names: Raschid Al-Kurdi, Abbas Mahdavi, Kevin Aziz, Vincent Marcinkowski 
* Student IDs: 921234870, 918345420, 920477087, 920508768 
* GitHub Names: ralkurdi, AbbasMahdavi021, kevinaziz11, SpaceManV3
* Group Name: Chutney
* Project: Basic File System
*
* File: Extent.h
*
* Description: Contiguous memory management and handling.
* Tells us what an Extent is in our file system
*
***************************************************************/

typedef struct {
    //number of contiguous blocks within the extent table
    int num;
    //the location of the first block of the extent on disk
    //if the numPos is negative, this suggests that it is Not
    //an extent
    int numPos;

}Extent;