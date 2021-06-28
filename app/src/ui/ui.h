#pragma once
#include "../exaocbot.h"

namespace exaocbot {
	enum class render_api {
		OPENGL,
		METAL,
		VULKAN
	};

	void create_ui(exaocbot_state& state) noexcept;
} // namespace exaocbot
