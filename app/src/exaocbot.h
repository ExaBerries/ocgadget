#pragma once
#include "util/image.h"
#include "v4l2.h"
#include <typeinfo>
#include <utility>
#include <mutex>
#include <vector>
#include <array>

namespace exaocbot {
	struct exaocbot_state {
		enum {
			WAITING_NEW,
			BUFFER_WRITTEN
		} image_buffer_state = WAITING_NEW;
		rgb_image image_buffer{1920, 1080};
		std::mutex image_buffer_mutex{};
		v4l2_device device{};
		std::array<char, 16> part_name_str{};
		std::array<char, 8> benchmark_name_short_str{};
		std::array<char, 16> benchmark_score_str{};
	};

	using image_buffer_state_t = decltype(std::declval<exaocbot_state>().image_buffer_state);
} // namespace exaocbot
