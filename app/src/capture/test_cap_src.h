#pragma once
#include "capture.h"

namespace ocgadget {
	struct test_cap_src : public capture_api_impl {
		[[nodiscard]] std::unique_ptr<capture_playback> create_capture_playback(capture_state_t& state) noexcept override;
		void find_capture_devices(capture_state_t& state) noexcept override;
	};
} // namespace ocgadget
