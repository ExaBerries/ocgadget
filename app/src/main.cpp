#include "exaocbot.h"
#include "v4l2.h"
#include "ui.h"
#include <thread>
#include <cstring>

namespace exaocbot {
} // namespace exaocbot

int main(int argc, char* argv[]) {
	using namespace exaocbot;

	exaocbot_state state{};
	initalize_v4l2_device(state.device, 0, state.image_buffer.width, state.image_buffer.height);
	std::memcpy(state.part_name_str.data(), "part name", 10);
	std::memcpy(state.benchmark_name_short_str.data(), "benchmark", 10);
	std::memcpy(state.benchmark_score_str.data(), "score", 6);

	bool running = true;
	std::thread v4l2_thread([&]() noexcept -> void {
		while (running) {
			if (state.image_buffer_state != image_buffer_state_t::WAITING_NEW) continue;

			std::scoped_lock lock{state.image_buffer_mutex};
			state.device.load_texture_data(state.image_buffer);

			state.image_buffer_state = image_buffer_state_t::BUFFER_WRITTEN;
		}
	});

	create_ui(state);

	running = false;
	v4l2_thread.join();
}
