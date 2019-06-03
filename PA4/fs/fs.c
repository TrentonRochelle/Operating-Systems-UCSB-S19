#include "fs.h"


fileDescriptor fd_array[32];
superBlock* superBlock_ptr;               // Super block
FAT* FAT_ptr;
file* files;
/*                              MAKE_FS                           */
int make_fs(char *disk_name){
    if(disk_name == NULL){
        printf("Name is NULL!\n");
        return -1;
    }
    make_disk(disk_name);
    open_disk(disk_name);

    superBlock_ptr = (superBlock*)malloc(sizeof(superBlock));
    // if (superBlock_ptr == NULL) return -1;
    superBlock_ptr->numFiles=0; 
    superBlock_ptr->blocksUsed=6; //1 for superBlock, 4 for FAT, 1 for files
    superBlock_ptr->dataIndex=4096; //where data starts
    superBlock_ptr->fatIndex=1; //1-4
    superBlock_ptr->filesIndex=5;
    // superBlock_ptr->files[0].size=10;
    // for(int i=0;i<64;i++){
    //     superBlock_ptr->files[i].used =false;
    // }
    char buf[BLOCK_SIZE] = "";
    memset(buf, 0, BLOCK_SIZE);
    memcpy(buf, superBlock_ptr, sizeof(superBlock)); //only takes 1 block
    block_write(0, buf);




    FAT_ptr = (FAT*)malloc(sizeof(FAT));
    // if (FAT_ptr == NULL) return -1;
    for(int i=0;i<4096;i++){ //initializing to empty
        FAT_ptr->fat[i] = -1;
    }

    int offset = 0;
    for(int i=1;i<5;i++){ //hardcoding...
        char buf[BLOCK_SIZE] = "";
        memset(buf, '\0', BLOCK_SIZE);
        memcpy(buf,(void*)FAT_ptr+offset, BLOCK_SIZE);
        block_write(i, buf);
        offset=offset+BLOCK_SIZE; //next block
    }
    free(superBlock_ptr);
    free(FAT_ptr);
    close_disk();
    // printf("Successfully made the file system\n\n");
    return 0;
}
/*                              MOUNT_FS                           */
int mount_fs(char *disk_name){
    if(disk_name == NULL){
        printf("Name is NULL!\n");
        return -1;
    }
    if(open_disk(disk_name)==-1){ //open disk handles error
        return -1;
    }
    /*                  SUPER BLOCK                   */
    superBlock_ptr = (superBlock*)malloc(sizeof(superBlock));
    char *address = (char*)superBlock_ptr;
    char buf[BLOCK_SIZE] = "";
    memset(buf, '\0', BLOCK_SIZE);
    block_read(0, buf);
    memcpy(address, buf, sizeof(superBlock_ptr)); //copy block 0 to superBlock_ptr

    /*                  FILES                   */
    files= (file*)malloc(BLOCK_SIZE);
    memset(buf,0,BLOCK_SIZE);
    block_read(superBlock_ptr->filesIndex,buf);
    memcpy(files,buf,sizeof(file)*superBlock_ptr->numFiles); //copy block 5 to files
    


    /*                  FAT                   */
    FAT_ptr = (FAT*)malloc(sizeof(FAT));
    address = (char*)FAT_ptr;
    int offset = 0;
    for(int i=1;i<5;i++){
        // printf("i: %d\n",i);
        char buf[BLOCK_SIZE] = "";
        block_read(i, buf);
        memcpy(address+offset, buf, BLOCK_SIZE); //copy blocks 1-4 to FAT_ptr
        offset=offset+BLOCK_SIZE;
    }
    /*                  FILE DESCRIPTORS                   */
    for(int i=0;i<32;i++){
        fd_array[i].used=false;
        fd_array[i].fileIndex=-1;
        fd_array[i].offset=0;
    }
    // printf("Successfully mounted the file system\n\n");
    // printf("file[0].name : %s\n",files[0].name);
    // printf("numFiles : %d\n",superBlock_ptr->numFiles);

    return 0;
}
/*                              UNMOUNT_FS                           */
int umount_fs(char *disk_name){
    if(disk_name == NULL){
        printf("Name is NULL!\n");
        return -1;
    }
    // printf("file[0].name : %s\n",files[0].name);

    /*                  SUPER BLOCK                   */
    char buf[BLOCK_SIZE] = "";
    // memset(buf, 0, BLOCK_SIZE);
    memcpy(buf, superBlock_ptr, sizeof(superBlock));
    block_write(0, buf); //write superBlock to block 0
    /*                  FILES                   */
    file* file_ptr = (file*)files;
    memset(buf,0,BLOCK_SIZE);
    char* index = buf;
    for(int i =0;i<64;i++){
        if(files[i].used==true){
            memcpy(index, &files[i],sizeof(files[i]));
            index += sizeof(file);
        }
    }
    block_write(superBlock_ptr->filesIndex,buf); //write files to block 5

    /*                  FAT                   */
    int offset = 0;
    for(int i=1;i<5;i++){
        char buf[BLOCK_SIZE] = "";
        memset(buf, '\0', BLOCK_SIZE);
        memcpy(buf,(void*)FAT_ptr+offset, BLOCK_SIZE); //write FAT to blocks 1-4
        block_write(i, buf);
        offset=offset+BLOCK_SIZE;
    }
    /*                  FILE DESCRIPTORS                   */
    for(int i=0;i<32;i++){ //reset fds
        fd_array[i].used=false;
        fd_array[i].fileIndex=-1;
        fd_array[i].offset=0;
    }
    free(superBlock_ptr);
    free(FAT_ptr);
    if(close_disk()==-1){
        return -1;
    }
    // printf("Successfully unmounted the file system\n\n");
    return 0;
}
/*                              FS_OPEN                           */
int fs_open(char *name){
    if(name == NULL){
        printf("Name is NULL!\n");
        return -1;
    }
    if(strlen(name)>15){
        printf("File name is longer than 15 characters!\n");
        return -1;
    }
    int fdIndex = get_emptyfdIndex();
    if(fdIndex==-1){
        printf("There is a maximum amount of file descriptors open!\n");
        return -1;
    }
    int fileIndex = get_fileIndex(name);
    if(fileIndex==-1){
        printf("File name does not exist in the disk!\n");
        return -1;
    }
    fd_array[fdIndex].used=true;
    fd_array[fdIndex].fileIndex=fileIndex;
    fd_array[fdIndex].offset = 0;
    files[fileIndex].fdCount++;
    // printf("Successfully opened a file\n\n");
    return fdIndex;
}

int get_emptyfdIndex(){ //returns the first empty index in fd_array
    for(int i=0;i<32;i++){
        if(fd_array[i].used==false){
            return i;
        }
    }
    return -1;
}
int get_fdIndex(int fileIndex){ //returns the fd_array index from fileIndex. returns -1 if not there
    for(int i=0;i<32;i++){
        if(fd_array[i].fileIndex==fileIndex){
            return i;
        }
    }
    return -1;
}

int get_fileIndex(char* name){ //returns file index from file name. returns -1 if not there
    for(int i=0;i<63;i++){
        if((strcmp(files[i].name,name)==0)&&(files[i].used==true)){
            return i;
        }
    }
    return -1;
}
/*                              FS_CLOSE                           */
int fs_close(int fildes){
    int fdIndex = fildes;
    if(fdIndex==-1){
        printf("File descriptor is not open!\n");
        return -1;
    }
    files[fd_array[fdIndex].fileIndex].fdCount--;
    fd_array[fdIndex].used=false;
    fd_array[fdIndex].fileIndex=-1;
    fd_array[fdIndex].offset = 0;
    // printf("Successfully closed a file\n\n");
    return 0;
}

int fs_create(char* name){ //creates a file
    if(strlen(name)>15){
        printf("File name is longer than 15 characters!\n");
        return -1;
    }
    if(superBlock_ptr->numFiles==64){
        printf("There is a maximum amount of files in the disk!\n");
        return -1;
    }
    int inDisk = get_fileIndex(name);
    if(inDisk!=-1){
        printf("File name already exists in the disk!\n");
        return -1;
    }
    int fileIndex = get_emptyfileIndex(); //will return a positive number due to previous error checks
    files[fileIndex].used=true;
    // files[fileIndex].name=name;
    strcpy(files[fileIndex].name, name);
    files[fileIndex].size=0;
    files[fileIndex].head=-1; //set head to -1 which will be checked in write/delete
    files[fileIndex].fdCount=0;
    files[fileIndex].numBlocks=0;

    superBlock_ptr->numFiles++;

    // printf("Successfully created a file\n\n");
    return 0;   
}

int get_emptyfileIndex(){ //gets the first empty index in files. returns -1 if files are full 
    for(int i=0;i<64;i++){
        if((files[i].used==false) || ((int)files[i].used == -1)){
            return i;
        }
    }
    return -1;
}
/*                              FS_DELETE                           */
int fs_delete(char* name){ //deletes a file
    if(name == NULL){
        printf("Name is NULL!\n");
        return -1;
    }
    if(strlen(name)>15){
        printf("File name is longer than 15 characters!\n");
        return -1;
    }
    int fileIndex = get_fileIndex(name);
    if(fileIndex==-1){
        printf("File name does not exist in the disk!\n");
        return -1;
    }
    int fdIndex = get_fdIndex(fileIndex);
    if(fdIndex!=-1){
        printf("Cannot delete file that has an open file descriptor!\n");
        return -1;
    }
    int currentIndex;
    int nextIndex;
    currentIndex=files[fileIndex].head;
    if(currentIndex==-1){            //create->delete without any file modifying

    }
    else{                           //file gets modified then deleted
        nextIndex = FAT_ptr->fat[currentIndex]; //get next index to delete
        FAT_ptr->fat[currentIndex] = -1; //update FAT
        while(nextIndex!=currentIndex){ //end of file loops to same FAT index
            currentIndex = nextIndex;
            nextIndex = FAT_ptr->fat[currentIndex];
            FAT_ptr->fat[currentIndex] = -1;
        }
    }
    files[fileIndex].used=false;
    superBlock_ptr->numFiles--;
    // printf("Successfully deleted a file\n\n");
    return 0;
}

/*                              FS_READ                           */
int fs_read(int fildes,void *buf, size_t nbyte){
    if(nbyte<0){
        printf("Can't read negative bytes!\n");
    }
    if(fd_array[fildes].used==false){
        printf("Cannot read from a file descriptor that is not open!\n");
        return -1;
    }
    char *dst = buf;
    int fileIndex = fd_array[fildes].fileIndex;
    int currentIndex = files[fileIndex].head;
    if(currentIndex==-1){
        printf("File has no contents\n");
        return 0;
    }
    int blockCount = 0;
    int offset=fd_array[fildes].offset;
    while(offset>=BLOCK_SIZE){//Get to current block to start read
        currentIndex = FAT_ptr->fat[currentIndex];
        offset=offset-BLOCK_SIZE;
        blockCount++;
    }
    char block[BLOCK_SIZE]="";
    int buf_offset = 0;

    block_read(currentIndex+4096,block); //FAT index to DATA block
    int bytesRead = 0;
    for(int i=offset;i<BLOCK_SIZE;i++){ //start at offset of the block
        dst[bytesRead++] = block[i];
        if(bytesRead== (int) nbyte){
            fd_array[fildes].offset+=bytesRead;
            return bytesRead;
        }
    }
    blockCount++;
    strcpy(block,"");
    while(bytesRead<(int)nbyte && blockCount<=files[fileIndex].numBlocks){
        currentIndex = FAT_ptr->fat[currentIndex]; //get next block to read
        strcpy(block,"");
        block_read(currentIndex+4096,block);
        for(int i=0;i<BLOCK_SIZE;i++){
            dst[bytesRead++] = block[i];
            if(bytesRead== (int) nbyte){
                fd_array[fildes].offset+=bytesRead;
                return bytesRead;
            }
        }
        blockCount++;
    }
    fd_array[fildes].offset += bytesRead; //update fildes offset since reading implicitly updates the offset
    return bytesRead;
}

/*                              FS_WRITE                           */
int fs_write(int fildes,void *buf, size_t nbyte){
    if(nbyte<0){
        printf("Can't write negative bytes!\n");
    }
    if(fd_array[fildes].used==false){
        printf("Cannot write to a file descriptor that is not open!\n");
        return -1;
    }
    char *src = buf;
    int fileIndex = fd_array[fildes].fileIndex;
    int currentIndex = files[fileIndex].head;
    int blockCount = 0;
    int offset=fd_array[fildes].offset;
    while(offset>=BLOCK_SIZE){//Get to current block
        currentIndex = FAT_ptr->fat[currentIndex];
        offset-=-BLOCK_SIZE;
        blockCount++;
    }
    char block[BLOCK_SIZE]="";
    int bytesWrote = 0;
    if(currentIndex!=-1){ //file was created and has been written to before
        block_read(currentIndex+4096,block);
        int bytesRead = 0;
        for(int i=offset;i<BLOCK_SIZE;i++){
            block[i] = src[bytesWrote++]; //updates the block at the offset
            if(bytesWrote == (int)nbyte){ //if doesn't reach end of block
                block_write(currentIndex,block);
                fd_array[fildes].offset+=bytesWrote;
                if(files[fileIndex].size < fd_array[fildes].offset){
                    files[fileIndex].size = fd_array[fildes].offset;
                }
                return bytesWrote;
            }
        }
        block_write(currentIndex+4096,block); //gets to end of block
        blockCount++;
    }
    //if file has been written to before, blockCount is >0, else blockCount is 0
    strcpy(block,"");
    while(bytesWrote<(int)nbyte && blockCount<files[fileIndex].numBlocks){ //write to allocated blocks
        currentIndex = FAT_ptr->fat[currentIndex];
        strcpy(block,"");
        for(int i=0;i<BLOCK_SIZE;i++){
            block[i]=src[bytesWrote++];
            if(bytesWrote==(int)nbyte){
                block_write(currentIndex+4096,block);
                fd_array[fildes].offset+=bytesWrote;
                if(files[fileIndex].size < fd_array[fildes].offset){
                    files[fileIndex].size = fd_array[fildes].offset;
                }
                return bytesWrote;
            }
        }
        block_write(currentIndex+4096,block);
        blockCount++;
    }
    //write to new blocks
    strcpy(block,"");
    int nextIndex;
    while(bytesWrote < (int)nbyte){
        nextIndex = get_emptyfatIndex();
        if(nextIndex==-1){
            printf("There are no more free blocks!\n");
            return bytesWrote;
        }
        files[fileIndex].numBlocks++;
        if(files[fileIndex].head==-1){
             files[fileIndex].head=nextIndex;
             currentIndex = nextIndex;
        }
        FAT_ptr->fat[currentIndex] = nextIndex;
        currentIndex = nextIndex;
        FAT_ptr->fat[currentIndex] = currentIndex;
        for(int i=0;i<BLOCK_SIZE;i++){
            block[i]=src[bytesWrote++];
            if(bytesWrote==(int)nbyte){
                block_write(currentIndex+4096,block);
                fd_array[fildes].offset+=bytesWrote;
                if(files[fileIndex].size < fd_array[fildes].offset){
                    files[fileIndex].size = fd_array[fildes].offset;
                }
                return bytesWrote;
            }
        }
        block_write(currentIndex+4096,block);

    }
    fd_array[fildes].offset += bytesWrote;
    if(files[fileIndex].size < fd_array[fildes].offset){
        files[fileIndex].size = fd_array[fildes].offset;
    }
    return bytesWrote;
}

int get_emptyfatIndex(){ //returns the first index of the FAT that is empty
    for(int i=0;i<4096;i++){
        if(FAT_ptr->fat[i]==-1){
            return i;
        }
    }
    return -1;
}

int fs_get_filesize(int fildes){
    if(fd_array[fildes].used==false){
        printf("Cannot get size of a file if the file descriptor is not open!\n");
        return -1;
    }
    return files[fd_array[fildes].fileIndex].size;


}
/*                              FS_LSEEK                           */
int fs_lseek(int fildes, off_t offset){ //only changes offset
    if(fd_array[fildes].used==false){
        printf("Cannot lseek a file if the file descriptor is not open!\n");
        return -1;
    }
    if(offset>files[fd_array[fildes].fileIndex].size || offset<0){
        printf("Cannot set the offset pointer outside the file limits!\n");
        return -1;
    }
    fd_array[fildes].offset = (int)offset;
    return 0;
}
/*                              FS_TRUNCATE                           */
int fs_truncate(int fildes, off_t length){
    if(fd_array[fildes].used==false){
        printf("Cannot truncate a file if the file descriptor is not open!\n");
        return -1;
    }
    if(length > files[fd_array[fildes].fileIndex].size || length < 0){
        printf("Cannot set the length to outside the file limits!\n");
        return -1;
    }
    int numBlocks = ((int)length + BLOCK_SIZE)/BLOCK_SIZE;
    int fileIndex = fd_array[fildes].fileIndex;
    int currentIndex = files[fileIndex].head;
    if(currentIndex==-1){
        return 0;
    }
    for(int i=0;i<numBlocks;i++){
        currentIndex = FAT_ptr->fat[currentIndex];
    }
    FAT_ptr->fat[currentIndex]=currentIndex;
    while(FAT_ptr->fat[currentIndex]!=currentIndex){
        currentIndex = FAT_ptr->fat[currentIndex];
        FAT_ptr->fat[currentIndex]=currentIndex;
        // superBlock_ptr->blocksUsed-=1;
    }
    files[fileIndex].size = (int) length;
    files[fileIndex].numBlocks = numBlocks;
    for(int i=0;i<32;i++){
        if(fd_array[i].used==true && fd_array[i].fileIndex == fileIndex && fd_array[i].offset>(int)length){
            fd_array[i].offset=(int)length;
        }
    }
    return 0;
}
