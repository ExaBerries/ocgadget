#include "msg.h"
extern "C" {
	#include <unistd.h>
	#include <fcntl.h>
	#include <termio.h>
	#include <io/ioctl.h>
}

namespace exaocbot {
	struct msg_state_linux : public msg_state_impl {

		virtual ~msg_state_linux() noexcept = default;

		void init(msg_state_t& state) noexcept override {
		}

		void loop(msg_state_t& state) noexcept override {
			usleep(8192);
		}

		void cleanup(msg_state_t& state) noexcept override {
		}
	};

	void msg_init_impl(msg_state_t& state) noexcept {
		state.impl = std::make_unique<msg_state_linux>();
	}
} // namespace exaocbot
