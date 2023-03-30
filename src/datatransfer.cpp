#include "datatransfer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>


void writeDataBytes(const char* data, size_t size)
{
    // create the shared memory segment
    const char *name = "sharedmemtest";
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (-1 == shm_fd) {
        printf("shm_open failed\n");
        return;
    }
    // configure the size of the shared memory object
    const int SIZE = size;
    ftruncate(shm_fd, SIZE);
    
    // now map the shared memory segment in the address space of the process
    void *ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    if (ptr == MAP_FAILED) {
        printf("Map failed\n");
        return;
    }

    memcpy(ptr, data, size);
}

void writeData(const std::vector<char> &buffer)
{
    writeDataBytes(buffer.data(), buffer.size());
}


void readDataBytes(char* data, size_t size)
{
    /* the size (in bytes) of shared memory object */
    const int SIZE = size;

    /* name of the shared memory object */
    const char* name = "sharedmemtest";

    /* shared memory file descriptor */
    int shm_fd;

    /* pointer to shared memory object */
    void* ptr;

    /* open the shared memory object */
    shm_fd = shm_open(name, O_RDONLY, 0666);
    if (-1 == shm_fd) {
        printf("shm_open failed\n");
        return;
    }

    
    /* memory map the shared memory object */
    ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);

    memcpy(data, ptr, size);

    /* remove the shared memory object */
    //shm_unlink(name);
}

void readData(std::vector<char> &buf)
{
    readDataBytes(buf.data(), buf.size());
}
