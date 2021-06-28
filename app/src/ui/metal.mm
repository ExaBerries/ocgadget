#include "ui_impl.h"

namespace exaocbot {
	template <>
	[[nodiscard]] std::optional<std::unique_ptr<renderer>> init_renderer<render_api::METAL>(ui_state_t* state) noexcept{
		return {};
	}
} // namespace exaocbot
