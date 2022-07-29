#include "ui_impl.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <cstring>

namespace ocgadget {
	void init_imgui(ui_state_t& ui_state) noexcept {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;

		ImGui::StyleColorsDark();
	}

	void loop_imgui(ui_state_t& ui_state) noexcept {
		ImGui::NewFrame();

		auto& capture_state = ui_state.eob_state->capture_state;
		{
			ImGui::Begin("ocgadget app");

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
			ImGui::Begin("ocgadget controller");
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

	void cleanup_imgui(ui_state_t& ui_state) noexcept {
		ImGui::DestroyContext();
	}
} // namespace ocgadget
