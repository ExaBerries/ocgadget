#include "msg.h"
#include <chrono>
#include <thread>
extern "C" {
	#include <unistd.h>
	#include <fcntl.h>
	#include <termio.h>
	#include <sys/ioctl.h>
}

namespace ocgadget {
	struct msg_state_linux : public msg_state_impl {

		virtual ~msg_state_linux() noexcept = default;

		void init(msg_state_t& state) noexcept override {
		}

		void loop(msg_state_t& state) noexcept override {
			std::this_thread::sleep_for(std::chrono::microseconds(8192));
		}

		void cleanup(msg_state_t& state) noexcept override {
		}
	};

	void msg_init_impl(msg_state_t& state) noexcept {
		state.impl = std::make_unique<msg_state_linux>();
	}
} // namespace ocgadget
