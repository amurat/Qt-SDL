#pragma once

#include "rendergl.h"

class RenderGLES2 : public RenderGL
{
public:
    virtual void setup();
    virtual void render(int w, int h);
};
