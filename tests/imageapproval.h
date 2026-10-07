#pragma once

#include "ApprovalTests.hpp"
#include <QImage>
#include <string>

// Writes the received image as PNG
class PngImageWriter : public ApprovalTests::ApprovalWriter
{
public:
    explicit PngImageWriter(const QImage& image) : image_(image) {}

    std::string getFileExtensionWithDot() const override;
    void write(std::string path) const override;
    void cleanUpReceived(std::string receivedPath) const override;

private:
    QImage image_;
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

void verifyImage(const QImage& image);
