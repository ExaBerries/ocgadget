#pragma once
#include "util/image.h"
#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <mutex>
extern "C" {
	#include <linux/videodev2.h>
}

namespace exaocbot {
	struct v4l2_file_descriptor {
		int fd = 0;

		v4l2_file_descriptor(const char* path) noexcept;
		~v4l2_file_descriptor() noexcept;
	};

	struct v4l2_device {
		std::string path{};
		std::string name{};
		std::string bus_info{};
	};

	struct v4l2_playback {
		std::shared_ptr<v4l2_device> device = nullptr;
		std::shared_ptr<v4l2_file_descriptor> fd{};
		uint8_t* buffer_start = nullptr;
		v4l2_buffer bufferinfo;
		uint32_t width = 0;
		uint32_t height = 0;
		bool capturing = false;

		void load_texture_data(rgb_image& image) noexcept;
		void start_streaming() noexcept;
		void stop_streaming() noexcept;
	};

	struct v4l2_config_t {
		bool dirty = false;
		std::shared_ptr<v4l2_device> current_v4l2_device{};
		uint32_t width = 1920;
		uint32_t height = 1080;
		uint32_t frame_rate = 60;
	};

	void find_v4l2_devices(std::vector<std::shared_ptr<v4l2_device>>& devices, std::mutex& devices_mutex, v4l2_config_t& v4l2_config, std::mutex& v4l2_config_mutex) noexcept;
	[[nodiscard]] v4l2_playback create_v4l2_playback(std::shared_ptr<v4l2_device> device, uint32_t width, uint32_t height) noexcept;
} // namespace exaocbot
