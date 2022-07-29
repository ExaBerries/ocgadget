#include "ocgadget.h"
#include <cstdio>
#include <cstdlib>

namespace ocgadget {
	[[nodiscard]] std::filesystem::path base_screenshot_folder() noexcept {
		return std::filesystem::path{std::getenv("USERPROFILE")} / "Documents" / "screenshots" / "ocgadget";
	}
} // namespace ocgadget
