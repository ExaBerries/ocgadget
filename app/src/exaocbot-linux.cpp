#include "exaocbot.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

namespace exaocbot {
	[[nodiscard]] std::filesystem::path base_screenshot_folder() noexcept {
		return std::filesystem::path{getpwuid(getuid())->pw_dir} / "Documents" / "screenshots" / "exaocbot";
	}
} // namespace exaocbot
