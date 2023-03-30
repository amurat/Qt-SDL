#include "datatransfer.h"

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

