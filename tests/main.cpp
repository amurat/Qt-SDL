// doctest main plus the ApprovalTests setup shared by all tests
#define APPROVALS_DOCTEST
#include "ApprovalTests.hpp"
#include "imageapproval.h"
#include <cstdlib>
#include <cstring>
#include <memory>

using namespace ApprovalTests;

namespace {

#ifdef __APPLE__
const char* const kBackend = "metal";
#else
const char* const kBackend = "d3d11";
#endif

std::shared_ptr<Reporter> makeReporter()
{
    // RENDERTESTS_APPROVE=1 accepts every received image as the new reference
    const char* approve = std::getenv("RENDERTESTS_APPROVE");
    if (approve && std::strcmp(approve, "0") != 0) {
        return std::make_shared<AutoApproveReporter>();
    }
    return std::make_shared<QuietReporter>();
}

// at most 2/255 per channel, and at most 0.1% of the pixels beyond that
auto comparatorDisposer = FileApprover::registerComparatorForExtension(
    ".png", std::make_shared<ToleranceImageComparator>(2, 0.001));
auto subdirectoryDisposer = Approvals::useApprovalsSubdirectory(std::string("approved/") + kBackend);
auto reporterDisposer = Approvals::useAsDefaultReporter(makeReporter());

}  // namespace
