#pragma once
#include "../ocgadget.h"

namespace ocgadget {
	enum class render_api {
		OPENGL,
		METAL,
		VULKAN
	};

	void create_ui(ocgadget_state& state) noexcept;
} // namespace ocgadget
