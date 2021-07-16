#pragma once
#include <exaocbot_usb_protocol.h>
#include <variant>
#include <vector>
#include <memory>

namespace exaocbot {
	using message_t = std::variant<msg_heartbeat, msg_discover>;

	struct msg_state_impl;

	struct msg_state_t {
		enum {
			DISCONNECTED,
			CONNECTING,
			CONNECTED
		} connection_state = DISCONNECTED;
		std::unique_ptr<msg_state_impl> impl{};
		std::vector<message_t> message_inbox{};
		std::vector<message_t> message_outbox{};
		uint64_t idempotency = 0;
	};

	struct msg_state_impl {
		virtual ~msg_state_impl() noexcept = default;

		virtual void init(msg_state_t& state) noexcept = 0;
		virtual void loop(msg_state_t& state) noexcept = 0;
		virtual void cleanup(msg_state_t& state) noexcept = 0;
	};

	void msg_init(msg_state_t& state) noexcept;
	void msg_init_impl(msg_state_t& state) noexcept;
	void msg_loop(msg_state_t& state) noexcept;
	void msg_cleanup(msg_state_t& state) noexcept;
} // namespace exaocbot
