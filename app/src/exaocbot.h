#pragma once
#include "util/image.h"
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
		enum {
			WAITING_NEW,
			BUFFER_WRITTEN
		} image_buffer_state = WAITING_NEW;
		std::mutex image_buffer_mutex{};
		std::optional<image_buffer_t> image_buffer{};
		std::mutex v4l2_devices_mutex{};
		std::vector<std::shared_ptr<v4l2_device>> v4l2_devices{};
		std::mutex v4l2_config_mutex{};
		v4l2_config_t v4l2_config{};
		std::optional<v4l2_playback> playback{};
		std::array<char, 16> part_name_str{};
		std::array<char, 16> benchmark_name_short_str{};
		std::array<char, 16> benchmark_score_str{};
		std::mutex input_events_mutex{};
		std::vector<input_event> input_events{};
		exaocbot_usb usb{};
	};

	using image_buffer_state_t = decltype(std::declval<exaocbot_state>().image_buffer_state);
} // namespace exaocbot
