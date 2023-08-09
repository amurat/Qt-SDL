#pragma once

#include "rendergl.h"

class RenderGL2 : public RenderGL
{
public:
    virtual void setup(GLESContext* context);
    virtual void render(GLESContext* context, int w, int h);
};

