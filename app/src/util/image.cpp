#include "image.h"
#include <png++/png.hpp>

namespace exaocbot {
	image_buffer_t::image_buffer_t(image_buffer_t::image_format_t format, uint32_t width, uint32_t height) noexcept : format(format), width(width), height(height) {
		switch (format) {
			case image_buffer_t::RGB:
				buffer.resize(width * height * 3u);
				break;
			case image_buffer_t::RGBA:
				buffer.resize(width * height * 4u);
				break;
			case image_buffer_t::YUYV_422:
				buffer.resize(width * height * 2u);
				break;
		}
	}

	void convert_yuyv_422_to_rgb(const image_buffer_t& image, uint8_t* output) noexcept {
		auto* yuyv = image.buffer.data();
		auto* rgb = output;
		auto& width = image.width;
		auto& height = image.height;

		int32_t y;
		int32_t cr;
		int32_t cb;

		double r;
		double g;
		double b;

		uint32_t i = 0;
		uint32_t j = 0;

		while (i < width * height * 3u) {
			y = yuyv[j];
			cb = yuyv[j+1];
			cr = yuyv[j+3];

			r = y + (1.4065 * (cr - 128));
			g = y - (0.3455 * (cb - 128)) - (0.7169 * (cr - 128));
			b = y + (1.7790 * (cb - 128));

			if (r < 0) r = 0;
			else if (r > 255) r = 255;
			if (g < 0) g = 0;
			else if (g > 255) g = 255;
			if (b < 0) b = 0;
			else if (b > 255) b = 255;

			rgb[i] = (unsigned char)r;
			rgb[i+1] = (unsigned char)g;
			rgb[i+2] = (unsigned char)b;

			y = yuyv[j+2];
			cb = yuyv[j+1];
			cr = yuyv[j+3];

			r = y + (1.4065 * (cr - 128));
			g = y - (0.3455 * (cb - 128)) - (0.7169 * (cr - 128));
			b = y + (1.7790 * (cb - 128));

			if (r < 0) r = 0;
			else if (r > 255) r = 255;
			if (g < 0) g = 0;
			else if (g > 255) g = 255;
			if (b < 0) b = 0;
			else if (b > 255) b = 255;

			rgb[i+3] = (unsigned char)r;
			rgb[i+4] = (unsigned char)g;
			rgb[i+5] = (unsigned char)b;

			i+=6;
			j+=4;
		}
	}

	void convert_yuyv_422_to_rgba(const image_buffer_t& image, uint8_t* output) noexcept {
		auto* yuyv = image.buffer.data();
		auto* rgb = output;
		auto& width = image.width;
		auto& height = image.height;

		int32_t y;
		int32_t cr;
		int32_t cb;

		double r;
		double g;
		double b;

		uint32_t i = 0;
		uint32_t j = 0;

		while (i < width * height * 3u) {
			y = yuyv[j];
			cb = yuyv[j+1];
			cr = yuyv[j+3];

			r = y + (1.4065 * (cr - 128));
			g = y - (0.3455 * (cb - 128)) - (0.7169 * (cr - 128));
			b = y + (1.7790 * (cb - 128));

			if (r < 0) r = 0;
			else if (r > 255) r = 255;
			if (g < 0) g = 0;
			else if (g > 255) g = 255;
			if (b < 0) b = 0;
			else if (b > 255) b = 255;

			rgb[i] = (unsigned char)r;
			rgb[i+1] = (unsigned char)g;
			rgb[i+2] = (unsigned char)b;

			y = yuyv[j+2];
			cb = yuyv[j+1];
			cr = yuyv[j+3];

			r = y + (1.4065 * (cr - 128));
			g = y - (0.3455 * (cb - 128)) - (0.7169 * (cr - 128));
			b = y + (1.7790 * (cb - 128));

			if (r < 0) r = 0;
			else if (r > 255) r = 255;
			if (g < 0) g = 0;
			else if (g > 255) g = 255;
			if (b < 0) b = 0;
			else if (b > 255) b = 255;

			rgb[i+3] = (unsigned char)r;
			rgb[i+4] = (unsigned char)g;
			rgb[i+5] = (unsigned char)b;

			i+=8;
			j+=4;
		}
	}

	void save_png(const image_buffer_t& image, const std::filesystem::path& path) noexcept {
		auto save_png_image = [&path](const image_buffer_t& rgb) noexcept -> void {
			png::image<png::rgb_pixel> png_image(rgb.width, rgb.height);
			for (uint64_t x = 0; x < rgb.width; x++) {
				for (uint64_t y = 0; y < rgb.height; y++) {
					png_image[y][x].red = rgb.buffer.data()[x * 3 + y * rgb.width * 3 + 0];
					png_image[y][x].green = rgb.buffer.data()[x * 3 + y * rgb.width * 3 + 1];
					png_image[y][x].blue = rgb.buffer.data()[x * 3 + y * rgb.width * 3 + 2];
				}
			}
			png_image.write(path.c_str());
		};

		if (image.format == image_buffer_t::RGB) {
			save_png_image(image);
		} else if (image.format == image_buffer_t::RGBA) {
			image_buffer_t rgb{image_buffer_t::RGB, image.width, image.height};
			for (uint64_t x = 0; x < image.width; x++) {
				for (uint64_t y = 0; y < image.height; y++) {
					rgb.buffer.data()[x * 3 + y * rgb.width * 3 + 0] = image.buffer.data()[x * 4 + y * image.width * 4 + 0];
					rgb.buffer.data()[x * 3 + y * rgb.width * 3 + 1] = image.buffer.data()[x * 4 + y * image.width * 4 + 1];
					rgb.buffer.data()[x * 3 + y * rgb.width * 3 + 2] = image.buffer.data()[x * 4 + y * image.width * 4 + 2];
				}
			}
			save_png_image(rgb);
		} else if (image.format == image_buffer_t::YUYV_422) {
			image_buffer_t rgb{image_buffer_t::RGB, image.width, image.height};
			convert_yuyv_422_to_rgb(image, rgb.buffer.data());
			save_png_image(rgb);
		}
	}
} // namespace exaocbot
