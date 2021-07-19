#include "exaocbot.h"
#include <cstdio>
#include <cstdlib>

namespace exaocbot {
	[[nodiscard]] std::filesystem::path base_screenshot_folder() noexcept {
		return std::filesystem::path{std::getenv("HOME")} / "Documents" / "screenshots" / "exaocbot";
	}
} // namespace exaocbot
