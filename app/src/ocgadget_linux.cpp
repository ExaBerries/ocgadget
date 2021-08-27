#include "ocgadget.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

namespace ocgadget {
	[[nodiscard]] std::filesystem::path base_screenshot_folder() noexcept {
		return std::filesystem::path{getpwuid(getuid())->pw_dir} / "Documents" / "screenshots" / "ocgadget";
	}
} // namespace ocgadget
