#include "imageapproval.h"

#include "stb_image.h"
#include "stb_image_write.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {

std::string diffPath(const std::string& receivedPath)
{
    const std::string marker = ".received.";
    std::string path = receivedPath;
    size_t pos = path.rfind(marker);
    if (pos != std::string::npos) {
        path.replace(pos, marker.size(), ".diff.");
    } else {
        path += ".diff.png";
    }
    return path;
}

RgbaImage load(const std::string& path)
{
    RgbaImage image;
    int w = 0;
    int h = 0;
    int channels = 0;
    // any PNG layout, expanded to RGBA
    stbi_uc* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!data) {
        std::cerr << "unable to read image " << path << std::endl;
        return image;
    }
    image = RgbaImage(w, h);
    std::copy(data, data + image.pixels.size(), image.pixels.begin());
    stbi_image_free(data);
    return image;
}

bool save(const RgbaImage& image, const std::string& path)
{
    return stbi_write_png(path.c_str(), image.width, image.height, 4,
                          image.pixels.data(), image.width * 4) != 0;
}

}  // namespace

std::string PngImageWriter::getFileExtensionWithDot() const
{
    return ".png";
}

void PngImageWriter::write(std::string path) const
{
    if (!save(image_, path)) {
        throw std::runtime_error("unable to write " + path);
    }
}

void PngImageWriter::cleanUpReceived(std::string receivedPath) const
{
    std::remove(receivedPath.c_str());
    std::remove(diffPath(receivedPath).c_str());
}

bool ToleranceImageComparator::contentsAreEquivalent(std::string receivedPath,
                                                     std::string approvedPath) const
{
    const RgbaImage received = load(receivedPath);
    const RgbaImage approved = load(approvedPath);
    if (received.isNull() || approved.isNull()) {
        return false;
    }
    if (received.width != approved.width || received.height != approved.height) {
        std::cerr << "image size " << received.width << "x" << received.height
                  << " differs from approved " << approved.width << "x" << approved.height
                  << std::endl;
        return false;
    }

    const int w = received.width;
    const int h = received.height;
    RgbaImage diff(w, h);
    long badPixels = 0;
    int maxDelta = 0;
    for (int y = 0; y < h; ++y) {
        const uint8_t* r = received.constScanLine(y);
        const uint8_t* a = approved.constScanLine(y);
        uint8_t* d = diff.scanLine(y);
        for (int x = 0; x < w; ++x, r += 4, a += 4, d += 4) {
            int pixelDelta = 0;
            for (int c = 0; c < 4; ++c) {
                pixelDelta = std::max(pixelDelta, std::abs(int(r[c]) - int(a[c])));
            }
            maxDelta = std::max(maxDelta, pixelDelta);
            if (pixelDelta > maxChannelDelta_) {
                ++badPixels;
                d[0] = 255; d[1] = 0; d[2] = 0;
            } else {
                // dimmed approved image as context
                d[0] = a[0] / 4; d[1] = a[1] / 4; d[2] = a[2] / 4;
            }
            d[3] = 255;
        }
    }

    const long allowed = long(maxBadPixelFraction_ * w * h);
    const std::string diffFile = diffPath(receivedPath);
    if (badPixels > allowed) {
        save(diff, diffFile);
        std::cerr << badPixels << " of " << long(w) * h << " pixels differ by more than "
                  << maxChannelDelta_ << " (allowed " << allowed << ", max delta " << maxDelta
                  << "), see " << diffFile << std::endl;
        return false;
    }
    std::remove(diffFile.c_str());
    return true;
}

void verifyImage(const RgbaImage& image)
{
    ApprovalTests::Approvals::verify(PngImageWriter(image));
}
