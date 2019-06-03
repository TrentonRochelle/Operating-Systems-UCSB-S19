#include <stddef.h>
#include <printf.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "disk.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef enum{                   // because C is inferior
    false,
    true
} bool;

typedef struct{ //in virtual disk
    bool used;                  // whether the file is being used
    char name[15];              // file name
    int size;                   // file size
    int head;                   // first data block
    int numBlocks;              // number of blocks
    int fdCount;                // number of file descriptors using this file
} file;


typedef struct{ //in virtual disk
    int numFiles;               // number of files in the FAT (<=64)
    // fileInfo files[64];         // file information array
    int filesIndex;             // index of where files start (5)
    int dataIndex;              // block where data starts
    int blocksUsed;             // how many blocks are in use?
    int fatIndex;               // index of where there FAT starts (1)
} superBlock;

typedef struct{ //in memory
    bool used;                  // whether the file descriptor is being used
    int fileIndex;              // file index
    int offset;                 // read offset used by fs_read()
} fileDescriptor;


typedef struct{ //in virtual disk
    //starts at index 4096!
    int fat[4096];              // the array showing where the data is located
} FAT;

int make_fs(char *disk_name);
int mount_fs(char *disk_name);
int umount_fs(char *disk_name);
int fs_open(char *name);
int fs_close(int fildes);
int fs_create(char* name);
int fs_delete(char* name);
int fs_read(int fildes,void *buf, size_t nbyte);
int fs_write(int fildes,void *buf, size_t nbyte);
int fs_get_filesize(int fildes);
int fs_lseek(int fildes, off_t offset);
int fs_truncate(int fildes, off_t length);

//Helper Functions
int get_emptyfdIndex();
int get_fdIndex(int fileIndex);
int get_fileIndex(char* name);
int get_emptyfileIndex();
int get_emptyfatIndex();