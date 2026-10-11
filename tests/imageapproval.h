#pragma once

#include "ApprovalTests.hpp"
#include <cstdint>
#include <string>
#include <vector>

// RGBA8 pixels, rows top-down
struct RgbaImage
{
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels;

    RgbaImage() = default;
    RgbaImage(int w, int h) : width(w), height(h), pixels(size_t(w) * h * 4) {}
    bool isNull() const { return pixels.empty(); }
    uint8_t* scanLine(int y) { return pixels.data() + size_t(y) * width * 4; }
    const uint8_t* constScanLine(int y) const { return pixels.data() + size_t(y) * width * 4; }
};

// Writes the received image as PNG
class PngImageWriter : public ApprovalTests::ApprovalWriter
{
public:
    explicit PngImageWriter(const RgbaImage& image) : image_(image) {}

    std::string getFileExtensionWithDot() const override;
    void write(std::string path) const override;
    void cleanUpReceived(std::string receivedPath) const override;

private:
    RgbaImage image_;
};

// Images match if at most maxBadPixelFraction of the pixels have a channel
// differing by more than maxChannelDelta (0-255). On a mismatch a
// <name>.diff.png is written next to the received image, with differing
// pixels in red.
class ToleranceImageComparator : public ApprovalTests::ApprovalComparator
{
public:
    ToleranceImageComparator(int maxChannelDelta, double maxBadPixelFraction)
        : maxChannelDelta_(maxChannelDelta), maxBadPixelFraction_(maxBadPixelFraction) {}

    bool contentsAreEquivalent(std::string receivedPath,
                               std::string approvedPath) const override;

private:
    int maxChannelDelta_;
    double maxBadPixelFraction_;
};

void verifyImage(const RgbaImage& image);
