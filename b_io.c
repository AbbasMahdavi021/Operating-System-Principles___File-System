/**************************************************************
* Class:  CSC-415-02  Fall 2022
* Names: Raschid Al-Kurdi, Abbas Mahdavi, Kevin Aziz, Vincent Marcinkowski 
* Student IDs: 921234870, 918345420, 920477087, 920508768 
* GitHub Names: ralkurdi, AbbasMahdavi021, kevinaziz11, SpaceManV3
* Group Name: Chutney
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*        cp2fs /home/student/Desktop/newFile.txt 
**************************************************************/
#include <math.h>           // for pow
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "fsLow.h"
#include "mfs.h"
#include "rootDirectory.h"
#include "volumeControlBlock.h"
#include "bitMap.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512
#define FILE_INITIAL_NUM_BLOCKS 10


//edited
typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	//holds the open file buffer
	char * BufferPos;		
	//Keeps track of the current position within the buffer
	int Pos;
	//however many valid bytes within the buffer are stored here
	int lenOfBuffer;		
	//holds the current BlockNum
	int currentBlock; 	
	//However many blocks the file occupies is stored here
	int numberOfBlocks; 	
	//the amoiunt of bytes being read
	int renderedBytes; 	
	//File Extent Pos
	int currExtentPos;   
	//what byte we are on in the file.
	int Offset;  
	//set depending on the flags passed to b_open
	int Validation;
	
	DE* FE;   // the DirectoryEntry relationship to the file
	} b_fcb;
	
	//file control block array
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].BufferPos = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].BufferPos == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}

//returns the address of a block on disk. It takes in a DirectoryEntry p of the file and the block you want from the file. 
//function taken directly from lecture slide of SFSU 415 operating systems class. Function written by Robert Bierman. 
int GetLBAfromFileBlock( int y, DE* p) { 
	int i = 0;
	//printf("name of file: %s and extent 0 num: %d and extent 0 blocknumber: %d\n", p->name, p->extents[0].num, p->extents[0].numPos);
	//loop from within the file's DE extent array
	//to arrive at the extent that contains the block that is being searched for
	while (p->extents[i].num <= y) {
		
		//grab next extent
		y = y - p->extents[i].num;
		//move onto the next extent
		i += 1; 
	} 
	//return first extent num that contains y
	return p->extents[i].numPos + y; 
	}




	//creates a file.
	DE fs_mkFile(DE* parent, char file[20], int lengthOfName){
		//declare file's DE to be returned
		DE DE; 
		//get free Pos of the parent and then check if it has space
		int Pos;
		int bitmapSize = 0;
		
		for( Pos = 2; Pos < BUFFER_SIZE; Pos +=1){
			if( parent[Pos].name[0] == '\0' ){
				//free directory entry
				break;
			}
		}
		if(Pos == BUFFER_SIZE){
			printf("No room in parent directory. Did not create file.\n");
			DE.name[0] = '\0'; 
			return DE; 
		}
		//Call blockAllocation by loading bitMap_struct
		
		//store the number of bytes in bitMap_struct
		 bitmapSize =  
		 ( ( vcb->numBlocks + 7) / 8 );

		bitProcessing();
		int DEnumBlocks =  (sizeof(DE) + vcb->blockSize -1) / vcb->blockSize;
		int DElocation = blockAllocation(DEnumBlocks, bitMap_struct, bitmapSize);
		int location = blockAllocation(FILE_INITIAL_NUM_BLOCKS, bitMap_struct, bitmapSize);
		
		if(location < 0 || DElocation < 0){ //failure condition
			printf("failed to allocate blocks for new file. File not created\n");
			DE.name[0] = '\0'; //indicate that file creation failed.
			return DE; 
		}
		//initialize the file DE fields
		DE.extents[0].numPos = location;
		DE.extents[0].num = FILE_INITIAL_NUM_BLOCKS;
		DE.extents[1].numPos = -1;
		DE.extents[1].num = 0; 
		DE.extents[2].numPos = -1;
		DE.extents[2].num = 0; 
		DE.extents[3].numPos = -1;
		DE.extents[3].num = 0; 
		DE.extents[4].numPos = -1;
		DE.extents[4].num = 0; 
		DE.extents[5].numPos = -1;
		DE.extents[5].num = 0; 
		DE.extents[6].numPos = -1;
		DE.extents[6].num = 0; 
		DE.extents[7].numPos = -1;
		DE.extents[7].num = 0;  
		DE.isDir = 0;
		DE.location = DElocation;
		//file starts with 0 
		DE.size = 0;
		//the name must be initialized value
		strncpy(DE.name, file, lengthOfName);
		//position the new file in the directory entry
		parent[Pos] = DE; 
		//write to disk
		int numBlocks = (BUFFER_SIZE * sizeof(DE) + vcb->blockSize -1) / vcb->blockSize; 
		if(LBAwrite(parent, numBlocks, parent[0].location) != numBlocks ){
			printf("LBAwrite Error:. File may not have been created.\n");
			//null terminate at the name field
			DE.name[0] = '\0';
		} 
		if(LBAwrite(&DE, 1, DElocation) != 1 ){
			printf("LBAwrite Error: File may not have been created.\n");
			//function failure
			DE.name[0] = '\0';
		} 

		return DE;
	}




// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR or O_CREAT
b_io_fd b_open (char * filename, int flags)
	{ 
	b_io_fd returnFd; 

	//*** TODO ***:  Modify to save or set any information needed
	//
	//
		
	if (startup == 0) b_init();  //Initialize our system
	
	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's
	//as ParsePath mutates, create copy of filename
	char* pathDecoy =  
	malloc(strlen(filename)+1);     

	strcpy(pathDecoy, filename);

	//declare a PatPassReturn object and parse the path
	parsePathResult parent = ParsePath(filename); 
	if(parent.status==0){
		//path is invalid
		perror("file not found\n");
		return -1;
	}
	
	DE FE;
	
	//path is in fact valid but the last element does not currently exist
	if(parent.status == 1){ 
		if( ( flags & O_CREAT ) == O_CREAT){
			printf("now\n");
			//generate the file
			//return the name of the created file
			
    		int Pos = 0;
    		for (int i = strlen(pathDecoy)-1; i >= 0; i--)
    		{
				if (pathDecoy[i] ==  '/') //find first slash
				{
					Pos = i++;
					break;
				}
			}
			
			char holder[strlen(pathDecoy) - Pos+1];
			int density = strlen(pathDecoy) - Pos;
			//copy over the chars starting at Pos into holder
			int i=0;
			while(pathDecoy[Pos] != '\0' && i < density){
				holder[i] = pathDecoy[Pos];

				i++;
				Pos++;
			}
			//null terminate the string
			holder[i] = '\0';

			FE = fs_mkFile(parent.directory, holder, density);
			printf("FE location right after mkFile function: %d\n", FE.location);
			if(FE.name[0] == '\0'){
				printf("failed to make file.\n");
				return -1;
			}
		}
		else{
			//create flag is not set so file cannot be created 
			printf("File not found.\n");
			return -1;
		}
	}
	else{
		//path is valid and last element does exist
		if(parent.directory[parent.indexOfLastElement].isDir == 1){
			//user tried to open a directory instead of a file
			printf("Directory cannot be opened this way.\n");
			return -1;
		}
		//last element exists and is a file
		FE = parent.directory[parent.indexOfLastElement];
		printf("File entry location is: %d\n", parent.directory[parent.indexOfLastElement].location );
	}
	//allocate memory 
	fcbArray[returnFd].BufferPos = malloc(B_CHUNK_SIZE);

	//Initialization
	fcbArray[returnFd].Pos = 0;
	fcbArray[returnFd].lenOfBuffer = B_CHUNK_SIZE; // B_CHUNK_SIZE - Pos
	fcbArray[returnFd].currentBlock = 0;
	fcbArray[returnFd].renderedBytes = 0;
	fcbArray[returnFd].Offset = 0;
	fcbArray[returnFd].FE = malloc((sizeof(DE) + vcb->blockSize -1 ));
	if(fcbArray[returnFd].FE == NULL){
		puts("malloc Error: b_open.");
	}
	if(LBAread(fcbArray[returnFd].FE, (sizeof(DE) + vcb->blockSize -1 ) / vcb->blockSize, FE.location) !=
	 (sizeof(DE) + vcb->blockSize -1 ) / vcb->blockSize){
		puts("LBAread Error: b_open");
	 }
	//determine the number of blocks the file takes
	int numberOfBlocks = 0;
	int i=0;
	while(fcbArray[returnFd].FE->extents[i].num > 0){
		numberOfBlocks += fcbArray[returnFd].FE->extents[i].num;
		i++;
	}
	printf("file location: %d  file number of blocks %d\n", FE.location, numberOfBlocks);
	fcbArray[returnFd].numberOfBlocks = numberOfBlocks;
	fcbArray[returnFd].currExtentPos = i;
	//flags determine the file permissions
	int permissions;
	if( (flags & O_RDONLY) == O_RDONLY){
		permissions = 1;
	}
	if( (flags & O_WRONLY) == O_WRONLY){
		permissions = 2;
	}
	if( (flags & O_RDWR) == O_RDWR){
		permissions == 3;
	}
	if( ( (flags & O_TRUNC) == O_TRUNC ) && ( permissions != 1 ) ){
		FE.size = 0;
		printf("Fitting the file: %s\n", filename);
		
		if(LBAwrite(&FE, (sizeof(DE) + vcb->blockSize-1) / vcb->blockSize, FE.location )
		!= (sizeof(DE) + vcb->blockSize-1) / vcb->blockSize ){
			printf("LBAwrite Error: b_open. Did not fit the file.\n");
			return -1;
		}
		//update the size of the file entry 
		fcbArray[returnFd].FE->size = 0;
	}
	fcbArray[returnFd].Validation = permissions;
	return (returnFd);						
	}

	
int b_seek (b_io_fd fd, off_t offsetPos, int whence)
	{
		//initialization
	if (startup == 0) b_init();  

	
	if ((fd < 0) || (fd >= MAXFCBS))
		{
			//filedescriptor is invalid
		return (-1); 					
		}
		if(whence == SEEK_SET){
			fcbArray[fd].Offset = offsetPos;
			return offsetPos;
		}
		if(whence==SEEK_CUR){
			fcbArray[fd].Offset += offsetPos;
			return fcbArray[fd].Offset;
		}
		if(whence==SEEK_END){
		fcbArray[fd].Offset = fcbArray[fd].FE->size + offsetPos;
		return fcbArray[fd].Offset;
		}		
	return (-1); 
	//indicate failure
	}
	



int b_write (b_io_fd fd, char * buffer, int num)
	{
		//initialization
	if (startup == 0) b_init();  

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS)){
		//file descriptor is invalid
			return (-1); 		
		}
		if(fcbArray[fd].Validation == O_RDONLY){
			printf("No permission to Write in this File!\n");
			return -1;
		}
	
	//bytes written at the end of the function
	int bytesWritten = num;
	
	//create a buffer 
	char* startOfBlock = malloc(B_CHUNK_SIZE);
	if(startOfBlock == NULL){
		printf("malloc Error: b_write\n");
		return -1;
	}
	//block and offset
	int blockTarget = (fcbArray[fd].Offset + vcb->blockSize -1 ) / vcb->blockSize -1;  
	int offsetPos =  fcbArray[fd].Offset % vcb->blockSize;
	printf("file offsetPos: %d and numberOfBlocks: %d\n", fcbArray[fd].Offset,  fcbArray[fd].numberOfBlocks);
	//check for space availability
	if( (fcbArray[fd].numberOfBlocks * vcb->blockSize - fcbArray[fd].Offset ) < num ) {
		printf("not enough space in file. Growing file...\n");
		// allocate space because there is not enough for the file

		//calculate the number of blocks 
		int bytesRemainder =
		 num - (fcbArray[fd].numberOfBlocks * vcb->blockSize - fcbArray[fd].Offset);
		//convert bytes to blocks
		int numBlocksRequired = (bytesRemainder + vcb->blockSize -1) / vcb->blockSize; 
		//check if numBlocksRequired is less than the regular we would need for the current extent
		Extent increment;
		int i=0;
		
		while(fcbArray[fd].FE->extents[i].num != 0 ){
			i++;
		}
		printf("i is: %d\n", i);
		//formula to double the number of blocks of the previous extent for current extent
		int passThrough = 10 * (int) pow(2, i);
		numBlocksRequired = (numBlocksRequired > passThrough) ? numBlocksRequired : passThrough;

		
		if(bitProcessing() == -1){
			printf("bitProcessing Error: b_write \n");
			free(startOfBlock);
			return -1;
		}
		
		// allocate space on disk 
 		int numPos = blockAllocation( numBlocksRequired , bitMap_struct, (vcb->numBlocks  + 7) / 8);
		
		
		if(numPos < 0){
			// PARTIAL WRITE
			printf("Not enough space on disk!\n Executing PARTIAL READ.\n");
			num = fcbArray[fd].numberOfBlocks * vcb->blockSize - fcbArray[fd].Offset;
			bytesWritten = num;
		}
		else{
			// If it returns a positive number then create an extent for the file 
			
			increment.num = numBlocksRequired;
			increment.numPos = numPos;
			fcbArray[fd].FE->extents[i] = increment;
			//update the number of blocks 
			fcbArray[fd].numberOfBlocks += numBlocksRequired;
			// updated the file DE extent table, write it to disk.
			int blocksForFileEntry = ( sizeof( DE ) + vcb->blockSize -1 ) / vcb->blockSize;
			if(LBAwrite( fcbArray[fd].FE, blocksForFileEntry, fcbArray[fd].FE->location) != blocksForFileEntry){
				printf("LBAread Error0; b_write\n");
				free(startOfBlock);
				return -1;
			}
		}
	}
	

	if(LBAread(startOfBlock, 1, GetLBAfromFileBlock( blockTarget, fcbArray[fd].FE)) != 1){
		printf("LBAread Error1; b_write\n");
		free(startOfBlock);
		return -1;
	}
	//fill the remaining space in the startOfBlock 
	int remainingSpaceInBlock = vcb->blockSize - offsetPos;
	strncpy(startOfBlock + offsetPos, buffer, remainingSpaceInBlock);
	//update Offset
	printf("strlen of buffer: %ld\n", strlen(buffer));
	remainingSpaceInBlock = (remainingSpaceInBlock > strlen(buffer)) ? strlen(buffer) : remainingSpaceInBlock;
	b_seek(fd, remainingSpaceInBlock, SEEK_CUR);
	//update num
	num -=  remainingSpaceInBlock;
	//commit the block to disk
	if(LBAwrite(startOfBlock, 1, GetLBAfromFileBlock( blockTarget, fcbArray[fd].FE)) !=1){
		printf("LBAread Error2; b_write\n");
		free(startOfBlock);
		return -1;
	}
	if(num <= 0){
	
		//update the size 
		fcbArray[fd].FE->size += bytesWritten;
	
		LBAwrite(fcbArray[fd].FE, ( sizeof(DE) + vcb->blockSize -1 ) / vcb->blockSize, fcbArray[fd].FE->location);
		
		free(startOfBlock);
		return bytesWritten;
	}
	
	//update the position within the buffer
	buffer += remainingSpaceInBlock; 
	//remaining content in the file needs to be pushed
	
	while(num > 0){
		
		blockTarget++;
		
		//read in the current block from disk
		if(LBAread(startOfBlock, 1, GetLBAfromFileBlock( blockTarget, fcbArray[fd].FE)) != 1){
			printf("LBAread Error3; b_write\n");
			free(startOfBlock);
			return -1;
		}
		// pass in the current block with contents in the buffer. 
		strncpy(startOfBlock, buffer, vcb->blockSize);
		//write the block back to disc
		if( LBAwrite(startOfBlock, 1, GetLBAfromFileBlock( blockTarget, fcbArray[fd].FE)) != 1){
			printf("LBAwrite Error4: b_write\n");
			free(startOfBlock);
			return -1;
		}
		//update num
		num -= vcb->blockSize;
		//update the buffer position     
		buffer+= vcb->blockSize;
	}
	//update the size of the file based on the amount of content written
	fcbArray[fd].FE->size += bytesWritten;
	LBAwrite(fcbArray[fd].FE, ( sizeof(DE) + vcb->blockSize -1) / vcb->blockSize, fcbArray[fd].FE->location);
	free(startOfBlock);
	b_seek(fd, bytesWritten, SEEK_CUR);
	return bytesWritten;
}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | where we are|  one by one                                    | from   |
//  | in first    |                                                |refilled|
//  | block       |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int num)
	{
		
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		//validate read permissions. If permission is 2, then file is WRITEONLY
		if( fcbArray[fd].Validation == 2){
			printf("No permission to Read this File!\n");
			return -1;
		}
		
		
		//b_seek(fd, 0, SEEK_SET); //start at beginning of file TODO: remove later
		//get how much content is left in file from current place.
		//printf("size of the file being read is: %ld and the offsetPos is: %d and the file name is: %s\n", fcbArray[fd].FE->size, fcbArray[fd].Offset,
		//fcbArray[fd].FE->name);

	int leftToRead = fcbArray[fd].FE->size - fcbArray[fd].Offset;
	if( leftToRead <= 0){
		printf("Reached EOF\n");
		return 0;
	}
	//define Pos 
	int userBufIndex = 0;


	
	if(num > leftToRead){
		printf("only Read Partial!\n");
		num = leftToRead;
	}
	// current num is our return value
	int totalBytesRead = num;
	
	fcbArray[fd].currentBlock = fcbArray[fd].Offset / B_CHUNK_SIZE;
	//calculate the current block number 
	int position = GetLBAfromFileBlock(fcbArray[fd].currentBlock, fcbArray[fd].FE);
	//get which byte of the current block we start at
	int startingByte = fcbArray[fd].Offset % B_CHUNK_SIZE;

	
		//load the block from disk 
		if( LBAread(fcbArray[fd].BufferPos, 1, position) != 1 ){
			printf("LBAread Error: Could not read from file!\n");
			return -1;
		}
		//read a block, now we are on the next block.
		fcbArray[fd].currentBlock++;
		//set the fcb buffer's Pos to the starting byte
		fcbArray[fd].Pos = startingByte;
	
		int amountToRead = (num < B_CHUNK_SIZE - fcbArray[fd].Pos) ? num : (B_CHUNK_SIZE - fcbArray[fd].Pos);
		//copy the bytes in to the user's buffer
		memcpy(buffer, fcbArray[fd].BufferPos+fcbArray[fd].Pos, amountToRead);
		userBufIndex+=amountToRead;
		num -= amountToRead;
		if(num == 0){
			
			//update the Offset
			b_seek(fd, totalBytesRead, SEEK_CUR);
			return totalBytesRead;
		}

	
	
	if(num > B_CHUNK_SIZE){ 
		int blocksToRead = num / B_CHUNK_SIZE;
		while(blocksToRead > 0){
			//users buffer reads in the current block
			LBAread(buffer + userBufIndex, 1, position);
			//update position within the users buffer
			userBufIndex+= B_CHUNK_SIZE;
			
			fcbArray[fd].currentBlock++;
			//determine the block number of the next file block
			position = GetLBAfromFileBlock(fcbArray[fd].currentBlock, fcbArray[fd].FE);
			//we just read a block so decrement the blocksToRead
			blocksToRead--;
		
			num -= B_CHUNK_SIZE;
		}
		if(num == 0){
			
			//update the Offset
			b_seek(fd, totalBytesRead, SEEK_CUR);
			return totalBytesRead;
		}
	}


		//update currentBlock 
		fcbArray[fd].currentBlock++;
		//get the next block to be read in
		position = GetLBAfromFileBlock(fcbArray[fd].currentBlock, fcbArray[fd].FE);
		//read in the block
		LBAread(fcbArray[fd].BufferPos, 1, position);
		//just read a block so update currentBlock
		fcbArray[fd].currentBlock++;

	// copy the bytes from file into buffer 
	memcpy(buffer + userBufIndex, fcbArray[fd].BufferPos, num);
	
	fcbArray[fd].renderedBytes += totalBytesRead;
	//update the Offset
	b_seek(fd, totalBytesRead, SEEK_CUR);
	// return the amount of bytes read into user's buffer
	return totalBytesRead;
}
	

int b_close (b_io_fd fd)
	{
		if(fcbArray[fd].BufferPos !=NULL){
			
		free(fcbArray[fd].BufferPos);
		//demonstrate that the file control block 
		//is no longer in use
		fcbArray[fd].BufferPos = NULL;
		}
		return 1;
	}
