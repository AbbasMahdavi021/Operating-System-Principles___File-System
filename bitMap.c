/**************************************************************
 * Class:  CSC-415-02  Fall 2022
 * Names: Raschid Al-Kurdi, Abbas Mahdavi, Kevin Aziz, Vincent Marcinkowski
 * Student IDs: 921234870, 918345420, 920477087, 920508768
 * GitHub Names: ralkurdi, AbbasMahdavi021, kevinaziz11, SpaceManV3
 * Group Name: Chutney
 * Project: Basic File System
 *
 * File: bitMap.c
 *
 * Description:
 *   This file includes functions that pertain to initializing
 *   the bitmap. The bitmap is a byte array where each bit
 *   corresponds to a block on disk.
 **************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <getopt.h>
#include <string.h>
#include "volumeControlBlock.h"
#include "bitMap.h"
#include "fsLow.h"

#define BITMAP_LOCATION 1
// defined to build .o, please chnage
#define CHAR_BIT 1

// returns the address of the bitmap
int Initialization(int Block, int Size)
{ // Block is the total number of blocks for the file system.

    int numberofBytes = ((Block + 7) / 8);
    int numberOfBlocks = (numberofBytes + Size - 1) / Size;
    bitmapBlockSize = numberOfBlocks;
    // . Before you can initialize the bits, you must have the memory to do so. So malloc
    // the space needed. In this example 5 * Size

    bitMap_struct = calloc((numberOfBlocks * Size), sizeof(unsigned char));
    if (bitMap_struct == NULL)
    {
        printf("malloc Error: bitmap_init\n");
        return -1;
    }

    // set the first bit to a 1 for the VCB
    // set numberOfBlocks bits to 1 for bitmap starting at BITMAP_LOCATION
    int res = blockAllocation((1 + numberOfBlocks), bitMap_struct, (numberOfBlocks * Size)); // 1 for the vcb + numberOfBlocks for the bitmap as first arg. numberOfBlocks*Size for bitmapSize
    if (res == -1)
    {
        printf("Not avaliable Space for the VCB and the Bitmap.\n");
        printf("Number of Blocks: %d\n", numberOfBlocks);
        return -1;
    }

    LBAwrite(bitMap_struct, numberOfBlocks, BITMAP_LOCATION);

    printf("Size of Bitmap: %d\n", numberOfBlocks);
    return BITMAP_LOCATION;
}

char *characterToBinary(char x)
{
    static char binary[CHAR_BIT + 1] = {0};
    int i;
    for (i = CHAR_BIT - 1; i >= 0; i--)
    {
        binary[i] = (x % 2) + '0';
        x /= 2;
    }
    return binary;
}

char genMask(int passThrough)
{
    // returns a char bitmask representation of the input int
    unsigned char mask = 1;
    // check if this works for deallocateBlocks too.
    // might need to set mask to 0 sometimes.
    mask = mask << passThrough;
    return mask;
}

int freedCheck(int digit, char test)
{ // from right to left so if you want to check leftmost bit you input 7 and if you want the rightmost bit you do 0
    char mask = genMask(7 - digit);
    int result = 0;
    if ((mask & test) == mask)
    {
        result = 1;
    }
    return result;
} // returns 1 if it is not free and 0 if it is free

// bitmapSize is the number of bytes allocated for bitmap third param is in bytes
int getStartIndex(int numBlocksToAllocate, char *bitmap, int bitmapSize)
{ // returns the indexPos of the bit not byte

    int frontBit; // to be returned
    int backBit;
    int currBit;
    for (currBit = 0; currBit < bitmapSize * 8; currBit++)
    {
        int byte = currBit / 8;   // the byte in the bitmap that we are in
        int offset = currBit % 8; // the bit in the byte that we are in
        if (freedCheck(offset, bitmap[byte]) == 0)
        { // if it is free
            frontBit = currBit;
            backBit = currBit + numBlocksToAllocate;
            // this for loop is probably unnecessary
            for (; currBit < backBit; currBit++)
            { // check numBlocksToAllocate number of contiguous bits from currBit
                byte = currBit / 8;
                offset = currBit % 8;
                if (freedCheck(offset, bitmap[byte]) == 1)
                { // if it is not free
                    break;
                }
            }
            if (currBit == backBit)
            {
                return frontBit;
            }
        }
    }
    printf("Number of Locks: %d\n", numBlocksToAllocate);
    printf("Error in finding start indexPos in getStartIndex\n");
    return -1;
    // not enough space/room
}

int blockAllocation(int numBlocksToAllocate, char *bitmap, int bitmapSize)
{ // returns the starting block number of the free space allocated on disc
    // get vcb for bitmap and freeblockcount
    // VCB* vcb = malloc(sizeof(VCB));
    if (vcb == NULL)
    {
        printf("malloc Error2: vcb\n");
        return -1;
    }

    if (vcb->freeBlockCount < numBlocksToAllocate)
    {
        printf("Not enough space on disc to allocate %d w/ number of blocks\n Free block count is %d\n", numBlocksToAllocate, vcb->freeBlockCount);
        return -1;
    }

    int startIndex = getStartIndex(numBlocksToAllocate, bitmap, bitmapSize);
    printf("Directory created!\n");
    if (startIndex == -1)
    { // if there is no space in bitmap
        return -1;
    }
    // fill in numBlocksToAllocate number of 1s
    for (int currBit = startIndex; currBit < numBlocksToAllocate + startIndex; currBit++)
    {
        int byte = currBit / 8;               // the byte in the bitmap currently
        int offset = currBit % 8;             // the bit in the byte currently
        char mask = genMask(7 - offset);      // rightmost bit - offset
        bitmap[byte] = (bitmap[byte] | mask); // set the bit to a 1.
        // Set it to a zero with & instead of |
    }
    // the number of free blocks in vcb is to be updated
    vcb->freeBlockCount -= numBlocksToAllocate;
    // write the vcb back to disc
    LBAwrite(vcb, 1, 0); // vcb is at 0 block
    // return the starting indexPos
    LBAwrite(bitmap, bitmapBlockSize, BITMAP_LOCATION);
    return startIndex;
}

void deallocateBlocks(int Contiguity, int entryPosition)
{
    // load bitmap if not already in memory
    if (bitProcessing() == -1)
    {
        printf("bitProcessing Error: deallocatBlocks\n");
        return;
    }

    int startingByteAddress = entryPosition / 8;
    // calculate the bit in the byte
    int startingBitAddress = entryPosition % 8;
    // create mask to flip bits
    char mask =
        genMask(7 - startingBitAddress);

    while (Contiguity > 0)
    {
        // set the current bit to a zero.
        bitMap_struct[startingByteAddress] &= (~mask);
        // move forward one bit
        entryPosition++;

        Contiguity--;
        // determine the position in the array
        startingByteAddress = entryPosition / 8;
        startingBitAddress = entryPosition % 8;
        // update mask
        mask = genMask(7 - startingBitAddress);
    }
}
// loads the bitmap into memory
int bitProcessing()
{

    if (bitMap_struct == NULL)
    {
        // number of bytes the bitmap contains
        int bitByteSize =
            (((vcb->numBlocks + 7 / 8) / 8) + (vcb->blockSize - 1) / vcb->blockSize) / vcb->blockSize;
        // load bitmap in memory
        bitMap_struct = malloc(bitByteSize + vcb->blockSize - 1);

        if (bitMap_struct == NULL)
        {
            perror("malloc Error3: bitmap \n");
            return -1;
        }
        int bitBlock = (bitByteSize + vcb->blockSize - 1) / vcb->blockSize;

        if (LBAread(bitMap_struct, bitBlock, vcb->bitMapIndex) != bitBlock)
        {

            printf("LBAread Error: bitmap\n");
            return -1;
        }
    }

    return 1; // positive case
}
