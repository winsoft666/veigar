#ifndef VEIGAR_DETAIL_METER_H_
#define VEIGAR_DETAIL_METER_H_
#pragma once

#include <ctime>
#include <limits>

namespace veigar {
namespace detail {
class TimeMeter {
   public:
    TimeMeter() { lStartTime_ = std::clock(); }

    void Restart() { lStartTime_ = std::clock(); }

    // ms
    long Elapsed() const { return std::clock() - lStartTime_; }

    long ElapsedMax() const { return (std::numeric_limits<std::clock_t>::max)() - lStartTime_; }

    long ElapsedMin() const { return 1L; }

   private:
    std::clock_t lStartTime_;
};
}  // namespace detail
}  // namespace veigar

#endif  //!VEIGAR_DETAIL_METER_H_