#include "ui.h"
#include <string>
#include <iostream>
#include <mutex>
#include <cstring>
#include <memory>
#include <utility>
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>

namespace exaocbot {
	static void glfw_error_callback(int error, const char* description) noexcept {
		std::cerr << "glfw error: " << error << '\t' << description << std::endl;
	}

	struct image_buffer_converter_t;

	struct ui_state_t {
		exaocbot_state* eob_state = nullptr;
		std::vector<std::array<char, 64>> v4l2_combo_strings{};
		char* v4l2_current_combo_item = nullptr;
		std::unique_ptr<image_buffer_converter_t> image_buffer_converter{};

		using v4l2_combo_string_t = decltype(v4l2_combo_strings)::value_type;
	};

	struct ui_gl_state_t {
		GLFWwindow* window = nullptr;

		GLuint v4l2_texture = 0;
		GLuint v4l2_render_program = 0;
		GLuint v4l2_vertex_buffer = 0;
	};

	struct image_buffer_converter_t {
		virtual ~image_buffer_converter_t() noexcept = default;
		virtual void init(ui_state_t& ui_state, ui_gl_state_t& gl_state) noexcept = 0;
		virtual void load_texture(ui_state_t& ui_state, ui_gl_state_t& gl_state) noexcept = 0;
		virtual void cleanup(ui_state_t& ui_state, ui_gl_state_t& gl_state) noexcept = 0;

		[[nodiscard]] virtual image_buffer_t::image_format_t input_format() noexcept = 0;
	};

	struct gl_rgb_passthrough_buffer_converter : public image_buffer_converter_t {
		virtual ~gl_rgb_passthrough_buffer_converter() noexcept = default;

		void init(ui_state_t& ui_state, ui_gl_state_t& gl_state) noexcept override {
			glGenTextures(1, &gl_state.v4l2_texture);
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, gl_state.v4l2_texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ui_state.eob_state->image_buffer->width, ui_state.eob_state->image_buffer->height, 0, GL_RGB, GL_UNSIGNED_BYTE, ui_state.eob_state->image_buffer->buffer.data());
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		void load_texture(ui_state_t& ui_state, ui_gl_state_t& gl_state) noexcept override {
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, gl_state.v4l2_texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ui_state.eob_state->image_buffer->width, ui_state.eob_state->image_buffer->height, 0, GL_RGB, GL_UNSIGNED_BYTE, ui_state.eob_state->image_buffer->buffer.data());
			glGenerateMipmap(GL_TEXTURE_2D);
			ui_state.eob_state->image_buffer_state = image_buffer_state_t::WAITING_NEW;
			ui_state.eob_state->image_buffer_mutex.unlock();
		}

		void cleanup(ui_state_t& ui_state, ui_gl_state_t& gl_state) noexcept override {
			glDeleteTextures(1, &gl_state.v4l2_texture);
		}

		[[nodiscard]] image_buffer_t::image_format_t input_format() noexcept override {
			return image_buffer_t::RGB;
		}
	};

	struct cpu_yuyv_422_buffer_converter : public image_buffer_converter_t {
		virtual ~cpu_yuyv_422_buffer_converter() noexcept = default;

		void init(ui_state_t& ui_state, ui_gl_state_t& gl_state) noexcept {
			glGenTextures(1, &gl_state.v4l2_texture);
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, gl_state.v4l2_texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ui_state.eob_state->image_buffer->width, ui_state.eob_state->image_buffer->height, 0, GL_RGB, GL_UNSIGNED_BYTE, ui_state.eob_state->image_buffer->buffer.data());
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		void load_texture(ui_state_t& ui_state, ui_gl_state_t& gl_state) noexcept {
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, gl_state.v4l2_texture);
			std::vector<uint8_t> buffer;
			buffer.resize(ui_state.eob_state->image_buffer->width * ui_state.eob_state->image_buffer->height * 3);
			convert_yuyv_422_to_rgb(*ui_state.eob_state->image_buffer, buffer.data());
			//memset(buffer.data(), 255, buffer.size());
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ui_state.eob_state->image_buffer->width, ui_state.eob_state->image_buffer->height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		void cleanup(ui_state_t& ui_state, ui_gl_state_t& gl_state) noexcept {
			glDeleteTextures(1, &gl_state.v4l2_texture);
		}

		[[nodiscard]] image_buffer_t::image_format_t input_format() noexcept override {
			return image_buffer_t::YUYV_422;
		}
	};

	static std::unique_ptr<image_buffer_converter_t> create_image_buffer_converter(image_buffer_t::image_format_t format, ui_state_t& ui_state, ui_gl_state_t& gl_state) noexcept {
		if (format == image_buffer_t::RGB) {
			return std::make_unique<gl_rgb_passthrough_buffer_converter>();
		} else if (format == image_buffer_t::YUYV_422) {
			return std::make_unique<cpu_yuyv_422_buffer_converter>();
		}
	}

	static void create_glfw_window(ui_state_t& ui_state, ui_gl_state_t& gl_state) noexcept {
		glfwSetErrorCallback(glfw_error_callback);

		if (!glfwInit()) {
			std::cerr << "could not initalize glfw" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

		gl_state.window = glfwCreateWindow(1920, 1080, "exaocbot", NULL, NULL);
		if (gl_state.window == nullptr) {
			std::cerr << "could not create glfw window" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		glfwMakeContextCurrent(gl_state.window);
		glfwSwapInterval(1);

		if (glewInit() != GLEW_OK) {
			std::cerr << "could not initalize glew" << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}

	static void init_imgui(ui_state_t& ui_state, ui_gl_state_t& gl_state) noexcept {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(gl_state.window, true);
		ImGui_ImplOpenGL3_Init("#version 130");
	}

	static void init_gl_ui(ui_state_t& ui_state, ui_gl_state_t& gl_state) noexcept {
		GLfloat vertices[] = {-1, -1, 0,
							3, -1, 0,
							-1, 3, 0};

		glEnable(GL_TEXTURE_2D);

		GLuint vert = glCreateShader(GL_VERTEX_SHADER);
		const std::string vert_str =
R"***(#version 130
in vec3 pos;
out vec2 uv;

void main() {
	gl_Position = vec4(pos, 1.0);
	uv = vec2((pos.x + 1.0) / 2.0, (1.0 - pos.y) / 2.0);
})***";
		GLint vert_len = vert_str.size();
		auto vert_str_ptr = vert_str.data();
		glShaderSource(vert, 1, &vert_str_ptr, &vert_len);
		glCompileShader(vert);

		GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
		const std::string frag_str =
R"***(#version 130
in vec2 uv;
uniform sampler2D tex;
out vec4 col;

void main() {
	col = texture(tex, uv);
})***";
		GLint frag_len = frag_str.size();
		auto frag_str_ptr = frag_str.data();
		glShaderSource(frag, 1, &frag_str_ptr, &frag_len);
		glCompileShader(frag);

		gl_state.v4l2_render_program = glCreateProgram();
		glAttachShader(gl_state.v4l2_render_program, vert);
		glAttachShader(gl_state.v4l2_render_program, frag);
		glLinkProgram(gl_state.v4l2_render_program);
		glValidateProgram(gl_state.v4l2_render_program);

		glGenBuffers(1, &gl_state.v4l2_vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, gl_state.v4l2_vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, gl_state.v4l2_vertex_buffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}

	static void loop_imgui(ui_state_t& ui_state, ui_gl_state_t& gl_state) noexcept {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			ImGui::Begin("screenshot");

			{
				std::scoped_lock devices_lock{ui_state.eob_state->v4l2_devices_mutex};
				ui_state.v4l2_combo_strings = {};
				ui_state.v4l2_combo_strings.reserve(ui_state.eob_state->v4l2_devices.size() + 1);
				if (ui_state.eob_state->v4l2_devices.size() == 0) {
					auto& arr = ui_state.v4l2_combo_strings.emplace_back();
					std::memcpy(arr.data(), "no device", 10);
				} else {
					for (const auto& device : ui_state.eob_state->v4l2_devices) {
						auto& arr = ui_state.v4l2_combo_strings.emplace_back();
						std::memcpy(arr.data(), device->name.data(), std::min(device->name.size(), ui_state_t::v4l2_combo_string_t().size()));
					}
				}

				if (ImGui::BeginCombo("##combo", ui_state.v4l2_current_combo_item)) {
					for (auto& combo_item : ui_state.v4l2_combo_strings) {
						bool is_selected = (ui_state.v4l2_current_combo_item == combo_item.data());
						if (ImGui::Selectable(combo_item.data(), is_selected)) {
							ui_state.v4l2_current_combo_item = combo_item.data();
						}
						if (is_selected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				if (ui_state.eob_state->v4l2_devices.size() > 0 && ui_state.v4l2_current_combo_item != nullptr) {
					std::scoped_lock v4l2_config_lock{ui_state.eob_state->v4l2_config_mutex};
					uint32_t i = std::distance(ui_state.v4l2_combo_strings.data(), reinterpret_cast<ui_state_t::v4l2_combo_string_t*>(ui_state.v4l2_current_combo_item));
					auto& device = ui_state.eob_state->v4l2_devices[i];
					if (ui_state.eob_state->v4l2_config.current_v4l2_device != nullptr) {
						if (ui_state.eob_state->v4l2_config.current_v4l2_device.get() != device.get()) {
							ui_state.eob_state->v4l2_config.current_v4l2_device = device;
							ui_state.eob_state->v4l2_config.dirty = true;
						}
					} else {
						ui_state.eob_state->v4l2_config.current_v4l2_device = device;
						ui_state.eob_state->v4l2_config.dirty = true;
					}
				}
			}

			ImGui::InputText("##partname", ui_state.eob_state->part_name_str.data(), ui_state.eob_state->part_name_str.size());
			ImGui::InputText("##benchmark", ui_state.eob_state->benchmark_name_short_str.data(), ui_state.eob_state->benchmark_name_short_str.size());
			ImGui::InputText("##score", ui_state.eob_state->benchmark_score_str.data(), ui_state.eob_state->benchmark_score_str.size());

			if (ImGui::Button("Screenshot") && ui_state.eob_state->image_buffer != std::nullopt) {
				std::scoped_lock lock{ui_state.eob_state->image_buffer_mutex};
				save_png(*ui_state.eob_state->image_buffer, "/home/exaberries/Documents/screenshots/exaocbot/" + std::string(ui_state.eob_state->part_name_str.data()) + "-" + std::string(ui_state.eob_state->benchmark_name_short_str.data()) + "-" + std::string(ui_state.eob_state->benchmark_score_str.data()) + ".png");
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
	}

	static void loop_gl(ui_state_t& ui_state, ui_gl_state_t gl_state) noexcept {
		ImGui::Render();
		int display_w;
		int display_h;
		glfwGetFramebufferSize(gl_state.window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.64f, 1.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(gl_state.v4l2_render_program);
		if (ui_state.image_buffer_converter != nullptr) {
			if (ui_state.eob_state->image_buffer_state == image_buffer_state_t::BUFFER_WRITTEN && ui_state.eob_state->image_buffer_mutex.try_lock()) {
				ui_state.image_buffer_converter->load_texture(ui_state, gl_state);
				ui_state.eob_state->image_buffer_state = image_buffer_state_t::WAITING_NEW;
				ui_state.eob_state->image_buffer_mutex.unlock();
			}
			glUniform1i(glGetUniformLocation(gl_state.v4l2_render_program, "tex"), 2);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, gl_state.v4l2_vertex_buffer);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	static void cleanup_ui(ui_state_t& ui_state, ui_gl_state_t gl_state) noexcept {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(gl_state.window);
		glfwTerminate();
	}

	void create_ui(exaocbot_state& state) noexcept {
		ui_state_t ui_state;
		ui_state.eob_state = &state;

		ui_gl_state_t gl_state;

		create_glfw_window(ui_state, gl_state);
		init_imgui(ui_state, gl_state);
		init_gl_ui(ui_state, gl_state);

		while (!glfwWindowShouldClose(gl_state.window)) {
			glfwPollEvents();

			if (ui_state.image_buffer_converter == nullptr || ui_state.image_buffer_converter->input_format() != ui_state.eob_state->playback->image_format) {
				if (ui_state.image_buffer_converter != nullptr) {
					ui_state.image_buffer_converter->cleanup(ui_state, gl_state);
					ui_state.image_buffer_converter.reset();
				}
				ui_state.image_buffer_converter = create_image_buffer_converter(ui_state.eob_state->playback->image_format, ui_state, gl_state);
				ui_state.image_buffer_converter->init(ui_state, gl_state);
			}

			loop_imgui(ui_state, gl_state);
			loop_gl(ui_state, gl_state);
			glfwSwapBuffers(gl_state.window);
		}

		cleanup_ui(ui_state, gl_state);
	}
} // namespace exaocbot
