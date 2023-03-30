#include "datatransfer.h"

int main()
{
    std::vector<char> buf;
    buf.resize(4096);

    std::string message("Hello World of C++!\n");
    std::copy(message.c_str(), message.c_str() + message.length(), buf.begin());
    writeData(buf);
    return 0;
}
