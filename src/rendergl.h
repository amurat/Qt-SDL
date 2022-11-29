#pragma once

class RenderGL {
public:
    virtual void setup() = 0;
    virtual void render(int w, int h) = 0;
};

class RenderGL2 : public RenderGL
{
public:
    virtual void setup();
    virtual void render(int w, int h);
};

class RenderGLES2 : public RenderGL
{
public:
    virtual void setup();
    virtual void render(int w, int h);
};

