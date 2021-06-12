#include "exaocbot.h"
#include "v4l2.h"
#include "ui.h"
#include <thread>
#include <cstring>
#include <unistd.h>
#include <iostream>

namespace exaocbot {
} // namespace exaocbot

int main(int argc, char* argv[]) {
	using namespace exaocbot;

	exaocbot_state state{};
	std::memcpy(state.part_name_str.data(), "part name", 10);
	std::memcpy(state.benchmark_name_short_str.data(), "benchmark", 10);
	std::memcpy(state.benchmark_score_str.data(), "score", 6);

	bool running = true;
	std::thread v4l2_thread([&]() noexcept -> void {
		while (running) {
			find_v4l2_devices(state.v4l2_devices, state.v4l2_devices_mutex, state.v4l2_config, state.v4l2_config_mutex);

			if (state.v4l2_config.dirty) {
				if (state.playback != std::nullopt) {
					state.playback->stop_streaming();
				}

				if (state.v4l2_config.current_v4l2_device != nullptr) {
					std::cout << "created playback" << std::endl;
					state.playback = create_v4l2_playback(state.v4l2_config.current_v4l2_device, state.v4l2_config.width, state.v4l2_config.height);
					state.playback->start_streaming();
				} else {
					state.playback = {};
				}

				std::scoped_lock v4l2_config_lock{};
				state.v4l2_config.dirty = false;
			}

			if (state.playback != std::nullopt) {
				if (state.image_buffer_state != image_buffer_state_t::WAITING_NEW) continue;
				std::scoped_lock lock{state.image_buffer_mutex};
				state.playback->load_texture_data(state.image_buffer);
				state.image_buffer_state = image_buffer_state_t::BUFFER_WRITTEN;
				usleep(16129);
			} else {
				usleep(64000);
			}

		}
	});

	create_ui(state);

	running = false;
	v4l2_thread.join();
}
