#include "ui.h"
#include <string>
#include <iostream>
#include <mutex>
#include <cstring>
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace exaocbot {
	static void glfw_error_callback(int error, const char* description) noexcept {
		std::cerr << "glfw error: " << error << '\t' << description << std::endl;
	}

	void create_ui(exaocbot_state& state) noexcept {
		std::vector<std::array<char, 24>> v4l2_combo_strings{};
		char* v4l2_current_combo_item = nullptr;
		using v4l2_combo_string_t = decltype(v4l2_combo_strings)::value_type;

		glfwSetErrorCallback(glfw_error_callback);

		if (!glfwInit()) {
			std::cerr << "could not initalize glfw" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

		GLFWwindow* window = glfwCreateWindow(1920, 1080, "exaocbot", NULL, NULL);
		if (window == nullptr) {
			std::cerr << "could not create glfw window" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);

		if (glewInit() != GLEW_OK) {
			std::cerr << "could not initalize glew" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 130");

		GLfloat vertices[] = {-1, -1, 0,
							3, -1, 0,
							-1, 3, 0};

		glEnable(GL_TEXTURE_2D);
		GLuint tex;
		glGenTextures(1, &tex);
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, state.image_buffer.width, state.image_buffer.height, 0, GL_RGB, GL_UNSIGNED_BYTE, state.image_buffer.buffer.data());
		glGenerateMipmap(GL_TEXTURE_2D);

		GLuint vert = glCreateShader(GL_VERTEX_SHADER);
		const std::string vert_str = "#version 130\nin vec3 pos;out vec2 uv;void main() {gl_Position=vec4(pos, 1.0);uv = vec2((pos.x + 1.0) / 2.0, (1.0 - pos.y) / 2.0);}";
		GLint vert_len = vert_str.size();
		auto vert_str_ptr = vert_str.data();
		glShaderSource(vert, 1, &vert_str_ptr, &vert_len);
		glCompileShader(vert);

		GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
		const std::string frag_str = "#version 130\nin vec2 uv;uniform sampler2D tex;out vec4 col;void main() {col = texture(tex, uv);}";
		GLint frag_len = frag_str.size();
		auto frag_str_ptr = frag_str.data();
		glShaderSource(frag, 1, &frag_str_ptr, &frag_len);
		glCompileShader(frag);

		GLuint program = glCreateProgram();
		glAttachShader(program, vert);
		glAttachShader(program, frag);
		glLinkProgram(program);
		glValidateProgram(program);

		GLuint vertexbuffer;
		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			{
				ImGui::Begin("screenshot");

				{
					std::scoped_lock devices_lock{state.v4l2_devices_mutex};
					v4l2_combo_strings = {};
					v4l2_combo_strings.reserve(state.v4l2_devices.size() + 1);
					if (state.v4l2_devices.size() == 0) {
						auto& arr = v4l2_combo_strings.emplace_back();
						std::memcpy(arr.data(), "no device", 10);
					} else {
						for (const auto& device : state.v4l2_devices) {
							auto& arr = v4l2_combo_strings.emplace_back();
							std::memcpy(arr.data(), device->name.data(), device->name.size());
						}
					}

					if (ImGui::BeginCombo("##combo", v4l2_current_combo_item)) {
						for (auto& combo_item : v4l2_combo_strings) {
							bool is_selected = (v4l2_current_combo_item == combo_item.data());
							if (ImGui::Selectable(combo_item.data(), is_selected)) {
								v4l2_current_combo_item = combo_item.data();
							}
							if (is_selected) {
								ImGui::SetItemDefaultFocus();
							}
						}
						ImGui::EndCombo();
					}

					if (state.v4l2_devices.size() > 0 && v4l2_current_combo_item != nullptr) {
						std::scoped_lock v4l2_config_lock{state.v4l2_config_mutex};
						uint32_t i = std::distance(v4l2_combo_strings.data(), reinterpret_cast<v4l2_combo_string_t*>(v4l2_current_combo_item));
						auto& device = state.v4l2_devices[i];
						if (state.v4l2_config.current_v4l2_device != nullptr) {
							if (state.v4l2_config.current_v4l2_device.get() != device.get()) {
								state.v4l2_config.current_v4l2_device = device;
								state.v4l2_config.dirty = true;
							}
						} else {
							state.v4l2_config.current_v4l2_device = device;
							state.v4l2_config.dirty = true;
						}
					}
				}

				ImGui::InputText("##partname", state.part_name_str.data(), state.part_name_str.size());
				ImGui::InputText("##benchmark", state.benchmark_name_short_str.data(), state.benchmark_name_short_str.size());
				ImGui::InputText("##score", state.benchmark_score_str.data(), state.benchmark_score_str.size());

				if (ImGui::Button("Screenshot")) {
					std::scoped_lock lock{state.image_buffer_mutex};
					save_png(state.image_buffer, "/home/exaberries/Documents/screenshots/exaocbot/" + std::string(state.part_name_str.data()) + "-" + std::string(state.benchmark_name_short_str.data()) + "-" + std::string(state.benchmark_score_str.data()) + ".png");
				}

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::End();
			}

			ImGui::Render();
			int display_w, display_h;
			glfwGetFramebufferSize(window, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			glClearColor(0.64f, 1.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			if (state.playback != std::nullopt) {
				glUseProgram(program);
				glActiveTexture(GL_TEXTURE0 + 2);
				glBindTexture(GL_TEXTURE_2D, tex);
				if (state.image_buffer_state == image_buffer_state_t::BUFFER_WRITTEN && state.image_buffer_mutex.try_lock()) {
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, state.image_buffer.width, state.image_buffer.height, 0, GL_RGB, GL_UNSIGNED_BYTE, state.image_buffer.buffer.data());
					glGenerateMipmap(GL_TEXTURE_2D);
					state.image_buffer_state = image_buffer_state_t::WAITING_NEW;
					state.image_buffer_mutex.unlock();
				}
				glUniform1i(glGetUniformLocation(program, "tex"), 2);

				glDrawArrays(GL_TRIANGLES, 0, 3);
			}

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window);
		}

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(window);
		glfwTerminate();
	}
} // namespace exaocbot
