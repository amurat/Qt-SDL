#pragma once

class GLESContext;

class RenderGL {
public:
    virtual void setup(GLESContext* context) = 0;
    virtual void render(GLESContext* context, int w, int h) = 0;
};

