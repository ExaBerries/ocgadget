#pragma once
#include "util/image.h"
#include "capture/capture.h"
#include "msg/msg.h"
#include "input.h"
#include <typeinfo>
#include <utility>
#include <mutex>
#include <vector>
#include <array>
#include <optional>
#include <filesystem>

namespace ocgadget {
	struct ocgadget_state {
		capture_state_t capture_state{};
		std::array<char, 32> part_name_str{};
		std::array<char, 32> benchmark_name_short_str{};
		std::array<char, 32> benchmark_score_str{};
		std::mutex input_events_mutex{};
		std::vector<input_event> input_events{};
		msg_state_t msg_state{};
	};

	[[nodiscard]] std::filesystem::path base_screenshot_folder() noexcept;
	void save_screenshot(ocgadget_state& state) noexcept;
} // namespace ocgadget
