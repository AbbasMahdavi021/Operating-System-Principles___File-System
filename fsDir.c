/**************************************************************
* Class:  CSC-415-02  Fall 2022
* Names: Raschid Al-Kurdi, Abbas Mahdavi, Kevin Aziz, Vincent Marcinkowski 
* Student IDs: 921234870, 918345420, 920477087, 920508768 
* GitHub Names: ralkurdi, AbbasMahdavi021, kevinaziz11, SpaceManV3
* Group Name: Chutney
* Project: Basic File System
*
* File: fsDir.c
*
* Description: Main file to contain function implementation 
* for the file system behavior and functionality.
*
***************************************************************/


#include "mfs.h"
#include "volumeControlBlock.h"
#include <stdio.h>
#include <stdlib.h>
#include "rootDirectory.h"
#include "fsLow.h"
#include <string.h>
#include "bitMap.h"

// allocate space for directory
char *cwdObj = "/";
DE *cwdPointer;

//We want to return wether a path is valid or not.
//We can tell if true or not, by checking the directry's parent is 
// in the right spot in the path.

// This determines if a path is valid or exitst
// Returns a pointer of the second to last Directory Entry

// The status of the path gives us info
// 0, path not invalid, so we don't use the returned pointer.
// 1, path valid, but last part of it doesn't exist.
// 2, path valid, and last part of it does exist already.

parsePathResult ParsePath(char *path)
{
    // We'll return this wrapper struct
    parsePathResult ret;
    ret.indexOfLastElement = 0;

    // A path can be either relative, or absoloute,
    //We must find out which one it is.

    DE *root = malloc(sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1);
    if (root == NULL)
    {
        printf("malloc Error: parsePath\n");
        ret.status = -1;
        return ret;
    }
    int numBlocks = (sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1) / vcb->blockSize;
    if (path[0] == '/')
    {
        //Given that path is absolute, we know that it starts at the root.
        //This will be helpful to keep track.
        LBAread(root, numBlocks, vcb->rootIndex);
    }
    else
    {
        //Given that path is relative, it starts at the cwd
        LBAread(root, numBlocks, cwdPointer[0].location);
    }
    ret.directory = root; // set the result to the root/first dir
    // get the first child directory
    char *currentToken = strtok(path, "/");
    //This vacriable keep track of whther is child is found.
    int found;
    //loop the path, and each directory in it
    while (currentToken != NULL)
    {              
        found = 0; //No child has been found yet
        //seach for child in first Dir
        for (int i = 0; i < BUFFER_SIZE; ++i)
        {
            if (strcmp(ret.directory[i].name, currentToken) == 0)
            {
                //Found a child, and the path is good so far
                found = 1;
                //We set the found child, to be the parent
                DE *nextDir = malloc(sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1);
                if (nextDir == NULL)
                {
                    printf("malloc Error: parsepath\n");
                    ret.status = -1;
                    return ret;
                }
                if (ret.directory[i].isDir == 0)
                {
                    //We find a file, so path becomes invalid
                    //given that it's not the last token in path
                    currentToken = strtok(NULL, "/");
                    //check if last token is indeed a file
                    if (currentToken == NULL)
                    {
                        //last token was a file, so path is fine
                        //and the token has a valid file name
                        ret.status = 2;
                        ret.indexOfLastElement = i;
                        return ret;
                    }
                    //last token is not a file, so path is invalid
                    ret.status = 0;
                    return ret;
                } 
                //Child is a valid directory is the parent directory
                //the parent becomes the child, for the next loop
                LBAread(nextDir, numBlocks, ret.directory[i].location);
                ret.directory = nextDir;
                ret.indexOfLastElement = i;
                //If a path was not found, just continue the loop
                break;
            }
        }
        if (found != 1)
        {
            //Did not find the child in cwd
            //if child dir/file is not the last path, it's a invalid path
            currentToken = strtok(NULL, "/");
            // confirm child is last DE in path
            if (currentToken == NULL)
            {
                // current token was the last path, so path IS valid
                // But the directory/file does NOT exist yet
                ret.status = 1;
                return ret;
            }
            // Could not find the token
            // and it was not the last DE in path 
            //so path is invalid
            ret.status = 0;
            return ret;
        }
        // get the next token for checking
        currentToken = strtok(NULL, "/");
    }
    // All DE are valid, so return parent of the last entry
    ret.status = 2;

    //change the parent
    if (LBAread(ret.directory, numBlocks, ret.directory[1].location) != numBlocks)
    {
        printf("LBAread failed in parsePath \n");
        ret.status = -10;
    }
    return ret;
}

// this prints the cwd we are in.
char *fs_getcwd(char *pathname, size_t size)
{
    return cwdObj;
}


// get the absolute path 
char *getOriginalPath(char *path)
{
    //Hold the absolute path
    char newPath[1000] = {};
    //Process the input path
    char *structure[strlen(path)];
    // for strtok_r
    char *pntr;
    //keep track of postion in the stack structure
    int height = -1;

    // fill structure with tokens from path
    char *token = strtok_r(path, "/", &pntr);

    while (token != NULL)
    {
        if (strcmp(token, "..") == 0)
        {
            // if the structure isn't empty
            if (height > -1)
            {
                //get the last element
                height--;
            }
        }
        else if (strcmp(token, ".") == 0) {}
        else
        {
            //the current tokem is an element,
            // so push it to the stack
            structure[++height] = token;
        }
        //move on to the next token for checking
        token = strtok_r(NULL, "/", &pntr);
    }

    //create a string from the stack structure
    newPath[0] = '/';
    for (int i = 0; i <= height; ++i)
    {   // go through the stack
        // create the string out of all the elements
        strcat(newPath, structure[i]);
        //if we haven't reached the final element
        if (i != height)
        {

            //seperate the element from the next, with an /
            strcat(newPath, "/");
        }
        //if we ARE at the last element, we can add a null term
        else
        {
            strcat(newPath, "\0");
        }
    }
    //We convert the new path to a char pointer
    char *result = (char *)malloc(strlen(newPath) * sizeof(char) + 1);
    strcpy(result, newPath);

    return result;
}


int fs_setcwd(char *pathname)
{
    //we keep a clone of the pathname,
    //  since path name modifies it. 
    char *copyOfPath = malloc(strlen(pathname) + 1);
    strcpy(copyOfPath, pathname);

    parsePathResult res;
    res = ParsePath(pathname);

    if (res.status == -1)
    {
        printf("parsePath Error: setcwd\n");
        if (res.directory != NULL)
            free(res.directory);
        res.directory = NULL;
        free(copyOfPath);
        return -1;
    }
    if (res.status != 2)
    {
        printf("Invalid path %s\n", pathname);
        free(copyOfPath);
        if (res.directory != NULL)
            free(res.directory);
        res.directory = NULL;
        return -1;
    }
    //Path is invalid if it points to a file
    if (res.directory[res.indexOfLastElement].isDir != 1)
    {
        // path points to file and command is invalid
        printf("Path %s points to a file! \n", pathname);
        free(copyOfPath);
        if (res.directory != NULL)
            free(res.directory);
        res.directory = NULL;
        return -1;
    }


    //We change directory, since path is valid
    //pathname is relative or absolute

    if (copyOfPath[0] != '/')
    {
        // path contains /, so it's relative
        // so set the path to the cwdObj
        // a new String is made to contain everything
        char newCwdString[strlen(cwdObj) + 2 + strlen(copyOfPath)];
        //the old directy is first part of the string
        strcpy(newCwdString, cwdObj);
        //attach a /,
        strcat(newCwdString, "/");
        //attach the relative path
        strcat(newCwdString, copyOfPath);
        // convert the created path into absolute path
        // set cwdObj to new path
        cwdObj = getOriginalPath(newCwdString);
    }
    else
    {
        printf("path is %s\n", copyOfPath);

        // path is absolute
        // convert pathname to abs path
        // set the cwdObj to the new pathname
        cwdObj = getOriginalPath(copyOfPath); // may need to free old cwdObj
    }

    // free stuff
    // set cwdPointer
    int cwdBlocks = (sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1) / vcb->blockSize;
    if (LBAread(cwdPointer, cwdBlocks, res.directory[res.indexOfLastElement].location) != cwdBlocks)
    {
        printf("LBAread Error: delete \n");
    }
    free(copyOfPath);
    if (res.directory != NULL)
        free(res.directory);
    res.directory = NULL;
    return 0;
}

// creates new directory function
int fs_mkdir(const char *pathname, mode_t mode)
{
    char copy[strlen(pathname)];

    strcpy(copy, pathname);
    int indexPos = 0;
    int indexParent;
    for (int i = strlen(copy); i >= 0; i--)
    {
        if (copy[i] == '/')
        {
            indexPos = ++i;
            break;
        }
    }

    parsePathResult parent;
    // use parsePath to get parent
    parent = ParsePath((char *)pathname);
    //check status of the returne path
    if (parent.status != 1)
    {
        printf("This is an invalid path: %s\n", copy);
        if (parent.status == 2)
        {
            printf(" %s already exists!\n", pathname + indexPos);
        }
        if (parent.directory != NULL)
            free(parent.directory);
        parent.directory = NULL;
        return -1;
    }

    //Get the name of the new directory we're going to create

    char newDirectoryName[strlen(copy) - indexPos + 1];
    strncpy(newDirectoryName, copy + indexPos, strlen(copy) - indexPos + 1);


    if (strcmp(newDirectoryName, parent.directory[0].name) == 0)
    {
        //child's name must not match, the parent
        printf("New Directory has an invalid name.");
        return -1;
    }
    //check for free space in parent
    for (int i = 2; i < BUFFER_SIZE; i++)
    {
        if (strlen(parent.directory[i].name) == 0)
        {
            //Allocate the free space needed for the new folder
            //on disk, and change the bitmap 
            int newLocation = initDirectory(parent.directory, vcb->blockSize);
            if (newLocation == -1)
            {
                printf("initDirectory Error: mkdir\n");
                if (parent.directory != NULL)
                    free(parent.directory);
                parent.directory = NULL;
                return -1;
            }
            DE newDirectory;
            // set the directory entries data
            newDirectory.isDir = 1;
            newDirectory.Created = time(NULL);
            newDirectory.Touched = time(NULL);
            newDirectory.Changed = time(NULL);
            newDirectory.location = newLocation;
            // set the name of the directory
            strncpy(newDirectory.name, newDirectoryName, sizeof(newDirectory.name));
            parent.directory[i] = newDirectory;
            //update the parent data
            int parentBlocks = (sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1) / vcb->blockSize;
            LBAwrite(parent.directory, parentBlocks, parent.directory->location);
            if (parent.directory != NULL)
                free(parent.directory);
            parent.directory = NULL;
            return 1;
        }
    }
    // The isn't enought space in the parent,
    //if we get to this part, so we print this info
    printf("No avaliable space in the parent folder, for the new folder!\n");
    if (parent.directory != NULL)
        free(parent.directory);
    parent.directory = NULL;
    return -1;
}

// Confirms is item is a dir, -1: invalid, 1: its a dir, 0: its a file 
int fs_isDir(char *pathname)
{
    // call parsePath function on the path
    parsePathResult result = ParsePath(pathname);
    // analyze the returned value of parsePath
    if (result.status == -1)
    {
        printf("ParsePath Error: isDir\n");

        return -1;
        if (result.directory != NULL)
            free(result.directory);
        result.directory = NULL;
    }
    else if (result.status < 2)
    {
        printf("Path is Invalid\n");
        if (result.directory != NULL)
            free(result.directory);
        result.directory = NULL;
        return -1;
    }
    // With a valid path, check the element, for isDir or not
    int res = result.directory[result.indexOfLastElement].isDir;
    if (result.directory != NULL)
        free(result.directory);
    result.directory = NULL;
    return res;
}

// Checks if its a file, -1, invalid, 1, yes, 0 no
int fs_isFile(char *filename)
{
    int res = fs_isDir(filename);
    if (res < 0)
    {
        return res;
    }
    return (res == 1) ? 0 : 1;
}


fdDir *fs_opendir(const char *pathname)
{

    // call parsePath function on the path
    parsePathResult result = ParsePath((char *)pathname);

    // analyze the returned value of parsePath
    if (result.status == -1)
    {
        printf("ParsePath Error. \n");
        if (result.directory != NULL)
            free(result.directory);
        result.directory = NULL;
        return NULL;
    }
    else if (result.status < 2)
    {
        printf("Path is Invalid\n");
        if (result.directory != NULL)
            free(result.directory);
        result.directory = NULL;
        return NULL;
    }
    // the path is a valid path

    struct fs_stat *stat = malloc(sizeof(struct fs_stat));
    if (stat == NULL)
    {
        printf("malloc Error: opendir. Could not open the dir!\n");
        if (result.directory != NULL)
            free(result.directory);
        result.directory = NULL;
        return NULL;
    }
    fs_stat(pathname, stat);
    fdDir *dir = malloc(sizeof(fdDir));
    if (dir == NULL)
    {
        printf("malloc Error2: opendir\n");
    }

    dir->len = stat->st_size;
    dir->directoryStartLocation = result.directory[result.indexOfLastElement].location;
    dir->dirEntryPosition = 0;
    dir->directory = malloc(sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1);
    int numBlocks = (sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1) / vcb->blockSize;
    if (LBAread(dir->directory, numBlocks, dir->directoryStartLocation) != numBlocks)
    {
        printf("LBAread Error: opendir \n");
        if (result.directory != NULL)
            free(result.directory);
        result.directory = NULL;
        return NULL;
    }
    if (result.directory != NULL)
        free(result.directory);
    result.directory = NULL;
    return dir;
}
struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{
    //argument must be valid
    if (dirp == NULL)
    {
        printf("No specified directory entry!\n");
        return NULL;
    }
    if (dirp->dirEntryPosition > BUFFER_SIZE)
    {
        return NULL;
    }

    //create a struct, this will be returned
    struct fs_diriteminfo *result = malloc(sizeof(struct fs_diriteminfo));
    result->len = sizeof(struct fs_diriteminfo);

    //The file Prop will either be set to file or folder
    result->fileProp = (dirp->directory[dirp->dirEntryPosition].isDir == 1) ? FT_DIRECTORY : FT_REGFILE;
    //set the directory item's name
    strncpy(result->name, dirp->directory[dirp->dirEntryPosition].name, sizeof(result->name));
    //get the next directory item
    int i = dirp->dirEntryPosition;
    char *currName = dirp->directory[++dirp->dirEntryPosition].name;
    while (strlen(currName) == 0 && (i < BUFFER_SIZE))
    {
        dirp->dirEntryPosition++;
        i++;
        currName = dirp->directory[dirp->dirEntryPosition].name;
    }

    return result;
}

int fs_closedir(fdDir *dirp)
{
    //free fsStat
    free(dirp->directory);
    dirp->directory = NULL;
    // free dirp instance
    free(dirp);
    return 1;
}

//To delete a file
//To the delete the file, we must be in it's parent folder
//File name must be valid ofc.
//if it got deleted, return 1, else return 0
int fs_delete(char *filename)
{
    //find the file first
    printf("cwdPointer name: %s and location: %d\n", cwdPointer[4].name, cwdPointer[4].location);
    for (int i = 2; i < BUFFER_SIZE; ++i)
    {
        if (strcmp(cwdPointer[i].name, filename) == 0)
        {
            // delete the found file
            //use the bitmap to free the now free blocks/space
            for (int currentExt = 0; currentExt < 8; currentExt++)
            {
                if (cwdPointer[i].extents[currentExt].num != 0)
                    deallocateBlocks(cwdPointer[i].extents[currentExt].num, cwdPointer[i].extents[currentExt].numPos);
            }

            // if we change the name of the file to null,
            // we have removed it from the parent directory 
            cwdPointer[i].name[0] = '\0';
            //the updated parent dir gets written to disk
            if (LBAwrite(cwdPointer, (sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1) / vcb->blockSize, cwdPointer[0].location) !=
                (sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1) / vcb->blockSize)
            {
                printf("LBAwrite Error: delete.\n");
                return -1;
            }
            // return 1, to say deletion was complete!

            return 1;
        }
    }
    // file was not found, and so nothing was deleted, 
    //if this point is reached, so print this info
    printf("could not find the file %s . Nothing was deleted\n", filename);
    return 0;
}

// remove a folder
int fs_rmdir(const char *pathname)
{
    // call the parse path function,
    parsePathResult result = ParsePath((char *)pathname);
    // last element is invalid
    if (result.status != 2)
    {
        if (result.directory != NULL)
            free(result.directory);
        result.directory = NULL;
        return -1;
    }
    DE *toBeRemoved = malloc(sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1);
    if (toBeRemoved == NULL)
    {
        printf("malloc failed in rmdir\n");
        if (result.directory != NULL)
        {
            free(result.directory);
            result.directory = NULL;
        }
        return -1;
    }
    int numBlocks = (sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1) / vcb->blockSize;

    LBAread(toBeRemoved, numBlocks, result.directory[result.indexOfLastElement].location);

    if (result.directory[result.indexOfLastElement].isDir == 0)
    {
        fs_delete(result.directory[result.indexOfLastElement].name);
    }

    //check the folder for being empty
    for (int i = 2; i < BUFFER_SIZE; ++i)
    {
        if (strlen(toBeRemoved[i].name) != 0)
        {
            // not empty when found child in directory
            printf("Directory is not empty, so it can't be removed!\n");
            if (result.directory != NULL)
            {
                free(result.directory);
                result.directory = NULL;
            }
            return -1;
        }
    }
    // if it's empty, remove, and update freespace
    deallocateBlocks(numBlocks, result.directory[result.indexOfLastElement].location);
    // delete the directory’s DE in the parent
    // by setting the name to empty string
    result.directory[result.indexOfLastElement].name[0] = '\0';
    // write the updated parent to disk
    if (LBAwrite(result.directory, numBlocks, result.directory->location) != numBlocks)
    {
        printf("LBAwrite Error: rmdir\n");
    }
    if (result.directory != NULL)
        free(result.directory);
    result.directory = NULL;
    return 1;
}

// returns 1 is complete, otherwise 0
// populates BufferPos from the args path
int fs_stat(const char *path, struct fs_stat *BufferPos)
{
    // parsePath to check for valid or not
    parsePathResult ret = ParsePath((char *)path);
    if (ret.status != 2)
    {
        printf(" %s is not a valid.\n", path);
        if (ret.directory != NULL)
            free(ret.directory);
        ret.directory = NULL;
        return 0;
    }


    // path is valid
    DE lastElement = ret.directory[ret.indexOfLastElement];
    BufferPos->st_size = (ret.directory[ret.indexOfLastElement].isDir == 1) ? BUFFER_SIZE * sizeof(DE) + vcb->blockSize - 1 : 10 * vcb->blockSize;
    //change third ternary operand so that it calcs the size of the file using extents.
    BufferPos->st_blksize = vcb->blockSize;
    BufferPos->st_blocks = (lastElement.size + vcb->blockSize - 1) / vcb->blockSize;
    BufferPos->st_accesstime = lastElement.Touched || (time_t)0;
    BufferPos->st_modtime = lastElement.Changed || (time_t)0;
    BufferPos->st_createtime = lastElement.Created || (time_t)0;
    if (ret.directory != NULL)
        free(ret.directory);
    ret.directory = NULL;
    return 1;
}


//if destination exists already, move the src there.
// If not, set dest to src
// dest must be either be a valid path to dest, a name, or a file
int fs_mv(char *srcPath, char *destPath)
{
    // check is src is valid
    parsePathResult srcResult = ParsePath(srcPath);

    // Not a valid path case:
    if (srcResult.status != 2)
    {
        printf("Invalid path!\n");
        if (srcResult.directory != NULL)
            free(srcResult.directory);
        srcResult.directory = NULL;
        return -1;
    }
    int numBlocks = (sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1) / vcb->blockSize;
    parsePathResult destResult = ParsePath(destPath);
    // valid path, but last element is not a existing file

    if (destResult.status == 1)
    {
        printf("Changing the name!\n");
        int nameLen = sizeof(srcResult.directory[srcResult.indexOfLastElement].name);
        // then rename the src to destPath
        strncpy(srcResult.directory[srcResult.indexOfLastElement].name, destPath, nameLen);

        if (LBAwrite(srcResult.directory, numBlocks, srcResult.directory[0].location) != numBlocks)
        {
            printf("LBAwrite Error: mv\n");
            if (srcResult.directory != NULL)
                free(srcResult.directory);
            srcResult.directory = NULL;
            return -1;
        }
        if (destResult.directory != NULL)
            free(destResult.directory);
        destResult.directory = NULL;
        if (srcResult.directory != NULL)
            free(srcResult.directory);
        srcResult.directory = NULL;
        return 1;
    }
    if (destResult.status == 2)
    {
        // the dest does exist but it points to a file
        if (destResult.directory[destResult.indexOfLastElement].isDir != 1)
        {
            if (srcResult.directory[srcResult.indexOfLastElement].isDir == 1)
            {
                printf("Cannot complete move. This is a file!\n");
                if (srcResult.directory != NULL)
                    free(srcResult.directory);
                srcResult.directory = NULL;
                return -1;
            }

            //Same move process for a file
            // change the DE in dest array to the src file DE
            destResult.directory[destResult.indexOfLastElement] = srcResult.directory[srcResult.indexOfLastElement];
            // delete src in its original parent
            srcResult.directory[srcResult.indexOfLastElement].name[0] = '\0';
            // write the modified directory Arrays back to disk
            if (LBAwrite(destResult.directory, numBlocks, destResult.directory[0].location) != numBlocks)
            {
                puts("LBAwrite Error: mv");
                return -1;
            }
            if (LBAwrite(srcResult.directory, numBlocks, srcResult.directory[0].location) != numBlocks)
            {
                puts("LBAwrite Error2: mv");
                return -1;
            }
            return 1;
        }

        // destPath is a valid path to a directory
        // get the path to set to src
        char *pathToSrc = getOriginalPath(srcPath);
        // move srcPath into destPath
        // write the dest Dir array into memory from disk
        DE *destinationDir = malloc(sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1);
        if (destinationDir == NULL)
        {
            printf("malloc Error: mv\n");
            if (srcResult.directory != NULL)
                free(srcResult.directory);
            srcResult.directory = NULL;
            return -1;
        }

        if (LBAread(destinationDir, numBlocks, destResult.directory[destResult.indexOfLastElement].location) != numBlocks)
        {
            printf("LBAread Error3: mv\n");
            if (srcResult.directory != NULL)
                free(srcResult.directory);
            srcResult.directory = NULL;
            return -1;
        }
        for (int i = 2; i < BUFFER_SIZE; ++i)
        {
            if (strlen(destinationDir[i].name) == 0)
            {
                // Space available for src DE
                // copy src DE into the free space
                printf("Moving the dir: %s w/ isDir status of: %d\n", srcResult.directory[srcResult.indexOfLastElement].name, srcResult.directory[srcResult.indexOfLastElement].isDir);
                destinationDir[i] = srcResult.directory[srcResult.indexOfLastElement];
                // delete the src DE from the parent
                srcResult.directory[srcResult.indexOfLastElement].name[0] = '\0';
                // write the updated directories to disk
                if (LBAwrite(srcResult.directory, numBlocks, srcResult.directory[0].location) != numBlocks)
                {
                    printf("LBAread Error4: mv\n");
                    if (srcResult.directory != NULL)
                    {
                        free(srcResult.directory);
                        srcResult.directory = NULL;
                    }
                    return -1;
                }
                if (LBAwrite(destinationDir, numBlocks, destinationDir[0].location) != numBlocks)
                {
                    printf("LBAread Error5: mv\n");
                    if (srcResult.directory != NULL)
                    {
                        free(srcResult.directory);
                        srcResult.directory = NULL;
                    }
                    return -1;
                }
                if (srcResult.directory != NULL)
                    free(srcResult.directory);
                srcResult.directory = NULL;
                return 1;
            }
        }
        //There was no available space in the dest dir
        //so we print this info
        printf("No available space in the distination directory! Could not move the source!\n");
    }
    return -1;
}

// loads a directory array, with a specified dir entry.
DE *loadDir(DE toBeLoaded)
{
    int numBlocks = (sizeof(DE) * BUFFER_SIZE + vcb->blockSize - 1) / vcb->blockSize;
    DE *ret = malloc(numBlocks);
    if (ret == NULL)
    {
        puts("malloc Error: loadDir");
        return NULL;
    }
    if (LBAread(ret, numBlocks, toBeLoaded.location) != numBlocks)
    {
        puts("loadDir Error!");
        free(ret);
        return NULL;
    }
    return ret;
}
