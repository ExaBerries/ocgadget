#include "image.h"
#include <png++/png.hpp>

namespace exaocbot {
	rgb_image::rgb_image(uint32_t width, uint32_t height) noexcept : buffer(width * height * 3), width(width), height(height) {
	}

	void convert_yuyv_to_rgb(const uint8_t* yuyv, rgb_image& dest) noexcept {
		auto* rgb = dest.buffer.data();
		auto& width = dest.width;
		auto& height = dest.height;

		int y;
		int cr;
		int cb;

		double r;
		double g;
		double b;

		int i = 0, j = 0;
		while (i < width * height * 3) {
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

	void save_png(const rgb_image& rgb, const std::filesystem::path& path) noexcept {
		png::image<png::rgb_pixel> image(rgb.width, rgb.height);
		for (int x = 0; x < rgb.width; x++) {
			for (int y = 0; y < rgb.height; y++) {
				image[y][x].red = rgb.buffer.data()[x * 3 + y * rgb.width * 3 + 0];
				image[y][x].green = rgb.buffer.data()[x * 3 + y * rgb.width * 3 + 1];
				image[y][x].blue = rgb.buffer.data()[x * 3 + y * rgb.width * 3 + 2];
			}
		}
		image.write(path.c_str());
	}
} // namespace exaocbot
