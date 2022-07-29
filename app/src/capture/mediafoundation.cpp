#include "mediafoundation.h"
#pragma comment (lib, "ole32.lib")
#pragma comment (lib, "mf.lib")
#pragma comment (lib, "mfplat.lib")
#pragma comment (lib, "mfuuid.lib")
#pragma comment (lib, "mfreadwrite.lib")
#include <iostream>
#include <locale>
#include <codecvt>

namespace ocgadget {
	[[nodiscard]] std::string lpwstr_to_string(LPWSTR& str) noexcept {
		std::wstring ws(str);
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(ws.data());
	}

	media_foundation::media_foundation() noexcept {
		HRESULT hr = S_OK;

		hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (FAILED(hr)) {
			std::cerr << "error on CoInitializeEx" << std::endl;
			return;
		}

		hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
		if (FAILED(hr)) {
			std::cerr << "error on MFStartup" << std::endl;
			return;
		}
	}

	media_foundation::~media_foundation() noexcept {
		MFShutdown();

		CoUninitialize();
	}

	struct mf_playback : public capture_playback {
		IMFMediaSource* imf_device = nullptr;
		IMFSourceReader* reader = nullptr;

		void load_texture_data(image_buffer_t& image) noexcept {
			IMFSample* sample = nullptr;
			HRESULT hr = S_OK;

			DWORD stream;
			DWORD flags;
			LONGLONG timestamp;

			while (true) {
				hr = reader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &stream, &flags, &timestamp, &sample);
				if (FAILED(hr)) {
					std::cerr << "error on read sample" << std::endl;
					return;
				}

				if (flags & MF_SOURCE_READERF_STREAMTICK) {
					continue;
				}

				break;
			}

			IMFMediaBuffer* buffer;

			hr = sample->ConvertToContiguousBuffer(&buffer);
			if (FAILED(hr)) {
				std::cerr << "error on convert to buffer" << std::endl;
				return;
			}

			BYTE* data;
			DWORD size;

			hr = buffer->Lock(&data, NULL, &size);
			if (FAILED(hr)) {
				std::cerr << "error on media buffer lock" << std::endl;
				return;
			}

			std::memcpy(image.buffer.data(), data, size);

			buffer->Unlock();
			buffer->Release();
			sample->Release();
		}

		void start_streaming() noexcept {
			HRESULT hr = S_OK;

			hr = MFCreateSourceReaderFromMediaSource(imf_device, NULL, &reader);
			if (FAILED(hr)) {
				std::cerr << "error creating source reader" << std::endl;
				return;
			}

			IMFMediaType* native_type = nullptr;
			DWORD index;

			// while (SUCCEEDED(hr)) {
			// 	hr = reader->GetNativeMediaType((DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM, index, &native_type);
			// 	if (hr == MF_E_NO_MORE_TYPES) {
			// 		break;
			// 	}
			// 	DWORD width;
			// 	DWORD height;

			// 	hr = MFGetAttributeSize(mediaType, MF_MT_FRAME_SIZE, &width, &height);
			// }

			//native_type->Release();

			IMFMediaType* type = nullptr;

			hr = MFCreateMediaType(&type);
			if (FAILED(hr)) {
				std::cerr << "error creating media type" << std::endl;
				return;
			}

			hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			if (FAILED(hr)) {
				std::cerr << "error setting major type" << std::endl;
				return;
			}

			hr = type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2);
			if (FAILED(hr)) {
				std::cerr << "error setting subtype" << std::endl;
				return;
			}

			hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, type);
			if (FAILED(hr)) {
				std::cerr << "error setting current media type" << std::endl;
				return;
			}

			type->Release();
		}

		void stop_streaming() noexcept {
			reader->Release();
			//imf_device->Release();
		}
	};

	[[nodiscard]] std::unique_ptr<capture_playback> media_foundation::create_capture_playback(capture_state_t& state) noexcept {
		HRESULT hr = S_OK;

		auto new_playback = std::make_unique<mf_playback>();
		auto& playback = *static_cast<mf_playback*>(new_playback.get());

		auto& mf_device = mf_devices[state.capture_config.current_device.get()];

		IMFAttributes* attr;

		hr = MFCreateAttributes(&attr, 2);
		if (FAILED(hr)) {
			std::cerr << "error on MFCreateAttributes" << std::endl;
			return new_playback;
		}

		hr = attr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
		if (FAILED(hr)) {
			std::cerr << "error on IMFAttributes_SetGUID" << std::endl;
			return new_playback;
		}

		auto symlink_wstr = std::wstring(mf_device.symlink.begin(), mf_device.symlink.end());

		hr = attr->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, symlink_wstr.data());
		if (FAILED(hr)) {
			std::cerr << "error setting name for finding capture device" << std::endl;
			return new_playback;
		}

		hr = MFCreateDeviceSource(attr, &playback.imf_device);
		if (FAILED(hr)) {
			std::cerr << "error creating media foundation device by name" << std::endl;
			return new_playback;
		}

		attr->Release();

		playback.width = state.capture_config.img_config.width;
		playback.height = state.capture_config.img_config.height;
		playback.device = state.capture_config.current_device;
		playback.image_format = image_buffer_t::YUYV_422;

		return new_playback;
	}

	void media_foundation::find_capture_devices(capture_state_t& state) noexcept {
		std::vector<mf_device> mf_devices_vec{};

		HRESULT hr = S_OK;

		uint32_t count;

		IMFActivate** devices;

		IMFAttributes* attr;

		hr = MFCreateAttributes(&attr, 1);
		if (FAILED(hr)) {
			std::cerr << "error on MFCreateAttributes" << std::endl;
			return;
		}

		hr = attr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
		if (FAILED(hr)) {
			std::cerr << "error on IMFAttributes_SetGUID" << std::endl;
			return;
		}

		hr = MFEnumDeviceSources(attr, &devices, &count);
		if (FAILED(hr)) {
			std::cerr << "error enumerating devices" << std::endl;
			return;
		}

		attr->Release();

		for (uint32_t i = 0; i < count; i++) {
			uint32_t length;
			LPWSTR name;
			LPWSTR symlink;

			hr = devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &name, &length);
			if (FAILED(hr)) {
				std::cerr << "error getting device name" << std::endl;
				continue;
			}

			hr = devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &symlink, &length);
			if (FAILED(hr)) {
				std::cerr << "error getting device symlink" << std::endl;
				continue;
			}

			mf_device new_device{lpwstr_to_string(name), lpwstr_to_string(symlink)};

			CoTaskMemFree(name);
			CoTaskMemFree(symlink);
			devices[i]->Release();

			mf_devices_vec.emplace_back(new_device);
		}

		CoTaskMemFree(devices);

		std::scoped_lock devices_lock{state.capture_devices_mutex};
		std::vector<std::shared_ptr<capture_device>> new_capture_devices{};
		std::unordered_map<capture_device*, mf_device> new_mf_devices{};

		for (auto& capture_device : state.capture_devices) {
			if (capture_device->api != this) {
				new_capture_devices.emplace_back(capture_device);
			}
		}

		for (auto& device : mf_devices_vec) {
			bool already_managed = false;
			mf_device* found_mf = nullptr;
			std::shared_ptr<capture_device> found_capture{};
			for (auto& [capture_device_ptr, mf_device] : mf_devices) {
				if (mf_device.name == device.name) {
					found_mf = &mf_device;
					for (auto& capture_device : state.capture_devices) {
						if (capture_device.get() == capture_device_ptr) {
							found_capture = capture_device;
						}
					}
					break;
				}
			}
			if (found_mf == nullptr && already_managed == false) {
				auto& new_capture_device = new_capture_devices.emplace_back(std::make_shared<capture_device>());
				new_capture_device->name = device.name;
				new_capture_device->api = this;
				new_mf_devices[new_capture_device.get()] = std::move(device);
			} else {
				auto& new_capture_device = new_capture_devices.emplace_back(found_capture);
				new_capture_device->name = device.name;
				new_capture_device->api = this;
				new_mf_devices[new_capture_device.get()] = std::move(device);
			}
		}

		state.capture_devices = std::move(new_capture_devices);
		mf_devices = std::move(new_mf_devices);
	}
} // namespace ocgadget
