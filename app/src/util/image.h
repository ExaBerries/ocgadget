#pragma once
#include <cstdint>
#include <vector>
#include <filesystem>

namespace exaocbot {
	struct rgb_image {
		std::vector<uint8_t> buffer{};
		uint32_t width = 0;
		uint32_t height = 0;

		rgb_image(uint32_t width, uint32_t height) noexcept;
		~rgb_image() noexcept = default;
	};

	void convert_yuyv_to_rgb(const uint8_t* yuyv, rgb_image& dest) noexcept;
	void save_png(const rgb_image& rgb, const std::filesystem::path& path) noexcept;
} // namespace exaocbot
