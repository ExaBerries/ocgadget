#include "avfoundation.h"
#include <iostream>

@interface avfoundation_playback_delegate : NSObject<AVCaptureVideoDataOutputSampleBufferDelegate>

@end

namespace exaocbot {
	avfoundation::avfoundation() noexcept {
		NSArray* device_types = [NSArray arrayWithObjects:AVCaptureDeviceTypeExternalUnknown, nil];
		discovery_session = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:device_types mediaType:AVMediaTypeVideo position:AVCaptureDevicePositionUnspecified];
	}

	struct avfoundation_playback : public capture_playback {
		AVCaptureSession* session = nullptr;
		AVCaptureDeviceInput* input = nullptr;
		AVCaptureVideoDataOutput* output = nullptr;
		avfoundation_playback_delegate* delegate = nullptr;

		void load_texture_data(image_buffer_t& image) noexcept {
		}

		void start_streaming() noexcept {
			[session startRunning];
		}

		void stop_streaming() noexcept {
		}
	};

	[[nodiscard]] std::unique_ptr<capture_playback> avfoundation::create_capture_playback(capture_state_t& state) noexcept {
		std::unique_ptr<capture_playback> new_playback = std::make_unique<avfoundation_playback>();
		auto& playback = *static_cast<avfoundation_playback*>(new_playback.get());

		playback.width = state.capture_config.img_config.width;
		playback.height = state.capture_config.img_config.height;
		playback.device = state.capture_config.current_device;

		auto& avfoundation_device = avfoundation_devices[state.capture_config.current_device.get()];

		NSError* error = nullptr;
		playback.input = [AVCaptureDeviceInput deviceInputWithDevice:avfoundation_device.avf_device error:&error];

		playback.output = [[AVCaptureVideoDataOutput alloc] init];
		playback.output.alwaysDiscardsLateVideoFrames = true;

		dispatch_queue_t queue;
		queue = dispatch_queue_create("exaocbot_queue", NULL);
		[playback.output setSampleBufferDelegate:playback.delegate queue:queue];

		NSString* key = (NSString*)kCVPixelBufferPixelFormatTypeKey; 
		NSNumber* value = [NSNumber numberWithUnsignedInt:kCVPixelFormatType_24RGB];
		NSDictionary* videoSettings = [NSDictionary dictionaryWithObject:value forKey:key];
		[playback.output setVideoSettings:videoSettings]; 

		playback.session = [[AVCaptureSession alloc] init];
		if (playback.input) {
			if ([playback.session canAddInput:playback.input]) {
				[playback.session addInput:playback.input];
			}

			if ([playback.session canAddOutput:playback.output]) {
				[playback.session addOutput:playback.output];
			}
		}

		return new_playback;
	}
	
	void avfoundation::find_capture_devices(capture_state_t& state) noexcept {
		std::vector<avfoundation_capture_device> avfoundation_devices_vec{};
		for (AVCaptureDevice* device in discovery_session.devices) {
			avfoundation_devices_vec.emplace_back(avfoundation_capture_device{std::string([device.localizedName UTF8String]), device});	
		}

		std::scoped_lock devices_lock{state.capture_devices_mutex};
		std::vector<std::shared_ptr<capture_device>> new_capture_devices{};
		std::unordered_map<capture_device*, avfoundation_capture_device> new_avfoundation_devices{};

		for (auto& capture_device : state.capture_devices) {
			if (capture_device->api != this) {
				new_capture_devices.emplace_back(capture_device);
			}
		}

		for (auto& device : avfoundation_devices_vec) {
			bool already_managed = false;
			avfoundation_capture_device* found_avfoundation = nullptr;
			std::shared_ptr<capture_device> found_capture{};
			for (auto& [capture_device_ptr, avfoundation_device] : avfoundation_devices) {
				if (avfoundation_device.name == device.name) {
					found_avfoundation = &avfoundation_device;
					for (auto& capture_device : state.capture_devices) {
						if (capture_device.get() == capture_device_ptr) {
							found_capture = capture_device;
						}
					}
					break;
				}
			}
			if (found_avfoundation == nullptr && already_managed == false) {
				auto& new_capture_device = new_capture_devices.emplace_back(std::make_shared<capture_device>());
				new_capture_device->name = device.name;
				new_capture_device->api = this;
				new_avfoundation_devices[new_capture_device.get()] = std::move(device);
			} else {
				auto& new_capture_device = new_capture_devices.emplace_back(found_capture);
				new_capture_device->name = device.name;
				new_capture_device->api = this;
				new_avfoundation_devices[new_capture_device.get()] = std::move(device);
			}
		}

		state.capture_devices = std::move(new_capture_devices);
		avfoundation_devices = std::move(new_avfoundation_devices);
	}
} // namespace exaocbot
