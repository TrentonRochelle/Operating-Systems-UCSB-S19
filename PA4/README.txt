For this project I implemented a simple filesystem modelled after FAT.

***to run do the following:

    make clean
    make all
    ./test



The basic structure of my filesystem looks as follows:

--------------------------------------------------------------------------
| superBlock(0) | FAT(1-4) | files(5) |  NULL(6-4095) | DATA (4096-8191) |
--------------------------------------------------------------------------

The following functions have been implemented:
--------------------------------------------------------------------------
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
--------------------------------------------------------------------------

Each value of the FAT array is initialized to -1. As a new block is written to, both the new and last block is set the the index of the new block.
The FAT indexes range from 0-4095 while the data is represented in blocks 4096-8191. Thus a fat index of 0 refers to block 4096.


I had troubles with this project, especially in regards to the third testcase given because the file information was not staying into disk.
The issue was that I had an array of fileinfo files[64] in my superBlock struct. When I did malloc(sizeof(superBlock)),
it did not allocate memory for the array implicitly. At the advice of others, I allocated memory for files separately from the superBlock and put them in blocks.

