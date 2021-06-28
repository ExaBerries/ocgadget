#include "ui_impl.h"
#if defined(__APPLE__)
#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#elif defined(__linux__)
#include <GL/glew.h>
#include <GL/gl.h>
#include <Gl/glext.h>
#endif
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

namespace exaocbot {
	struct gl_image_converter_data {
		ui_state_t* ui_state = nullptr;;
		GLuint capture_texture = 0;
		GLuint capture_render_program = 0;
		GLuint capture_vertex_buffer = 0;
	};

	struct gl_cpu_yuyv_422_buffer_converter : public image_buffer_converter_t {
		gl_image_converter_data* data = nullptr;
		std::vector<uint8_t> buffer{};

		gl_cpu_yuyv_422_buffer_converter(gl_image_converter_data* data) noexcept : data(data) {
		};

		~gl_cpu_yuyv_422_buffer_converter() noexcept = default;

		void init() noexcept override {
			glGenTextures(1, &data->capture_texture);
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, data->capture_texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data->ui_state->eob_state->capture_state.image_buffer->width, data->ui_state->eob_state->capture_state.image_buffer->height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		}

		void load_texture() noexcept override {
			if (buffer.size() != data->ui_state->eob_state->capture_state.image_buffer->width * data->ui_state->eob_state->capture_state.image_buffer->height * 3u) {
				buffer.resize(data->ui_state->eob_state->capture_state.image_buffer->width * data->ui_state->eob_state->capture_state.image_buffer->height * 3u);
			}
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, data->capture_texture);
			convert_yuyv_422_to_rgb(*data->ui_state->eob_state->capture_state.image_buffer, buffer.data());
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data->ui_state->eob_state->capture_state.image_buffer->width, data->ui_state->eob_state->capture_state.image_buffer->height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
		}

		void cleanup() noexcept override {
			glDeleteTextures(1, &data->capture_texture);
		}

		[[nodiscard]] image_buffer_t::image_format_t input_format() noexcept override {
			return image_buffer_t::YUYV_422;
		}
	};

	struct gl_rgb_passthrough_buffer_converter : public image_buffer_converter_t {
		gl_image_converter_data* data = nullptr;

		gl_rgb_passthrough_buffer_converter(gl_image_converter_data*data) noexcept : data(data) {
		}

		virtual ~gl_rgb_passthrough_buffer_converter() noexcept = default;

		void init() noexcept override {
			glGenTextures(1, &data->capture_texture);
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, data->capture_texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data->ui_state->eob_state->capture_state.image_buffer->width, data->ui_state->eob_state->capture_state.image_buffer->height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		}

		void load_texture() noexcept override {
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, data->capture_texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data->ui_state->eob_state->capture_state.image_buffer->width, data->ui_state->eob_state->capture_state.image_buffer->height, 0, GL_RGB, GL_UNSIGNED_BYTE, data->ui_state->eob_state->capture_state.image_buffer->buffer.data());
		}

		void cleanup() noexcept override {
			glDeleteTextures(1, &data->capture_texture);
		}

		[[nodiscard]] image_buffer_t::image_format_t input_format() noexcept override {
			return image_buffer_t::RGB;
		}
	};

	struct gl_gpu_compute_yuyv_422_buffer_converter : public image_buffer_converter_t {
		gl_image_converter_data* data = nullptr;
		GLuint program = 0;
		GLuint ssbo = 0;
		std::vector<uint32_t> buffer{};

		gl_gpu_compute_yuyv_422_buffer_converter(gl_image_converter_data* data) noexcept : data(data) {
		}

		virtual ~gl_gpu_compute_yuyv_422_buffer_converter() noexcept = default;

		void init() noexcept override {
			glGenTextures(1, &data->capture_texture);
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, data->capture_texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, data->ui_state->eob_state->capture_state.image_buffer->width, data->ui_state->eob_state->capture_state.image_buffer->height, 0, GL_RGBA, GL_FLOAT, nullptr);
			glBindImageTexture(2, data->capture_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

			const std::string source =
R"***(#version 430
layout(local_size_x = 1024) in;
layout(std430, binding = 1) buffer yuyv {
	uvec4 yuyv_data[];
};
layout(rgba32f, binding = 2) uniform writeonly image2D img_output;

void main() {
	vec4 pixel = vec4(1.0, 1.0, 0.2, 1.0);
	uint j = gl_GlobalInvocationID.x;
	uint i = j;
	uint xp = (i * 2) % 1920;
	uint yp = (i * 2) / 1920;

	float y = yuyv_data[j].x;
	float cb = yuyv_data[j].y;
	float cr = yuyv_data[j].w;

	float r = y + (1.4065 * (cr - 128));
	float g = y - (0.3455 * (cb - 128)) - (0.7169 * (cr - 128));
	float b = y + (1.7790 * (cb - 128));

	if (r < 0) r = 0;
	else if (r > 255) r = 255;
	if (g < 0) g = 0;
	else if (g > 255) g = 255;
	if (b < 0) b = 0;
	else if (b > 255) b = 255;

	imageStore(img_output, ivec2(xp, yp), vec4(r / 255, g / 255, b / 255, 1.0));

	y = yuyv_data[j].z;
	cb = yuyv_data[j].y;
	cr = yuyv_data[j].w;

	r = y + (1.4065 * (cr - 128));
	g = y - (0.3455 * (cb - 128)) - (0.7169 * (cr - 128));
	b = y + (1.7790 * (cb - 128));

	if (r < 0) r = 0;
	else if (r > 255) r = 255;
	if (g < 0) g = 0;
	else if (g > 255) g = 255;
	if (b < 0) b = 0;
	else if (b > 255) b = 255;

	imageStore(img_output, ivec2(xp + 1, yp), vec4(r / 255, g / 255, b / 255, 1.0));
})***";

			GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
			auto* source_ptr = source.c_str();
			glShaderSource(shader, 1, &source_ptr, NULL);
			glCompileShader(shader);

			/*GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE) {
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
				glDeleteShader(shader);
				std::cout << infoLog.data() << std::endl;
				std::exit(EXIT_FAILURE);
			}*/

			program = glCreateProgram();
			glAttachShader(program, shader);
			glLinkProgram(program);

			glGenBuffers(1, &ssbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
			glBufferData(GL_SHADER_STORAGE_BUFFER, 1920u * 1080u * 2u * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
		}

		void load_texture() noexcept override {
			if (buffer.size() != data->ui_state->eob_state->capture_state.image_buffer->buffer.size()) {
				buffer.resize(data->ui_state->eob_state->capture_state.image_buffer->buffer.size());
			}
			for (uint32_t i = 0; i < data->ui_state->eob_state->capture_state.image_buffer->buffer.size(); i++) {
				buffer[i] = data->ui_state->eob_state->capture_state.image_buffer->buffer.data()[i];
			}
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, buffer.size() * sizeof(uint32_t), buffer.data());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, data->capture_texture);
			glUseProgram(program);
			glUniform1i(glGetUniformLocation(data->capture_render_program, "img_output"), 2);
			glDispatchCompute(buffer.size() / (4 * 1024), 1, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		void cleanup() noexcept override {
			glDeleteTextures(1, &data->capture_texture);
			glDeleteProgram(program);
			glDeleteBuffers(1, &ssbo);
		}

		[[nodiscard]] image_buffer_t::image_format_t input_format() noexcept override {
			return image_buffer_t::YUYV_422;
		}
	};

	/*static void gl_debug_output(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* user_param) noexcept {
		if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

		std::cout << "---------------" << std::endl;
		std::cout << "Debug message (" << id << "): " <<  message << std::endl;

		switch (source) {
			case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
			case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
			case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
		}
		std::cout << std::endl;

		switch (type) {
			case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
			case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
			case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
			case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
			case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
			case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
			case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
		}
		std::cout << std::endl;

		switch (severity) {
			case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
			case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
			case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
			case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
		}
		std::cout << std::endl;
		std::cout << std::endl;
	}*/

	struct opengl_renderer : public renderer {
		ui_state_t* ui_state = nullptr;
		gl_image_converter_data data{};
		GLint gl_major = 0;
		GLint gl_minor = 0;

		opengl_renderer(ui_state_t* state) noexcept : ui_state(state) {
			data.ui_state = state;
		}

		virtual void init_glfw_hints() noexcept override {
			#if defined(__linux__)
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
			#elif defined(__APPLE__)
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
				glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
			#endif
			//glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
		}

		virtual void glfw_window_created() noexcept override {
			glewExperimental = true;
			if (glewInit() != GLEW_OK) {
				std::cerr << "could not initalize glew" << std::endl;
				std::exit(EXIT_FAILURE);
			}

			glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
			glGetIntegerv(GL_MINOR_VERSION, &gl_minor);

			std::cout << "OpenGL " << gl_major << "." << gl_minor << std::endl;

			/*glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(gl_debug_output, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);*/
		}
		
		virtual void init() noexcept override {
			ImGui_ImplGlfw_InitForOpenGL(ui_state->window, true);
			
			#if defined(__linux__)
				ImGui_ImplOpenGL3_Init("#version 130");
			#elif defined(__APPLE__)
				ImGui_ImplOpenGL3_Init("#version 150");
			#endif

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

			data.capture_render_program = glCreateProgram();
			glAttachShader(data.capture_render_program, vert);
			glAttachShader(data.capture_render_program, frag);
			glLinkProgram(data.capture_render_program);
			glValidateProgram(data.capture_render_program);

			glGenBuffers(1, &data.capture_vertex_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, data.capture_vertex_buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, data.capture_vertex_buffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		}

		virtual void loop_pre_imgui() noexcept override {
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
		}
		
		virtual void loop() noexcept override {
			ImGui::Render();
			int display_w;
			int display_h;
			glfwGetFramebufferSize(ui_state->window, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			glClearColor(0.64f, 1.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			if (ui_state->image_buffer_converter != nullptr) {
				if (ui_state->eob_state->capture_state.image_buffer_state == image_buffer_state_t::BUFFER_WRITTEN && ui_state->eob_state->capture_state.image_buffer_mutex.try_lock()) {
					ui_state->image_buffer_converter->load_texture();
					ui_state->eob_state->capture_state.image_buffer_state = image_buffer_state_t::WAITING_NEW;
					ui_state->eob_state->capture_state.image_buffer_mutex.unlock();
				}
				glUseProgram(data.capture_render_program);
				glActiveTexture(GL_TEXTURE0 + 2);
				glBindTexture(GL_TEXTURE_2D, data.capture_texture);
				glUniform1i(glGetUniformLocation(data.capture_render_program, "tex"), 2);
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, data.capture_vertex_buffer);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		virtual void cleanup() noexcept override {
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
		}

		[[nodiscard]] virtual std::unique_ptr<image_buffer_converter_t> create_image_buffer_converter(image_buffer_t::image_format_t format) noexcept override {
			if (format == image_buffer_t::RGB) {
				return std::make_unique<gl_rgb_passthrough_buffer_converter>(&data);
			} else if (format == image_buffer_t::YUYV_422) {
				if (gl_major >= 4 && gl_minor >= 3) {
					return std::make_unique<gl_gpu_compute_yuyv_422_buffer_converter>(&data);
				} else {
					return std::make_unique<gl_cpu_yuyv_422_buffer_converter>(&data);
				}
			}
			std::cout << "could not create image buffer converter" << std::endl;
			std::exit(EXIT_FAILURE);
		}
	};

	template <>
	[[nodiscard]] std::optional<std::unique_ptr<renderer>> init_renderer<render_api::OPENGL>(ui_state_t* state) noexcept{
		return std::make_unique<opengl_renderer>(state);
	}
} // namespace exaocbot
