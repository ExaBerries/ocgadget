#include "ocgadget.h"
#include "capture/capture.h"
#include "ui/ui.h"
#include <thread>
#include <cstring>
#include <iostream>

namespace ocgadget {
} // namespace ocgadget

int main(int argc, char* argv[]) {
	using namespace ocgadget;

	ocgadget_state state{};
	std::memcpy(state.part_name_str.data(), "part name", 10);
	std::memcpy(state.benchmark_name_short_str.data(), "benchmark", 10);
	std::memcpy(state.benchmark_score_str.data(), "score", 6);

	bool running = true;
	
	std::thread capture_thread([&]() noexcept -> void {
		capture_init(state.capture_state);
		while (running) {
			capture_loop(state.capture_state);
		}
		capture_cleanup(state.capture_state);
	});

	std::thread controller_thread([&]() noexcept -> void {
		msg_init(state.msg_state);
		while (running) {
			msg_loop(state.msg_state);
		}
		msg_cleanup(state.msg_state);
	});

	create_ui(state);

	running = false;
	capture_thread.join();
	controller_thread.join();
}
