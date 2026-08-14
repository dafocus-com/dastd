/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 04-AUG-2023
**/
#pragma once
#include <chrono>
#include <limits>
namespace dastd {

using steady_clock = std::chrono::steady_clock;
using steady_duration = steady_clock::duration;
using steady_time_point = steady_clock::time_point;
static constexpr steady_duration k_steady_duration_INVALID = std::numeric_limits<steady_duration>::max();


using high_resolution_clock = std::chrono::high_resolution_clock;
using high_resolution_duration = high_resolution_clock::duration;
using high_resolution_time_point = high_resolution_clock::time_point;

static constexpr high_resolution_duration k_high_resolution_duration_INVALID = std::numeric_limits<high_resolution_duration>::max();

} // namespace dastd
