#include "capture.h"
#include <iostream>
#include <unistd.h>

namespace exaocbot {
	void capture_playback::load_texture_data(image_buffer_t& image) noexcept {

	}
	
	void capture_playback::start_streaming() noexcept{

	}

	void capture_playback::stop_streaming() noexcept {

	}

	static void find_capture_devices(std::vector<std::shared_ptr<capture_device>>& devices, std::mutex& devices_mutex, capture_config_t& capture_config, std::mutex& capture_config_mutex) noexcept {

	}
	
	[[nodiscard]] static capture_playback create_capture_playback(std::shared_ptr<capture_device> device, uint32_t width, uint32_t height) noexcept {
		return capture_playback{};
	}

	void capture_loop(capture_state_t& state) noexcept {
		find_capture_devices(state.capture_devices, state.capture_devices_mutex, state.capture_config, state.capture_config_mutex);

		if (state.capture_config.dirty) {
			if (state.playback != std::nullopt) {
				state.playback->stop_streaming();
			}

			if (state.capture_config.current_device != nullptr) {
				state.playback = create_capture_playback(state.capture_config.current_device, state.capture_config.width, state.capture_config.height);
				state.capture_config.image_format = state.playback->image_format;
				{
					std::scoped_lock img_bfr_lock{state.image_buffer_mutex};
					if (state.image_buffer != std::nullopt) {
						state.image_buffer = {};
					}
					state.image_buffer = {state.capture_config.image_format, state.capture_config.width, state.capture_config.height};
				}
				std::cout << "created playback" << std::endl;
				state.playback->start_streaming();
			} else {
				state.playback = {};
			}

			std::scoped_lock capture_config_lock{};
			state.capture_config.dirty = false;
		}

		if (state.playback != std::nullopt && state.image_buffer != std::nullopt) {
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
} // namespace exaocbot
