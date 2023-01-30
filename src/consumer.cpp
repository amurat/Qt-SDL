#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <vector>

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

int main()
{
	/* the size (in bytes) of shared memory object */
	const int SIZE = 4096;
    std::vector<char> buffer(SIZE);

    readData(buffer);
    char* data = &buffer[0];
    printf("%s", data);
	return 0;
}

