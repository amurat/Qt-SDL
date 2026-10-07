#pragma once

#include "rendergl.h"
#include <memory>

class Hemisphere;
class Icosahedron;
struct LinesScene;

enum class Scene {
    Lines,
    Hemisphere,
    Icosahedron,
    Triangle
};

class RenderGLES2 : public RenderGL
{
public:
    // the scene picked by the RENDER_* #define in rendergles2.cpp
    static Scene defaultScene();

    explicit RenderGLES2(Scene scene = defaultScene());
    virtual ~RenderGLES2();

    virtual void setup(GLESContext* context);
    virtual void render(GLESContext* context, int w, int h);

    // animation frame drawn by the next render(), which then advances it by one
    void setFrame(int frame);

private:
    void setupTriangle(GLESContext* context);

    Scene scene_;
    int frame_;

    std::unique_ptr<Hemisphere> hemisphere_;
    std::unique_ptr<Icosahedron> icosahedron_;
    std::unique_ptr<LinesScene> lines_;

    unsigned int triangleProgram_;
    unsigned int triangleVao_;
    unsigned int triangleVbo_;
};
