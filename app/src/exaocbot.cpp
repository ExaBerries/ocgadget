#include "exaocbot.h"
#include <iostream>

namespace exaocbot {
	void save_screenshot(exaocbot_state& state) noexcept {
		std::scoped_lock lock{state.capture_state.image_buffer_mutex};
		if (state.part_name_str.empty() && state.benchmark_name_short_str.empty() && state.benchmark_score_str.empty()) return;
		std::filesystem::path dir_path = base_screenshot_folder() / state.part_name_str.data() / state.benchmark_name_short_str.data();
		std::filesystem::create_directories(dir_path);
		std::filesystem::path png_path = dir_path / (std::string(state.benchmark_score_str.data()) + ".png");
		save_png(*state.capture_state.image_buffer, png_path);
	}
} // namespace exaocbot
