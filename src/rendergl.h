#pragma once

class RenderGL {
public:
    virtual void setup() = 0;
    virtual void render(int w, int h) = 0;
};

