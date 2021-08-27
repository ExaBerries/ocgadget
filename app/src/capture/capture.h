#pragma once
#include "../util/image.h"
#include <optional>
#include <vector>
#include <memory>
#include <string>
#include <mutex>

namespace ocgadget {
	struct capture_api_impl;

	struct capture_device {
		std::string name{};
		capture_api_impl* api = nullptr;
	};

	struct capture_img_config {
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t frame_rate = 0;
		image_buffer_t::image_format_t image_format = image_buffer_t::RGB;
	};

	struct capture_config_t {
		bool dirty = false;
		std::shared_ptr<capture_device> current_device{};
		capture_img_config img_config{};
	};

	struct capture_playback {
		std::shared_ptr<capture_device> device = nullptr;
		uint32_t width = 0;
		uint32_t height = 0;
		image_buffer_t::image_format_t image_format = image_buffer_t::RGB;
		bool capturing = false;

		virtual ~capture_playback() noexcept = default;

		virtual void load_texture_data(image_buffer_t& image) noexcept = 0;
		virtual void start_streaming() noexcept = 0;
		virtual void stop_streaming() noexcept = 0;
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
		std::vector<std::shared_ptr<capture_api_impl>> capture_apis{};
		std::mutex capture_config_mutex{};
		capture_config_t capture_config{};
		std::unique_ptr<capture_playback> playback{};
	};

	using image_buffer_state_t = decltype(std::declval<capture_state_t>().image_buffer_state);

	struct capture_api_impl {
		virtual ~capture_api_impl() noexcept = default;

		[[nodiscard]] virtual std::unique_ptr<capture_playback> create_capture_playback(capture_state_t& state) noexcept = 0;
		virtual void find_capture_devices(capture_state_t& state) noexcept = 0;
	};

	void capture_init(capture_state_t& state) noexcept;
	void capture_apis_init(capture_state_t& state) noexcept;
	void capture_loop(capture_state_t& state) noexcept;
	void capture_cleanup(capture_state_t& state) noexcept;
} // namespace ocgadget
