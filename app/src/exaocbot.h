#pragma once
#include "util/image.h"
#include "capture/capture.h"
#include "v4l2.h"
#include "input.h"
#include "exaocbot_usb.h"
#include <typeinfo>
#include <utility>
#include <mutex>
#include <vector>
#include <array>
#include <optional>

namespace exaocbot {
	struct exaocbot_state {
		capture_state_t capture_state{};
		std::array<char, 16> part_name_str{};
		std::array<char, 16> benchmark_name_short_str{};
		std::array<char, 16> benchmark_score_str{};
		std::mutex input_events_mutex{};
		std::vector<input_event> input_events{};
		exaocbot_usb usb{};
	};
} // namespace exaocbot
