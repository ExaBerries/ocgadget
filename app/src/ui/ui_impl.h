#pragma once
#include "../util/image.h"
#include "ui.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cstdint>
#include <vector>
#include <optional>
#include <memory>

namespace exaocbot {
	struct image_buffer_converter_t {
		virtual ~image_buffer_converter_t() noexcept = default;
		virtual void init() noexcept = 0;
		virtual void load_texture() noexcept = 0;
		virtual void cleanup() noexcept = 0;

		[[nodiscard]] virtual image_buffer_t::image_format_t input_format() noexcept = 0;
	};

	struct renderer_t {
		virtual ~renderer_t() noexcept = default;

		virtual void init_glfw_hints() noexcept = 0;
		virtual void glfw_window_created() noexcept = 0;
		virtual void init() noexcept = 0;
		virtual void loop_pre_imgui() noexcept = 0;
		virtual void loop() noexcept = 0;
		virtual void cleanup() noexcept = 0;
		[[nodiscard]] virtual std::unique_ptr<image_buffer_converter_t> create_image_buffer_converter(image_buffer_t::image_format_t format) noexcept = 0;
	};

	struct ui_state_t {
		exaocbot_state* eob_state = nullptr;
		GLFWwindow* window = nullptr;
		std::unique_ptr<renderer_t> renderer{};
		std::vector<std::array<char, 64>> capture_combo_strings{};
		char* capture_current_combo_item = nullptr;
		std::unique_ptr<image_buffer_converter_t> image_buffer_converter{};
		bool input_capture = false;

		using capture_combo_string_t = decltype(capture_combo_strings)::value_type;
	};

	template <render_api api>
	[[nodiscard]] std::optional<std::unique_ptr<renderer_t>> init_renderer(ui_state_t* state) noexcept;
} // namespace exaocbot
