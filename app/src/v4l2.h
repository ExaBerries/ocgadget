#pragma once
#include "util/image.h"
#include <cstdint>
extern "C" {
	#include <linux/videodev2.h>
}

namespace exaocbot {
	struct v4l2_device {
		int fd = 0;
		uint8_t* buffer_start = nullptr;
		v4l2_buffer bufferinfo;
		uint32_t width = 0;
		uint32_t height = 0;
		bool capturing = false;

		~v4l2_device() noexcept;

		void load_texture_data(rgb_image& image) noexcept;
	};

	void initalize_v4l2_device(v4l2_device& device, uint32_t id, uint32_t width, uint32_t height) noexcept;
} // namespace exaocbot
