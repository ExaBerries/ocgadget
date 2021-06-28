#pragma once
#include <exaocbot_usb_protocol.h>
#include <variant>
#include <vector>

namespace exaocbot {
	using message_t = std::variant<msg_heartbeat, msg_discover>;

	struct exaocbot_usb {
		enum {
			DISCONNECTED,
			CONNECTED
		} state = DISCONNECTED;
		std::vector<message_t> message_queue{};
		uint64_t idempotency = 0;
	};
} // namespace exaocbot
