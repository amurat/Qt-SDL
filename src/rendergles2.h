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
    Triangle,
    ColorChecker
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
    void setupColorChecker(GLESContext* context);
    void setupSrgbPass();
    // (re)creates the linear scene buffer at w x h; false if RGBA16F isn't renderable
    bool ensureLinearTarget(int w, int h);
    void releaseLinearTarget();
    void drawScene(int w, int h);

    Scene scene_;
    int frame_;

    std::unique_ptr<Hemisphere> hemisphere_;
    std::unique_ptr<Icosahedron> icosahedron_;
    std::unique_ptr<LinesScene> lines_;

    unsigned int triangleProgram_;
    unsigned int triangleVao_;
    unsigned int triangleVbo_;

    unsigned int colorCheckerProgram_;
    unsigned int colorCheckerVao_;
    int colorCheckerResolutionLoc_;

    // scenes render into a linear RGBA16F buffer, encoded to sRGB into the context's target
    unsigned int linearFbo_;
    unsigned int linearColor_;
    unsigned int linearDepth_;
    int linearWidth_;
    int linearHeight_;
    bool linearFailed_;
    unsigned int srgbProgram_;
    unsigned int srgbVao_;
};
