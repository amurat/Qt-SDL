#include "datatransfer.h"

int main()
{
    int numFloats = 343;
    std::vector<char> buf(numFloats*sizeof(float));
    float* floatBuf = (float*)&buf[0];
    std::fill(floatBuf, floatBuf+numFloats, 1.0);
    writeData(buf);
    return 0;
}
