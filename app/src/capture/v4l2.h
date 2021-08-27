#pragma once
#include "capture.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace ocgadget {
	struct v4l2_capture_device {
		std::string path{};
		std::string name{};
		std::string bus_info{};
	};

	struct v4l2 : public capture_api_impl {
		std::unordered_map<capture_device*, v4l2_capture_device> v4l2_devices{};

		[[nodiscard]] std::unique_ptr<capture_playback> create_capture_playback(capture_state_t& state) noexcept override;
		void find_capture_devices(capture_state_t& state) noexcept override;
	};
} // namespace ocgadget
