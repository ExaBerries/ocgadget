#pragma once
#include "capture.h"
#include <unordered_map>
#define COBJMACROS
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <atlstr.h>

namespace ocgadget {
	struct mf_device {
		std::string name{};
		std::string symlink{};
	};

	struct media_foundation : public capture_api_impl {\
		std::unordered_map<capture_device*, mf_device> mf_devices{};

		media_foundation() noexcept;
		~media_foundation() noexcept;

		[[nodiscard]] std::unique_ptr<capture_playback> create_capture_playback(capture_state_t& state) noexcept override;
		void find_capture_devices(capture_state_t& state) noexcept override;
	};
} // namespace ocgadget

