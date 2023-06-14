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
* Description: This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/

#ifndef _MFS_H
#define _MFS_H
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "rootDirectory.h"
#include "b_io.h"

#include <dirent.h>
#define FT_REGFILE	DT_REG
#define FT_DIRECTORY DT_DIR
#define FT_LINK	DT_LNK

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif


 extern DE* cwdPointer; //global variables
 extern char* cwdObj;


// This structure is returned by fs_readdir to provide the caller with information
// about each file as it iterates through a directory
struct fs_diriteminfo
	{
     short len;    /* length of this record */
     char fileProp;    
    char name[256]; 			/* filename max filename is 255 characters */
	};

// This is a private structure used only by fs_opendir, fs_readdir, and fs_closedir
// Think of this like a file descriptor but for a directory - one can only read
// from a directory.  This structure helps you (the file system) keep track of
// which directory entry you are currently processing so that everytime the caller
// calls the function readdir, you give the next entry in the directory
typedef struct fdDir
	{
	//Fill in this structure with what your open/read directory needs  *****/
	 short  len;		/*length of this record */
	 short	dirEntryPosition;	/*which directory entry position, like file pos */
	uint64_t	directoryStartLocation;		/*Starting LBA of directory */
	DE* directory;			//the directory array NEED to read it from disk upon initializationof the fdDir (in the open function)
	} fdDir;

//this struct packages the status code 
//DE pointer as return value for parse path
typedef struct parsePathResult {
	//reference pointer to a directory entry
	DE* directory;
	//indicates the status of path passed 
	//to parse path
	int status;
	//0 means path is invalid
	//1  means path is valid 
	//2 means path is valid and last token does exist
	int indexOfLastElement;
}parsePathResult;

//Given a file path, returns whether the path is valid
//indicates through status
//whether the last token exists or not

struct parsePathResult ParsePath(char* path);

// Key directory functions
int fs_mkdir(const char *pathname, mode_t mode);
int fs_rmdir(const char *pathname);

// Directory iteration functions
fdDir * fs_opendir(const char *pathname);
struct fs_diriteminfo *fs_readdir(fdDir *dirp);
int fs_closedir(fdDir *dirp);

// Misc directory functions
char * fs_getcwd(char *pathname, size_t size);
int fs_setcwd(char *pathname);   //linux chdir
int fs_isFile(char * filename);	//return 1 if file, 0 otherwise ONLY IMPLEMENT isDir
int fs_isDir(char * pathname);		//return 1 if directory, 0 otherwise
int fs_delete(char* filename);	//removes a file


// This is the strucutre that is filled in from a call to fs_stat
struct fs_stat
	{
	off_t     st_size;    		/* total size, in bytes */
	blksize_t st_blksize; 		/* blocksize for file system I/O */
	blkcnt_t  st_blocks;  		/* number of 512B blocks allocated */
	time_t    st_accesstime;   	/* time of last access */
	time_t    st_modtime;   	/* time of last modification */
	time_t    st_createtime;   	/* time of last status change */
	
	/* add additional attributes here for your file system */
	};

int fs_stat(const char *path, struct fs_stat *BufferPos);

int fs_mv(char* srcPath, char* destPath );


#endif

