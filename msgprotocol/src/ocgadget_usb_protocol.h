#pragma once
#include <stdint.h>

namespace ocgadget {
	constexpr uint32_t USB_PROTOCOL_REVISION = 0x01;

	struct msg_heartbeat {
		constexpr static uint32_t MSG_ID = 0x01;
		constexpr static bool ACKNOWLEDGE = false;
		uint64_t idempotency = 0;
	};

	struct msg_acknowledge {
		constexpr static uint32_t MSG_ID = 0x2;
		constexpr static bool ACKNOWLEDGE = false;
		uint64_t idempotency = 0;
		uint64_t msg_recived_idempotency = 0;
	};

	struct msg_discover {
		constexpr static uint32_t MSG_ID = 0x03;
		constexpr static bool ACKNOWLEDGE = true;
		uint64_t idempotency = 0;
		uint32_t CONTROLLER_PROTOCOL_REVISION = 0;
	};

	struct msg_capabilities {
		constexpr static uint32_t MSG_ID = 0x04;
		constexpr static bool ACKNOWLEDGE = true;
	};
} // namespace ocgadget
