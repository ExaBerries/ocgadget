#pragma once
#include "capture.h"
#include <unordered_map>
#import <AVFoundation/AVFoundation.h>

namespace ocgadget {
	struct avfoundation_capture_device {
		std::string name{};
		AVCaptureDevice* avf_device = nullptr;
	};

	struct avfoundation : public capture_api_impl {
		AVCaptureDeviceDiscoverySession* discovery_session = nullptr;
		std::unordered_map<capture_device*, avfoundation_capture_device> avfoundation_devices{};

		avfoundation() noexcept;

		[[nodiscard]] std::unique_ptr<capture_playback> create_capture_playback(capture_state_t& state) noexcept override;
		void find_capture_devices(capture_state_t& state) noexcept override;
	};
} // namespace ocgadget
