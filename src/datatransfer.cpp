#include "datatransfer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

void writeData(const std::vector<char> &buffer)
{
    // create the shared memory segment
    const char *name = "sharedmemtest";
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    
    // configure the size of the shared memory object
    const int SIZE = buffer.size();
    ftruncate(shm_fd, SIZE);
    
    // now map the shared memory segment in the address space of the process
    void *ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    if (ptr == MAP_FAILED) {
        printf("Map failed\n");
        return -1;
    }

    std::copy(buffer.begin(), buffer.end(), (char*)ptr);
}

void readData(std::vector<char> &buf)
{
    /* the size (in bytes) of shared memory object */
    const int SIZE = buf.size();

    /* name of the shared memory object */
    const char* name = "sharedmemtest";

    /* shared memory file descriptor */
    int shm_fd;

    /* pointer to shared memory object */
    void* ptr;

    /* open the shared memory object */
    shm_fd = shm_open(name, O_RDONLY, 0666);

    /* memory map the shared memory object */
    ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);

    std::copy((char*)ptr, ((char*)ptr) + SIZE, buf.begin());

    /* remove the shared memory object */
    //shm_unlink(name);
}

