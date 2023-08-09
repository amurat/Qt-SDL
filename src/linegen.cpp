#include "linegen.h"

void generateCircleLineStripTestData(std::vector<glm::vec4>& varray)
{
    varray.clear();
    for (int u=0; u <= 360; u += 10)
    {
        double a = u*M_PI/180.0;
        double c = cos(a), s = sin(a);
        varray.emplace_back(glm::vec4((float)c, (float)s, 0.0f, 1.0f));
    }
}

void generateLineStripTestData(std::vector<glm::vec4>& varray)
{
    varray.clear();
    varray.emplace_back(glm::vec4(1.0f, -1.0f, 0.0f, 1.0f));
    for (int u=0; u <= 90; u += 10)
    {
        double a = u*M_PI/180.0;
        double c = cos(a), s = sin(a);
        varray.emplace_back(glm::vec4((float)c, (float)s, 0.0f, 1.0f));
    }
    varray.emplace_back(glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f));
    for (int u = 90; u >= 0; u -= 10)
    {
        double a = u * M_PI / 180.0;
        double c = cos(a), s = sin(a);
        varray.emplace_back(glm::vec4((float)c-1.0f, (float)s-1.0f, 0.0f, 1.0f));
    }
    varray.emplace_back(glm::vec4(1.0f, -1.0f, 0.0f, 1.0f));
}

void convertLineStripToLines(std::vector<glm::vec4>& varray)
{
    std::vector<glm::vec4> result;
    const size_t num_lines = varray.size()-1;
    for (auto i = 0; i < num_lines; i++) {
        result.push_back(varray[i]);
        result.push_back(varray[i+1]);
    }
    result.push_back(varray[num_lines]);
    result.push_back(varray[0]);

    varray = result;
}
