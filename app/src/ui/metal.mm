#include "ui_impl.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_metal.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#include <iostream>

namespace exaocbot {
	struct mtl_image_converter_data {
		ui_state_t* ui_state = nullptr;;
		id<MTLDevice> device;
		CAMetalLayer* layer = nullptr;
		id<MTLCommandQueue> command_queue;
		MTLRenderPassDescriptor* main_pass = nullptr;
		id<MTLCommandBuffer> buffer;
		id<MTLRenderCommandEncoder> encoder;
	};

	struct metal_renderer : public renderer {
		ui_state_t* ui_state = nullptr;
		mtl_image_converter_data data{};
		id<CAMetalDrawable> current_drawable;
		NSWindow* cocoa_window = nullptr;
	
		metal_renderer(ui_state_t* state) noexcept : ui_state(state) {
			data.ui_state = state;
		}

		virtual void init_glfw_hints() noexcept {
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
		
		virtual void glfw_window_created() noexcept {
		}
		
		virtual void init() noexcept {
			data.device = MTLCreateSystemDefaultDevice();
			data.command_queue = [data.device newCommandQueue];

			data.layer = [CAMetalLayer layer];
			data.layer.device = data.device;
			data.layer.opaque = YES;
			data.layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
			
			cocoa_window = glfwGetCocoaWindow(ui_state->window);
			cocoa_window.contentView.layer = data.layer;
			cocoa_window.contentView.wantsLayer = YES;

			ImGui_ImplGlfw_InitForOpenGL(ui_state->window, true);
			ImGui_ImplMetal_Init(data.device);

			data.main_pass = [MTLRenderPassDescriptor renderPassDescriptor];
			data.main_pass.colorAttachments[0].clearColor = MTLClearColorMake(0.64f, 1.0f, 0.0f, 1.0f);
			data.main_pass.colorAttachments[0].loadAction = MTLLoadActionClear;
			data.main_pass.colorAttachments[0].storeAction = MTLStoreActionStore;
		}
		
		virtual void loop_pre_imgui() noexcept {
			int display_w;
			int display_h;
			glfwGetFramebufferSize(ui_state->window, &display_w, &display_h);
			data.layer.drawableSize = CGSizeMake(display_w, display_h);
			current_drawable = [data.layer nextDrawable];
			
			data.buffer = [data.command_queue commandBuffer];
			
			data.main_pass.colorAttachments[0].texture = current_drawable.texture;
			
			ImGui_ImplMetal_NewFrame(data.main_pass);
			ImGui_ImplGlfw_NewFrame();
		}
		
		virtual void loop() noexcept {
			ImGui::Render();
			ImDrawData* draw_data = ImGui::GetDrawData();
			data.encoder = [data.buffer renderCommandEncoderWithDescriptor:data.main_pass];
			[data.encoder pushDebugGroup:@"Dear ImGui rendering"];
			ImGui_ImplMetal_RenderDrawData(draw_data, data.buffer, data.encoder);
			[data.encoder popDebugGroup];
			[data.encoder endEncoding];
			[data.buffer presentDrawable:current_drawable];
			[data.buffer commit];
		}
		
		virtual void cleanup() noexcept {
			ImGui_ImplMetal_Shutdown();
			ImGui_ImplGlfw_Shutdown();
		}
		
		[[nodiscard]] virtual std::unique_ptr<image_buffer_converter_t> create_image_buffer_converter(image_buffer_t::image_format_t format) noexcept {
			std::cout << "could not create image buffer converter" << std::endl;
			std::exit(EXIT_FAILURE);
		}
	};

	template <>
	[[nodiscard]] std::optional<std::unique_ptr<renderer>> init_renderer<render_api::METAL>(ui_state_t* state) noexcept {
		return std::make_unique<metal_renderer>(state);
	}
} // namespace exaocbot
