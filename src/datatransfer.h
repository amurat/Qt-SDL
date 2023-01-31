#pragma once

#include <vector>
#include <string>

extern void writeData(const std::vector<char> &buffer);
extern void readData(std::vector<char> &buf);

extern void writeDataBytes(const char* data, size_t size);
extern void readDataBytes(char* data, size_t size);


