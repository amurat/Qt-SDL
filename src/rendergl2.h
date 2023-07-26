#pragma once

#include "rendergl.h"

class RenderGL2 : public RenderGL
{
public:
    virtual void setup();
    virtual void render(int w, int h);
};

