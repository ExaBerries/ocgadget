#pragma once
#include "../util/image.h"
#include <optional>
#include <mutex>

namespace exaocbot {
	enum class capture_api_t {
		V4L2
	};

	struct capture_device {
		std::string name{};
	};

	struct capture_config_t {
		bool dirty = false;
		std::shared_ptr<capture_device> current_device{};
		image_buffer_t::image_format_t image_format = image_buffer_t::RGB;
		uint32_t width = 1920;
		uint32_t height = 1080;
		uint32_t frame_rate = 60;
	};

	struct capture_playback {
		std::shared_ptr<capture_device> device = nullptr;
		int32_t width = 0;
		uint32_t height = 0;
		image_buffer_t::image_format_t image_format = image_buffer_t::RGB;
		bool capturing = false;

		void load_texture_data(image_buffer_t& image) noexcept;
		void start_streaming() noexcept;
		void stop_streaming() noexcept;
	};

	struct capture_state_t {
		enum {
			WAITING_NEW,
			BUFFER_WRITTEN
		} image_buffer_state = WAITING_NEW;
		std::mutex image_buffer_mutex{};
		std::optional<image_buffer_t> image_buffer{};
		std::mutex capture_devices_mutex{};
		std::vector<std::shared_ptr<capture_device>> capture_devices{};
		std::mutex capture_config_mutex{};
		capture_config_t capture_config{};
		std::optional<capture_playback> playback{};
	};

	using image_buffer_state_t = decltype(std::declval<capture_state_t>().image_buffer_state);

	void capture_loop(capture_state_t& state) noexcept;
} // namespace exaocbot
