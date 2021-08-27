#include "test_cap_src.h"
#include <iostream>

namespace ocgadget {
	struct test_cap_src_playback : public capture_playback {
		void load_texture_data(image_buffer_t& image) noexcept {
			for (uint32_t x = 0; x < image.width; x++) {
				for (uint32_t y = 0; y < image.height; y++) {
					bool a = ((x / 32) % 2 == 0);
					bool b = ((y / 32) % 2 == 0);
					image.buffer.data()[(x + y * image.width) * 4 + 0] = 0x00;
					image.buffer.data()[(x + y * image.width) * 4 + 1] = (a && b) ? 0xFF : 0x00;
					image.buffer.data()[(x + y * image.width) * 4 + 2] = 0x00;
					image.buffer.data()[(x + y * image.width) * 4 + 3] = 0XFF;
				}
			}
		}
		
		void start_streaming() noexcept {
		}
		
		void stop_streaming() noexcept {
		}
	};

	[[nodiscard]] std::unique_ptr<capture_playback> test_cap_src::create_capture_playback(capture_state_t& state) noexcept {
		auto new_playback = std::make_unique<test_cap_src_playback>();
		auto& playback = *static_cast<test_cap_src_playback*>(new_playback.get());
		playback.width = state.capture_config.img_config.width;
		playback.height = state.capture_config.img_config.height;
		playback.device = state.capture_config.current_device;
		playback.image_format = image_buffer_t::RGBA;
		return new_playback;
	}

	void test_cap_src::find_capture_devices(capture_state_t& state) noexcept {
		for (auto& capture_device : state.capture_devices) {
			if (capture_device->api == this) {
				return;
			}
		}

		auto& new_capture_device = state.capture_devices.emplace_back(std::make_shared<capture_device>());
		new_capture_device->name = "test capture device";
		new_capture_device->api = this;
	}
} // namespace ocgadget
