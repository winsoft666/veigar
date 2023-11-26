#include "time_util.h"
#include "os_platform.h"
#ifdef VEIGAR_OS_WINDOWS
#ifndef _INC_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // !WIN32_LEAN_AND_MEAN
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif  // !_WINSOCKAPI_
#include <Windows.h>
#endif
#include <mmsystem.h>
#include <sys/timeb.h>
#pragma warning(disable : 4995)
#else
#include <sys/time.h>
#endif

namespace veigar {

int64_t TimeUtil::GetCurrentTimestamp() {
#ifdef VEIGAR_OS_WINDOWS
    union {
        int64_t ns100;
        FILETIME ft;
    } fileTime;
    GetSystemTimeAsFileTime(&fileTime.ft);

    // 116444736000000000 is the number of total 100 nanoseconds that from 1601/1/1 00:00:00:000 to 1970/1/1 00:00:00:000
    int64_t lNowMicroMS = (long long)((fileTime.ns100 - 116444736000000000LL) / 10LL);

    return lNowMicroMS;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t lNowMicroMS = tv.tv_sec * 1000000 + tv.tv_usec;
    return lNowMicroMS;
#endif
}

}  // namespace veigar