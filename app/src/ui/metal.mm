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

namespace ocgadget {
	struct mtl_image_converter_data {
		ui_state_t* ui_state = nullptr;;
		id<MTLDevice> device;
		CAMetalLayer* layer = nullptr;
		id<MTLCommandQueue> command_queue;
		MTLRenderPassDescriptor* main_pass = nullptr;
		id<MTLCommandBuffer> buffer;
		id<MTLRenderCommandEncoder> encoder;
		id<MTLLibrary> library;
		id<MTLFunction> vert;
		id<MTLFunction> frag;
		id<MTLRenderPipelineState> state;
		id<MTLTexture> tex;
	};

	struct metal_rgba_passthrough_buffer_converter : public image_buffer_converter_t {
		mtl_image_converter_data* data = nullptr;
		
		metal_rgba_passthrough_buffer_converter(mtl_image_converter_data* data) noexcept : data(data) {
		};

		~metal_rgba_passthrough_buffer_converter() noexcept = default;

		void init() noexcept override {
			MTLTextureDescriptor* tex_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:1920 height:1080 mipmapped:false];
			data->tex = [data->device newTextureWithDescriptor:tex_desc];
		}

		void load_texture() noexcept override {
			[data->tex replaceRegion:MTLRegionMake2D(0, 0, 1920, 1080) mipmapLevel:0 withBytes:data->ui_state->eob_state->capture_state.image_buffer->buffer.data() bytesPerRow:(1920u * 4u)];
		}

		void cleanup() noexcept override {
		}

		[[nodiscard]] image_buffer_t::image_format_t input_format() noexcept override {
			return image_buffer_t::RGBA;
		}
	};

	struct metal_cpu_yuyv_422_buffer_converter : public image_buffer_converter_t {
		mtl_image_converter_data* data = nullptr;
		std::vector<uint8_t> buffer{};
		
		metal_cpu_yuyv_422_buffer_converter(mtl_image_converter_data* data) noexcept : data(data) {
		};

		~metal_cpu_yuyv_422_buffer_converter() noexcept = default;

		void init() noexcept override {
			MTLTextureDescriptor* tex_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:1920 height:1080 mipmapped:false];
			data->tex = [data->device newTextureWithDescriptor:tex_desc];
		}

		void load_texture() noexcept override {
			if (buffer.size() != data->ui_state->eob_state->capture_state.image_buffer->width * data->ui_state->eob_state->capture_state.image_buffer->height * 4u) {
				buffer.resize(data->ui_state->eob_state->capture_state.image_buffer->width * data->ui_state->eob_state->capture_state.image_buffer->height * 4u);
			}
			convert_yuyv_422_to_rgba(*data->ui_state->eob_state->capture_state.image_buffer, buffer.data());

			[data->tex replaceRegion:MTLRegionMake2D(0, 0, 1920, 1080) mipmapLevel:0 withBytes:buffer.data() bytesPerRow:(1920u * 4u)];
		}

		void cleanup() noexcept override {
		}

		[[nodiscard]] image_buffer_t::image_format_t input_format() noexcept override {
			return image_buffer_t::YUYV_422;
		}
	};

	struct metal_renderer : public renderer_t {
		ui_state_t* ui_state = nullptr;
		mtl_image_converter_data data{};
		id<CAMetalDrawable> current_capture_drawable;
		NSWindow* capture_cocoa_window = nullptr;
		NSWindow* ui_cocoa_window = nullptr;
		id<MTLDevice> ui_device;
		CAMetalLayer* ui_layer = nullptr;
		id<CAMetalDrawable> current_ui_drawable;
		id<MTLCommandQueue> ui_command_queue;
		MTLRenderPassDescriptor* ui_main_pass = nullptr;
		id<MTLCommandBuffer> ui_buffer;
		id<MTLRenderCommandEncoder> ui_encoder;
		std::mutex commit_mutex;
	
		metal_renderer(ui_state_t* state) noexcept : ui_state(state) {
			data.ui_state = state;
		}

		void glfw_hints_capture() noexcept override {
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}

		void glfw_hints_ui() noexcept override {
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
		
		void glfw_capture_window_created() noexcept override {
		}

		void glfw_ui_window_created() noexcept override {
		}
		
		void init_capture() noexcept override {
			data.device = MTLCreateSystemDefaultDevice();
			data.command_queue = [data.device newCommandQueue];

			data.layer = [CAMetalLayer layer];
			data.layer.device = data.device;
			data.layer.opaque = YES;
			data.layer.pixelFormat = MTLPixelFormatBGRA8Unorm;

			capture_cocoa_window = glfwGetCocoaWindow(ui_state->capture_window);
			capture_cocoa_window.contentView.layer = data.layer;
			capture_cocoa_window.contentView.wantsLayer = YES;

			data.main_pass = [MTLRenderPassDescriptor renderPassDescriptor];
			data.main_pass.colorAttachments[0].clearColor = MTLClearColorMake(0.64f, 1.0f, 0.0f, 1.0f);
			data.main_pass.colorAttachments[0].loadAction = MTLLoadActionClear;
			data.main_pass.colorAttachments[0].storeAction = MTLStoreActionStore;
			data.main_pass.depthAttachment.texture = nullptr;

			NSError* error = nullptr;

			constexpr auto* source = 
R"***(#include <metal_stdlib>
using namespace metal;

typedef struct {
	float4 position [[position]];
	float2 tex_coord;
} RasterizerData;

typedef struct {
	float3 position [[attribute(0)]];
} VertexData;

vertex RasterizerData vert(VertexData v [[stage_in]]) {
	RasterizerData out;
	out.position = float4(v.position, 1.0);
	out.tex_coord = float2((v.position.x + 1.0) / 2.0, (1.0 - v.position.y) / 2.0);
	return out;
}

fragment float4 frag(RasterizerData in [[stage_in]], texture2d<float> tex [[texture(0)]]) {
	constexpr sampler samp(mag_filter::linear, min_filter::linear);
	float4 col = tex.sample(samp, in.tex_coord);
	col.a = 1.0;
	return col;
}

)***";

			data.library = [data.device newLibraryWithSource:[NSString stringWithUTF8String:source] options:nil error:&error];
			if (!data.library) {
				NSLog(@"%@", error);
			}

			data.vert = [data.library newFunctionWithName:@"vert"];
			data.frag = [data.library newFunctionWithName:@"frag"];

			MTLRenderPipelineDescriptor* state_desc = [[MTLRenderPipelineDescriptor alloc] init];
			state_desc.vertexFunction = data.vert;
			state_desc.fragmentFunction = data.frag;
			state_desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
			state_desc.colorAttachments[0].blendingEnabled = false;
			state_desc.depthAttachmentPixelFormat = MTLPixelFormatInvalid;

			MTLVertexDescriptor* vert_desc = [MTLVertexDescriptor vertexDescriptor];
			vert_desc.attributes[0].format = MTLVertexFormatFloat3;
			vert_desc.attributes[0].offset = 0;
			vert_desc.attributes[0].bufferIndex = 0;
			vert_desc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
			vert_desc.layouts[0].stride = sizeof(float) * 3;
			vert_desc.layouts[0].stepRate = 1;

			state_desc.vertexDescriptor = vert_desc;

			data.state = [data.device newRenderPipelineStateWithDescriptor:state_desc error:&error];
		}

		void init_ui() noexcept override {
			@autoreleasepool {
				ui_device = MTLCreateSystemDefaultDevice();
				ui_command_queue = [ui_device newCommandQueue];

				ui_layer = [CAMetalLayer layer];
				ui_layer.device = ui_device;
				ui_layer.opaque = YES;
				ui_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;

				ui_cocoa_window = glfwGetCocoaWindow(ui_state->ui_window);
				ui_cocoa_window.contentView.layer = ui_layer;
				ui_cocoa_window.contentView.wantsLayer = YES;

				ui_main_pass = [MTLRenderPassDescriptor renderPassDescriptor];
				ui_main_pass.colorAttachments[0].clearColor = MTLClearColorMake(0.64f, 1.0f, 0.0f, 1.0f);
				ui_main_pass.colorAttachments[0].loadAction = MTLLoadActionClear;
				ui_main_pass.colorAttachments[0].storeAction = MTLStoreActionStore;
				ui_main_pass.depthAttachment.texture = nullptr;

				ImGui_ImplGlfw_InitForOpenGL(ui_state->ui_window, true);
				ImGui_ImplMetal_Init(ui_device);
			}
		}
		
		void loop_pre_imgui() noexcept override {
			@autoreleasepool {
				int display_w;
				int display_h;
				glfwGetFramebufferSize(ui_state->ui_window, &display_w, &display_h);
				ui_layer.drawableSize = CGSizeMake(display_w, display_h);
				current_ui_drawable = [ui_layer nextDrawable];
				
				ui_buffer = [ui_command_queue commandBuffer];

				ui_main_pass.colorAttachments[0].texture = current_ui_drawable.texture;
				
				ImGui_ImplMetal_NewFrame(ui_main_pass);
				ImGui_ImplGlfw_NewFrame();
			}
		}
		
		void loop_capture() noexcept override {
			@autoreleasepool {
				int display_w;
				int display_h;
				glfwGetFramebufferSize(ui_state->capture_window, &display_w, &display_h);
				data.layer.drawableSize = CGSizeMake(display_w, display_h);
				current_capture_drawable = [data.layer nextDrawable];

				data.buffer = [data.command_queue commandBuffer];
				[data.buffer enqueue];
				
				data.main_pass.colorAttachments[0].texture = current_capture_drawable.texture;

				data.encoder = [data.buffer renderCommandEncoderWithDescriptor:data.main_pass];
				if (ui_state->image_buffer_converter != nullptr) {
					[data.encoder pushDebugGroup:@"capture rendering"];
					[data.encoder setRenderPipelineState:data.state];
					if (ui_state->eob_state->capture_state.image_buffer_state == image_buffer_state_t::BUFFER_WRITTEN && ui_state->eob_state->capture_state.image_buffer_mutex.try_lock()) {
						ui_state->image_buffer_converter->load_texture();
						ui_state->eob_state->capture_state.image_buffer_state = image_buffer_state_t::WAITING_NEW;
						ui_state->eob_state->capture_state.image_buffer_mutex.unlock();
					}
					[data.encoder setFragmentTexture:data.tex atIndex:0];

					float vertices[] = {-1, -1, 0,
										3, -1, 0,
										-1, 3, 0};

					[data.encoder setVertexBytes:vertices length:sizeof(vertices) atIndex:0];
					[data.encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
					[data.encoder popDebugGroup];
				}
				[data.encoder endEncoding];
				[data.buffer presentDrawable:current_capture_drawable];
				std::scoped_lock lock{commit_mutex};
				[data.buffer commit];
			}
		}
		
		void loop_ui() noexcept override {
			@autoreleasepool {
				ImGui::Render();
				ImDrawData* draw_data = ImGui::GetDrawData();
				ui_encoder = [ui_buffer renderCommandEncoderWithDescriptor:ui_main_pass];
				[ui_encoder pushDebugGroup:@"Dear ImGui rendering"];
				ImGui_ImplMetal_RenderDrawData(draw_data, ui_buffer, ui_encoder);
				[ui_encoder popDebugGroup];
				[ui_encoder endEncoding];
				[ui_buffer presentDrawable:current_ui_drawable];
				std::scoped_lock lock{commit_mutex};
				[ui_buffer commit];
			}
		}

		void cleanup_capture() noexcept override {
		}

		void cleanup_ui() noexcept override {
			ImGui_ImplMetal_Shutdown();
			ImGui_ImplGlfw_Shutdown();
		}
		
		[[nodiscard]] virtual std::unique_ptr<image_buffer_converter_t> create_image_buffer_converter(image_buffer_t::image_format_t format) noexcept override {
			if (format == image_buffer_t::RGBA) {
				return std::make_unique<metal_rgba_passthrough_buffer_converter>(&data);
			} else if (format == image_buffer_t::YUYV_422) {
				return std::make_unique<metal_cpu_yuyv_422_buffer_converter>(&data);
			}
			std::cout << "could not create image buffer converter" << std::endl;
			std::exit(EXIT_FAILURE);
		}
	};

	template <>
	[[nodiscard]] std::optional<std::unique_ptr<renderer_t>> init_renderer<render_api::METAL>(ui_state_t* state) noexcept {
		return std::make_unique<metal_renderer>(state);
	}
} // namespace ocgadget
