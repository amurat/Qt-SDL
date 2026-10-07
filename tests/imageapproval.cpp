#include "imageapproval.h"

#include <QString>
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

QImage load(const std::string& path)
{
    QImage image(QString::fromStdString(path));
    if (image.isNull()) {
        std::cerr << "unable to read image " << path << std::endl;
        return image;
    }
    return image.convertToFormat(QImage::Format_RGBA8888);
}

}  // namespace

std::string PngImageWriter::getFileExtensionWithDot() const
{
    return ".png";
}

void PngImageWriter::write(std::string path) const
{
    if (!image_.save(QString::fromStdString(path), "PNG")) {
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
    const QImage received = load(receivedPath);
    const QImage approved = load(approvedPath);
    if (received.isNull() || approved.isNull()) {
        return false;
    }
    if (received.size() != approved.size()) {
        std::cerr << "image size " << received.width() << "x" << received.height()
                  << " differs from approved " << approved.width() << "x" << approved.height()
                  << std::endl;
        return false;
    }

    const int w = received.width();
    const int h = received.height();
    QImage diff(w, h, QImage::Format_RGBA8888);
    long badPixels = 0;
    int maxDelta = 0;
    for (int y = 0; y < h; ++y) {
        const uchar* r = received.constScanLine(y);
        const uchar* a = approved.constScanLine(y);
        uchar* d = diff.scanLine(y);
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
        diff.save(QString::fromStdString(diffFile), "PNG");
        std::cerr << badPixels << " of " << long(w) * h << " pixels differ by more than "
                  << maxChannelDelta_ << " (allowed " << allowed << ", max delta " << maxDelta
                  << "), see " << diffFile << std::endl;
        return false;
    }
    std::remove(diffFile.c_str());
    return true;
}

void verifyImage(const QImage& image)
{
    ApprovalTests::Approvals::verify(PngImageWriter(image));
}
