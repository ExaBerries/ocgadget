#include "msg.h"

namespace ocgadget {
	void msg_init(msg_state_t& state) noexcept {
		msg_init_impl(state);
		state.impl->init(state);
	}

	void msg_loop(msg_state_t& state) noexcept {
		state.impl->loop(state);
	}

	void msg_cleanup(msg_state_t& state) noexcept {
		state.impl->cleanup(state);
	}
} // namespace ocgadget
