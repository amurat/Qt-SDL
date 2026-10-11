// Renders each scene offscreen and compares it with its approved image
#include <doctest/doctest.h>
#include "ApprovalTests.hpp"
#include "glad/glad_gles32.h"
#include "glescontext.h"
#include "imageapproval.h"
#include "rendergles2.h"
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace {

std::vector<std::string> glMessages;

void onGLMessage(unsigned int source, unsigned int type, unsigned int id, unsigned int severity,
                 int length, const char* message, const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_HIGH_KHR || severity == GL_DEBUG_SEVERITY_MEDIUM_KHR) {
        glMessages.push_back(message);
    }
}

// one ANGLE context for the whole run
GLESContext* context()
{
    static GLESContext* ctx = []() -> GLESContext* {
        GLESContext* c = new GLESContext(nullptr);
        if (!c->createOffscreen()) {
            delete c;
            return nullptr;
        }
        // WebGL has no KHR_debug; renderScene() checks glGetError() instead
        if (glDebugMessageCallbackKHR) {
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR);
            glDebugMessageCallbackKHR(onGLMessage, nullptr);
        }
        return c;
    }();
    return ctx;
}

RgbaImage renderScene(Scene scene, int w, int h, int frame)
{
    GLESContext* ctx = context();
    REQUIRE(ctx);
    REQUIRE(ctx->setOffscreenRenderTarget(w, h));
    glMessages.clear();
    while (glGetError() != GL_NO_ERROR) {
    }

    RgbaImage image(w, h);
    {
        RenderGLES2 renderer(scene);
        renderer.setup(ctx);
        renderer.setFrame(frame);
        renderer.render(ctx, w, h);
        ctx->finish();

        std::vector<unsigned char> pixels(size_t(w) * h * 4);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        // GL rows run bottom-up
        for (int y = 0; y < h; ++y) {
            std::memcpy(image.scanLine(y), pixels.data() + size_t(h - 1 - y) * w * 4, size_t(w) * 4);
        }
        // the renderer's destructor deletes its GL objects
        ctx->makeCurrent();
    }
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        glMessages.push_back("glGetError: " + std::to_string(error));
    }

    std::string messages;
    for (const std::string& message : glMessages) {
        messages += message + "\n";
    }
    CHECK_MESSAGE(glMessages.empty(), messages);
    return image;
}

}  // namespace

// One test case per image: doctest stops a test case at its first failed approval.
// The test case name becomes the file name: tests/approved/<backend>/rendertests.<name>.approved.png

TEST_CASE("Triangle")
{
    verifyImage(renderScene(Scene::Triangle, 256, 256, 0));
}

// non-square, to cover the aspect ratio and the screen-space thickness
TEST_CASE("Lines.frame0")
{
    verifyImage(renderScene(Scene::Lines, 320, 240, 0));
}

TEST_CASE("Lines.frame150")
{
    verifyImage(renderScene(Scene::Lines, 320, 240, 150));
}

TEST_CASE("Icosahedron.frame0")
{
    verifyImage(renderScene(Scene::Icosahedron, 256, 256, 0));
}

// the sphere looks the same at every rotation, so vary the aspect ratio instead
TEST_CASE("Icosahedron.wide")
{
    verifyImage(renderScene(Scene::Icosahedron, 320, 240, 0));
}
