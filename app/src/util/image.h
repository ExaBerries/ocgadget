#pragma once
#include <cstdint>
#include <vector>
#include <filesystem>

namespace ocgadget {
	struct image_buffer_t {
		enum {
			RGB,
			RGBA,
			YUYV_422
		} format = RGB;
		std::vector<uint8_t> buffer{};
		uint32_t width = 0;
		uint32_t height = 0;

		using image_format_t = decltype(format);

		image_buffer_t(image_format_t format, uint32_t width, uint32_t height) noexcept;
		~image_buffer_t() noexcept = default;
	};

	void convert_yuyv_422_to_rgb(const image_buffer_t& image, uint8_t* output) noexcept;
	void convert_yuyv_422_to_rgba(const image_buffer_t& image, uint8_t* output) noexcept;
	void save_png(const image_buffer_t& image, const std::filesystem::path& path) noexcept;
} // namespace ocgadget
