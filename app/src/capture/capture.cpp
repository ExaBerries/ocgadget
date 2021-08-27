#include "capture.h"
#include <iostream>
#include <algorithm>
#include <unistd.h>

namespace ocgadget {
	static void find_capture_devices(capture_state_t& state) noexcept {
		for (auto& api_impl : state.capture_apis) {
			api_impl->find_capture_devices(state);
		}

		auto itr = std::find(state.capture_devices.begin(), state.capture_devices.end(), state.capture_config.current_device);

		if (itr == state.capture_devices.end()) {
			std::scoped_lock config_lock{state.capture_config_mutex};
			state.capture_config.current_device = {};
			state.capture_config.dirty = true;
		}
	}
	
	[[nodiscard]] static std::unique_ptr<capture_playback> create_capture_playback(capture_state_t& state) noexcept {
		return state.capture_config.current_device->api->create_capture_playback(state);
	}

	void capture_init(capture_state_t& state) noexcept {
		state.capture_config.img_config.width = 1920;
		state.capture_config.img_config.height = 1080;
		capture_apis_init(state);
	}

	void capture_loop(capture_state_t& state) noexcept {
		find_capture_devices(state);

		if (state.capture_config.dirty) {
			if (state.playback != nullptr) {
				state.playback->stop_streaming();
			}

			if (state.capture_config.current_device != nullptr) {
				state.playback = create_capture_playback(state);
				state.capture_config.img_config.image_format = state.playback->image_format;
				{
					std::scoped_lock img_bfr_lock{state.image_buffer_mutex};
					state.image_buffer = {state.capture_config.img_config.image_format, state.capture_config.img_config.width, state.capture_config.img_config.height};
				}
				std::cout << "created playback" << std::endl;
				state.playback->start_streaming();
			} else {
				state.playback = {};
			}

			std::scoped_lock capture_config_lock{};
			state.capture_config.dirty = false;
		}

		if (state.playback != nullptr && state.image_buffer != std::nullopt) {
			if (state.image_buffer_state != image_buffer_state_t::WAITING_NEW) {
				usleep(200);
				return;
			}
			std::scoped_lock lock{state.image_buffer_mutex};
			state.playback->load_texture_data(*state.image_buffer);
			state.image_buffer_state = image_buffer_state_t::BUFFER_WRITTEN;
			usleep(12000);
		} else {
			usleep(64000);
		}
	}

	void capture_cleanup(capture_state_t& state) noexcept {
	}
} // namespace ocgadget
