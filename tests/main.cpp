// doctest main plus the ApprovalTests setup shared by all tests
#define APPROVALS_DOCTEST
#include "ApprovalTests.hpp"
#include "imageapproval.h"
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

using namespace ApprovalTests;

namespace {

#if TARGET_OS_IPHONE
const char* const kBackend = "metal-ios";
#elif defined(__APPLE__)
const char* const kBackend = "metal";
#elif defined(__EMSCRIPTEN__)
const char* const kBackend = "webgl";
#else
const char* const kBackend = "d3d11";
#endif

// AutoApproveReporter copies with "cp" through system(), which iOS and wasm lack
class CopyApproveReporter : public Reporter
{
public:
    bool report(std::string received, std::string approved) const override
    {
        std::filesystem::copy_file(received, approved,
                                   std::filesystem::copy_options::overwrite_existing);
        return true;
    }
};

std::shared_ptr<Reporter> makeReporter()
{
    // RENDERTESTS_APPROVE=1 accepts every received image as the new reference
    const char* approve = std::getenv("RENDERTESTS_APPROVE");
    if (approve && std::strcmp(approve, "0") != 0) {
        return std::make_shared<CopyApproveReporter>();
    }
    return std::make_shared<QuietReporter>();
}

// at most 2/255 per channel, and at most 0.1% of the pixels beyond that
auto comparatorDisposer = FileApprover::registerComparatorForExtension(
    ".png", std::make_shared<ToleranceImageComparator>(2, 0.001));
auto subdirectoryDisposer = Approvals::useApprovalsSubdirectory(std::string("approved/") + kBackend);
auto reporterDisposer = Approvals::useAsDefaultReporter(makeReporter());

}  // namespace
