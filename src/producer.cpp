#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <vector>
#include <string>

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

int main()
{
    std::vector<char> buf;
    buf.resize(4096);

    std::string message("Hello World of C++!\n");
    std::copy(message.c_str(), message.c_str() + message.length(), buf.begin());
    writeData(buf);
    return 0;
}
