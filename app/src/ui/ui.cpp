#include "ui.h"
#include "ui_impl.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <string>
#include <iostream>
#include <mutex>
#include <cstring>
#include <memory>
#include <utility>
#include <chrono>
#include <filesystem>
#include <thread>

namespace exaocbot {
	#ifndef __APPLE__
		template <>
		[[nodiscard]] std::optional<std::unique_ptr<renderer_t>> init_renderer<render_api::METAL>(ui_state_t* state) noexcept {
			return {};
		}
	#endif

	static void glfw_error_callback(int error, const char* description) noexcept {
		std::cerr << "glfw error: " << error << '\t' << description << std::endl;
	}

	struct image_buffer_converter_t;

	struct glfw_usr_ptr {
		ui_state_t* ui_state = nullptr;
	};

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept {
		auto* glfw_user_pointer = reinterpret_cast<glfw_usr_ptr*>(glfwGetWindowUserPointer(window));
		if (key == GLFW_KEY_SCROLL_LOCK && action == GLFW_PRESS && mods & GLFW_MOD_SHIFT) {
			glfw_user_pointer->ui_state->input_capture = !glfw_user_pointer->ui_state->input_capture;
		}
		if (glfw_user_pointer->ui_state->input_capture) {
			std::scoped_lock lock{glfw_user_pointer->ui_state->eob_state->input_events_mutex};
			glfw_user_pointer->ui_state->eob_state->input_events.emplace_back(keyboard_input{
				(action == GLFW_PRESS) ? keyboard_input::DOWN : keyboard_input::UP,
				[&]() noexcept {
					switch (key) {
						case GLFW_KEY_ESCAPE:
							return keyboard_input::ESCAPE;
						case GLFW_KEY_F1:
							return keyboard_input::F1;
						case GLFW_KEY_F2:
							return keyboard_input::F2;
						case GLFW_KEY_F3:
							return keyboard_input::F3;
						case GLFW_KEY_F4:
							return keyboard_input::F4;
						case GLFW_KEY_F5:
							return keyboard_input::F5;
						case GLFW_KEY_F6:
							return keyboard_input::F6;
						case GLFW_KEY_F7:
							return keyboard_input::F7;
						case GLFW_KEY_F8:
							return keyboard_input::F8;
						case GLFW_KEY_F9:
							return keyboard_input::F9;
						case GLFW_KEY_F10:
							return keyboard_input::F10;
						case GLFW_KEY_F11:
							return keyboard_input::F11;
						case GLFW_KEY_F12:
							return keyboard_input::F12;
						case GLFW_KEY_PRINT_SCREEN:
							return keyboard_input::PRINT_SCREEN;
						case GLFW_KEY_SCROLL_LOCK:
							return keyboard_input::SCROLL_LOCK;
						case GLFW_KEY_PAUSE:
							return keyboard_input::PAUSE;
						case GLFW_KEY_GRAVE_ACCENT:
							return keyboard_input::ACUTE;
						case GLFW_KEY_1:
							return keyboard_input::ONE;
						case GLFW_KEY_2:
							return keyboard_input::TWO;
						case GLFW_KEY_3:
							return keyboard_input::THREE;
						case GLFW_KEY_4:
							return keyboard_input::FOUR;
						case GLFW_KEY_5:
							return keyboard_input::FIVE;
						case GLFW_KEY_6:
							return keyboard_input::SIX;
						case GLFW_KEY_7:
							return keyboard_input::SEVEN;
						case GLFW_KEY_8:
							return keyboard_input::EIGHT;
						case GLFW_KEY_9:
							return keyboard_input::NINE;
						case GLFW_KEY_0:
							return keyboard_input::ZERO;
						case GLFW_KEY_MINUS:
							return keyboard_input::MINUS;
						case GLFW_KEY_EQUAL:
							return keyboard_input::EQUALS;
						case GLFW_KEY_BACKSPACE:
							return keyboard_input::BACKSLASH;
						case GLFW_KEY_INSERT:
							return keyboard_input::INSERT;
						case GLFW_KEY_HOME:
							return keyboard_input::HOME;
						case GLFW_KEY_PAGE_UP:
							return keyboard_input::PAGE_UP;
						case GLFW_KEY_NUM_LOCK:
							return keyboard_input::NUM_LOCK;
						case GLFW_KEY_KP_DIVIDE:
							return keyboard_input::NUMPAD_SLASH;
						case GLFW_KEY_KP_MULTIPLY:
							return keyboard_input::NUMPAD_ASTERISK;
						case GLFW_KEY_KP_SUBTRACT:
							return keyboard_input::NUMPAD_MINUS;
						case GLFW_KEY_TAB:
							return keyboard_input::TAB;
						case GLFW_KEY_Q:
							return keyboard_input::Q;
						case GLFW_KEY_W:
							return keyboard_input::W;
						case GLFW_KEY_E:
							return keyboard_input::E;
						case GLFW_KEY_R:
							return keyboard_input::R;
						case GLFW_KEY_T:
							return keyboard_input::T;
						case GLFW_KEY_Y:
							return keyboard_input::Y;
						case GLFW_KEY_U:
							return keyboard_input::U;
						case GLFW_KEY_I:
							return keyboard_input::I;
						case GLFW_KEY_O:
							return keyboard_input::O;
						case GLFW_KEY_P:
							return keyboard_input::P;
						case GLFW_KEY_LEFT_BRACKET:
							return keyboard_input::LEFT_SQURE_BRACKET;
						case GLFW_KEY_RIGHT_BRACKET:
							return keyboard_input::RIGHT_SQUARE_BRACKET;
						case GLFW_KEY_BACKSLASH:
							return keyboard_input::BACKSLASH;
						case GLFW_KEY_DELETE:
							return keyboard_input::DELETE;
						case GLFW_KEY_END:
							return keyboard_input::END;
						case GLFW_KEY_PAGE_DOWN:
							return keyboard_input::PAGE_DOWN;
						case GLFW_KEY_KP_7:
							return keyboard_input::NUMPAD_SEVEN;
						case GLFW_KEY_KP_8:
							return keyboard_input::NUMPAD_EIGHT;
						case GLFW_KEY_KP_9:
							return keyboard_input::NUMPAD_NINE;
						case GLFW_KEY_KP_ADD:
							return keyboard_input::NUMPAD_PLUS;
						case GLFW_KEY_CAPS_LOCK:
							return keyboard_input::CAPS_LOCK;
						case GLFW_KEY_A:
							return keyboard_input::A;
						case GLFW_KEY_S:
							return keyboard_input::S;
						case GLFW_KEY_D:
							return keyboard_input::D;
						case GLFW_KEY_F:
							return keyboard_input::F;
						case GLFW_KEY_G:
							return keyboard_input::G;
						case GLFW_KEY_H:
							return keyboard_input::H;
						case GLFW_KEY_J:
							return keyboard_input::J;
						case GLFW_KEY_K:
							return keyboard_input::K;
						case GLFW_KEY_L:
							return keyboard_input::L;
						case GLFW_KEY_SEMICOLON:
							return keyboard_input::SEMICOLON;
						case GLFW_KEY_APOSTROPHE:
							return keyboard_input::APOSTROPHE;
						case GLFW_KEY_ENTER:
							return keyboard_input::ENTER;
						case GLFW_KEY_KP_4:
							return keyboard_input::NUMPAD_FOUR;
						case GLFW_KEY_KP_5:
							return keyboard_input::NUMPAD_FIVE;
						case GLFW_KEY_KP_6:
							return keyboard_input::NUMPAD_SIX;
						case GLFW_KEY_LEFT_SHIFT:
							return keyboard_input::LEFT_SHIFT;
						case GLFW_KEY_Z:
							return keyboard_input::Z;
						case GLFW_KEY_X:
							return keyboard_input::X;
						case GLFW_KEY_C:
							return keyboard_input::C;
						case GLFW_KEY_V:
							return keyboard_input::V;
						case GLFW_KEY_B:
							return keyboard_input::B;
						case GLFW_KEY_N:
							return keyboard_input::N;
						case GLFW_KEY_M:
							return keyboard_input::M;
						case GLFW_KEY_COMMA:
							return keyboard_input::COMMA;
						case GLFW_KEY_PERIOD:
							return keyboard_input::PEROID;
						case GLFW_KEY_SLASH:
							return keyboard_input::SLASH;
						case GLFW_KEY_RIGHT_SHIFT:
							return keyboard_input::RIGHT_SHIFT;
						case GLFW_KEY_UP:
							return keyboard_input::UP_ARROW;
						case GLFW_KEY_KP_1:
							return keyboard_input::NUMPAD_ONE;
						case GLFW_KEY_KP_2:
							return keyboard_input::NUMPAD_TWO;
						case GLFW_KEY_KP_3:
							return keyboard_input::NUMPAD_THREE;
						case GLFW_KEY_KP_ENTER:
							return keyboard_input::NUMPAD_ENTER;
						case GLFW_KEY_LEFT_CONTROL:
							return keyboard_input::LEFT_CONTROL;
						case GLFW_KEY_LEFT_SUPER:
							return keyboard_input::LEFT_OS;
						case GLFW_KEY_LEFT_ALT:
							return keyboard_input::LEFT_ALT;
						case GLFW_KEY_SPACE:
							return keyboard_input::SPACEBAR;
						case GLFW_KEY_RIGHT_ALT:
							return keyboard_input::RIGHT_ALT;
						case GLFW_KEY_RIGHT_SUPER:
							return keyboard_input::RIGHT_OS;
						// case GLFW_KEY_RIGHT_CLICK:
						// 	return keyboard_input::RIGHT_CLICK;
						case GLFW_KEY_RIGHT_CONTROL:
							return keyboard_input::RIGHT_CONTROL;
						case GLFW_KEY_LEFT:
							return keyboard_input::LEFT_ARROW;
						case GLFW_KEY_DOWN:
							return keyboard_input::DOWN_ARROW;
						case GLFW_KEY_RIGHT:
							return keyboard_input::RIGHT_ARROW;
						case GLFW_KEY_KP_0:
							return keyboard_input::NUMPAD_ZERO;
						case GLFW_KEY_KP_DECIMAL:
							return keyboard_input::NUMPAD_PEROID;
						default:
							return keyboard_input::ESCAPE;
					}
				}()
			});
		}
	}

	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) noexcept {
		auto* glfw_user_pointer = reinterpret_cast<glfw_usr_ptr*>(glfwGetWindowUserPointer(window));
		if (glfw_user_pointer->ui_state->input_capture) {
			std::scoped_lock lock{glfw_user_pointer->ui_state->eob_state->input_events_mutex};
			glfw_user_pointer->ui_state->eob_state->input_events.emplace_back(mouse_movement_input{
				xpos,
				ypos
			});
		}
	}

	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) noexcept {
		auto* glfw_user_pointer = reinterpret_cast<glfw_usr_ptr*>(glfwGetWindowUserPointer(window));
		if (glfw_user_pointer->ui_state->input_capture) {
			std::scoped_lock lock{glfw_user_pointer->ui_state->eob_state->input_events_mutex};
			glfw_user_pointer->ui_state->eob_state->input_events.emplace_back(mouse_button_input{
				(action == GLFW_PRESS) ? mouse_button_input::DOWN : mouse_button_input::UP,
				[&]() noexcept {
					switch (button) {
						case GLFW_MOUSE_BUTTON_LEFT:
							return mouse_button_input::LEFT;
						case GLFW_MOUSE_BUTTON_MIDDLE:
							return mouse_button_input::MIDDLE;
						case GLFW_MOUSE_BUTTON_RIGHT:
							return mouse_button_input::RIGHT;
						default:
							return mouse_button_input::LEFT;
					}
				}()
			});
		}
	}

	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
		auto* glfw_user_pointer = reinterpret_cast<glfw_usr_ptr*>(glfwGetWindowUserPointer(window));
		if (glfw_user_pointer->ui_state->input_capture) {
			std::scoped_lock lock{glfw_user_pointer->ui_state->eob_state->input_events_mutex};
			glfw_user_pointer->ui_state->eob_state->input_events.emplace_back(mouse_scroll_input{
				xoffset,
				yoffset
			});
		}
	}

	static void create_capture_glfw_window(ui_state_t& ui_state, glfw_usr_ptr& glfw_user_pointer) noexcept {
		ui_state.capture_window = glfwCreateWindow(1920, 1080, "exaocbot capture", NULL, NULL);
		if (ui_state.capture_window == nullptr) {
			std::cerr << "could not create capture glfw window" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		glfwSetWindowUserPointer(ui_state.capture_window, &glfw_user_pointer);
		glfwSetKeyCallback(ui_state.capture_window, key_callback);
		glfwSetCursorPosCallback(ui_state.capture_window, cursor_position_callback);
		glfwSetMouseButtonCallback(ui_state.capture_window, mouse_button_callback);
		glfwSetScrollCallback(ui_state.capture_window, scroll_callback);
	}

	static void create_ui_glfw_window(ui_state_t& ui_state, glfw_usr_ptr& glfw_user_pointer) noexcept {
		ui_state.ui_window = glfwCreateWindow(1280, 720, "exaocbot", NULL, NULL);
		if (ui_state.ui_window == nullptr) {
			std::cerr << "could not create glfw window" << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}

	static void init_imgui(ui_state_t& ui_state) noexcept {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;

		ImGui::StyleColorsDark();
	}

	static void loop_imgui(ui_state_t& ui_state) noexcept {
		ImGui::NewFrame();

		auto& capture_state = ui_state.eob_state->capture_state;
		{
			ImGui::Begin("exaocbot app");

			{
				std::scoped_lock devices_lock{capture_state.capture_devices_mutex};
				ui_state.capture_combo_strings = {};
				ui_state.capture_combo_strings.reserve(capture_state.capture_devices.size() + 1);
				if (capture_state.capture_devices.size() == 0) {
					auto& arr = ui_state.capture_combo_strings.emplace_back();
					std::memcpy(arr.data(), "no device", 10);
				} else {
					for (const auto& device : capture_state.capture_devices) {
						auto& arr = ui_state.capture_combo_strings.emplace_back();
						std::memcpy(arr.data(), device->name.data(), std::min(device->name.size(), ui_state_t::capture_combo_string_t().size()));
					}
				}

				if (ImGui::BeginCombo("##combo", ui_state.capture_current_combo_item)) {
					for (auto& combo_item : ui_state.capture_combo_strings) {
						bool is_selected = (ui_state.capture_current_combo_item == combo_item.data());
						if (ImGui::Selectable(combo_item.data(), is_selected)) {
							ui_state.capture_current_combo_item = combo_item.data();
						}
						if (is_selected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				if (capture_state.capture_devices.size() > 0 && ui_state.capture_current_combo_item != nullptr) {
					std::scoped_lock capture_config_lock{capture_state.capture_config_mutex};
					uint32_t i = std::distance(ui_state.capture_combo_strings.data(), reinterpret_cast<ui_state_t::capture_combo_string_t*>(ui_state.capture_current_combo_item));
					auto& device = capture_state.capture_devices[i];
					if (capture_state.capture_config.current_device != nullptr) {
						if (capture_state.capture_config.current_device.get() != device.get()) {
							capture_state.capture_config.current_device = device;
							capture_state.capture_config.dirty = true;
						}
					} else {
						capture_state.capture_config.current_device = device;
						capture_state.capture_config.dirty = true;
					}
				}
			}

			ImGui::InputText("##partname", ui_state.eob_state->part_name_str.data(), ui_state.eob_state->part_name_str.size());
			ImGui::InputText("##benchmark", ui_state.eob_state->benchmark_name_short_str.data(), ui_state.eob_state->benchmark_name_short_str.size());
			ImGui::InputText("##score", ui_state.eob_state->benchmark_score_str.data(), ui_state.eob_state->benchmark_score_str.size());

			if (ImGui::Button("Screenshot") && capture_state.image_buffer != std::nullopt) {
				save_screenshot(*ui_state.eob_state);
			}

			ImGui::Text("Input: ");
			ImGui::SameLine();
			if (ui_state.input_capture) {
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "ON");
			} else {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "OFF");
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		{
			ImGui::Begin("exaocbot controller");
			ImGui::Text("State: ");
			ImGui::SameLine();
			if (ui_state.eob_state->msg_state.connection_state == msg_state_t::CONNECTED) {
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "CONNECTED");
			} else if (ui_state.eob_state->msg_state.connection_state == msg_state_t::CONNECTING) {
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "CONNECTING");
			} else {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "DISCONNECTED");
			}
			ImGui::End();
		}
	}

	static void cleanup_ui(ui_state_t& ui_state) noexcept {
		glfwDestroyWindow(ui_state.capture_window);
		glfwDestroyWindow(ui_state.ui_window);
		glfwTerminate();
	}

	void create_ui(exaocbot_state& state) noexcept {
		ui_state_t ui_state;
		ui_state.eob_state = &state;

		auto result = init_renderer<render_api::METAL>(&ui_state);
		if (result != std::nullopt) {
			ui_state.renderer = std::move(result.value());
		} else if (result = init_renderer<render_api::OPENGL>(&ui_state); result != std::nullopt) {
			ui_state.renderer = std::move(result.value());
		} else {
			std::cout << "could not initialize renderer" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		glfw_usr_ptr glfw_user_pointer{&ui_state};

		glfwSetErrorCallback(glfw_error_callback);

		if (!glfwInit()) {
			std::cerr << "could not initialize glfw" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		std::thread ui_thread([&]() noexcept -> void {
			ui_state.renderer->glfw_hints_ui();
			create_ui_glfw_window(ui_state, glfw_user_pointer);
			ui_state.renderer->glfw_ui_window_created();
			init_imgui(ui_state);
			ui_state.renderer->init_ui();

			while (!glfwWindowShouldClose(ui_state.ui_window)) {
				glfwPollEvents();

				ui_state.renderer->loop_pre_imgui();
				loop_imgui(ui_state);
				ui_state.renderer->loop_ui();
			}

			ui_state.renderer->cleanup_ui();

			ImGui::DestroyContext();

			glfwSetWindowShouldClose(ui_state.capture_window, GLFW_TRUE);
		});

		ui_state.renderer->glfw_hints_capture();
		create_capture_glfw_window(ui_state, glfw_user_pointer);
		ui_state.renderer->glfw_capture_window_created();
		ui_state.renderer->init_capture();

		while (!glfwWindowShouldClose(ui_state.capture_window)) {
			glfwPollEvents();

			if (ui_state.input_capture) {
				glfwSetInputMode(ui_state.capture_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			} else {
				glfwSetInputMode(ui_state.capture_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}

			if (ui_state.eob_state->capture_state.playback != nullptr && (ui_state.image_buffer_converter == nullptr || ui_state.image_buffer_converter->input_format() != ui_state.eob_state->capture_state.playback->image_format)) {
				if (ui_state.image_buffer_converter != nullptr) {
					ui_state.image_buffer_converter->cleanup();
					ui_state.image_buffer_converter.reset();
				}
				ui_state.image_buffer_converter = ui_state.renderer->create_image_buffer_converter(ui_state.eob_state->capture_state.playback->image_format);
				ui_state.image_buffer_converter->init();
			}

			ui_state.renderer->loop_capture();
		}

		ui_state.renderer->cleanup_capture();
		glfwSetWindowShouldClose(ui_state.ui_window, GLFW_TRUE);

		ui_thread.join();

		cleanup_ui(ui_state);
	}
} // namespace exaocbot
