#ifndef LINEGEN_H
#define LINEGEN_H

#include <vector>
#include <glm/glm.hpp>

void generateCircleLineStripTestData(std::vector<glm::vec4>& varray);
void generateLineStripTestData(std::vector<glm::vec4>& varray);
void convertLineStripToLines(std::vector<glm::vec4>& varray);

#endif
