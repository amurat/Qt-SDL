// Force-included into the ApprovalTests library on iOS, where system() is
// unavailable. ApprovalTests only needs it to launch diff tools and to copy
// files, which rendertests doesn't use (QuietReporter, CopyApproveReporter).
#pragma once
#include <stdlib.h>
#ifdef __cplusplus
#include <cstdlib>
#endif

static inline int approvaltests_no_system(const char*)
{
    return -1;
}
#define system approvaltests_no_system
