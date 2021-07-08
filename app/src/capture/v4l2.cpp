#include "v4l2.h"
#include "capture.h"
#include <iostream>
#include <cstring>
#include <filesystem>
#include <type_traits>
extern "C" {
	#include <fcntl.h>
	#include <sys/ioctl.h>
	#include <unistd.h>
	#include <sys/mman.h>
	#include <linux/videodev2.h>
}

namespace exaocbot {
	static std::string deocde_ioctl_error() noexcept {
		switch (errno) {
			case EINVAL:
				return "EINVAL";
			case EAGAIN:
				return "EAGAIN";
			case EIO:
				return "EIO";
			case EPIPE:
				return "EPIPE";
			case EBADF:
				return "EBADF";
			default:
				return std::to_string(errno);
		}
	}

	struct v4l2_file_descriptor {
		int fd = 0;

		v4l2_file_descriptor(const char* path) noexcept;
		~v4l2_file_descriptor() noexcept;
	};

	v4l2_file_descriptor::v4l2_file_descriptor(const char* path) noexcept {
		fd = open(path, O_RDWR);
	}

	v4l2_file_descriptor::~v4l2_file_descriptor() noexcept {
		close(fd);
	}

	struct v4l2_playback : public capture_playback {
		std::shared_ptr<v4l2_file_descriptor> fd{};
		uint8_t* buffer_start = nullptr;
		v4l2_buffer bufferinfo{};

		void load_texture_data(image_buffer_t& image) noexcept;
		void start_streaming() noexcept;
		void stop_streaming() noexcept;
	};

	void v4l2_playback::load_texture_data(image_buffer_t& image) noexcept {
		struct timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(fd->fd, &fds);
		int r = select(fd->fd + 1, &fds, NULL, NULL, &tv);
		if (r == 0) {
			capturing = false;
			return;
		}

		capturing = true;

		if (ioctl(fd->fd, VIDIOC_DQBUF, &bufferinfo) < 0) {
			std::cerr << "error dequeueing " << deocde_ioctl_error() << std::endl;
		}

		if (ioctl(fd->fd, VIDIOC_QBUF, &bufferinfo) < 0) {
			std::cerr << "error queueing " << deocde_ioctl_error() << std::endl;
		}

		std::memcpy(image.buffer.data(), buffer_start, bufferinfo.length);
	}

	void v4l2_playback::start_streaming() noexcept {
		std::memset(&bufferinfo, 0, sizeof(bufferinfo));

		bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		bufferinfo.memory = V4L2_MEMORY_MMAP;
		bufferinfo.index = 0;

		if (ioctl(fd->fd, VIDIOC_QBUF, &bufferinfo) < 0) {
			std::cerr << "error queueing before streaming start " << deocde_ioctl_error() << std::endl;
			std::exit(EXIT_FAILURE);
		}

		if (ioctl(fd->fd, VIDIOC_STREAMON, &bufferinfo.type) < 0) {
			std::cerr << "could not start v4l2 playback streaming" << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}

	void v4l2_playback::stop_streaming() noexcept {
		ioctl(fd->fd, VIDIOC_STREAMOFF, &bufferinfo.type);
	}

	[[nodiscard]] std::unique_ptr<capture_playback> v4l2::create_capture_playback(capture_state_t& state) noexcept {
		std::unique_ptr<capture_playback> new_playback = std::make_unique<v4l2_playback>();
		auto& playback = *static_cast<v4l2_playback*>(new_playback.get());

		playback.width = state.capture_config.img_config.width;
		playback.height = state.capture_config.img_config.height;
		playback.device = state.capture_config.current_device;

		auto& v4l2_device = v4l2_devices[state.capture_config.current_device.get()];
		playback.fd = std::make_shared<v4l2_file_descriptor>(v4l2_device.path.c_str());

		v4l2_format format;
		format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
		format.fmt.pix.width = state.capture_config.img_config.width;
		format.fmt.pix.height = state.capture_config.img_config.height;

		playback.image_format = image_buffer_t::YUYV_422;

		if (ioctl(playback.fd->fd, VIDIOC_S_FMT, &format) < 0) {
			std::cerr << "could not set v4l2 playback format" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		v4l2_requestbuffers bufrequest;
		bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		bufrequest.memory = V4L2_MEMORY_MMAP;
		bufrequest.count = 1;

		if (ioctl(playback.fd->fd, VIDIOC_REQBUFS, &bufrequest) < 0) {
			std::cerr << "could not request v4l2 playback buffer" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		v4l2_buffer rinfo;
		std::memset(&rinfo, 0, sizeof(rinfo));
		rinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		rinfo.memory = V4L2_MEMORY_MMAP;
		rinfo.index = 0;

		if (ioctl(playback.fd->fd, VIDIOC_QUERYBUF, &rinfo) < 0) {
			std::cerr << "could not request v4l2 playback buffer size" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		playback.buffer_start = (uint8_t*) mmap(NULL, rinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, playback.fd->fd, rinfo.m.offset);
		//std::memset(playback.buffer_start, 0, rinfo.length);

		if (playback.buffer_start == MAP_FAILED) {
			std::cerr << "could not create mmap" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		return new_playback;
	}

	void v4l2::find_capture_devices(capture_state_t& state) noexcept {
		std::vector<v4l2_capture_device> v4l2_devices_vec{};
		for (const auto& entry : std::filesystem::directory_iterator("/dev")) {
			if (entry.path().string().rfind("/dev/video", 0) == 0) {

				v4l2_file_descriptor fd{entry.path().c_str()};

				v4l2_capability cap;
				if (ioctl(fd.fd, VIDIOC_QUERYCAP, &cap) < 0) {
					if (errno == EBADF) {
						continue;
					}
					std::cerr << "error querying v4l2 device capability " << deocde_ioctl_error() << std::endl;
					std::exit(EXIT_FAILURE);
				}

				if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) || !(cap.capabilities & V4L2_CAP_STREAMING)) {
					continue;
				}

				int32_t index = 0;
				v4l2_input input;
				if (ioctl(fd.fd, VIDIOC_G_INPUT, &index) < 0) {
					continue;
				}

				memset(&input, 0, sizeof(input));
				input.index = index;

				if (ioctl(fd.fd, VIDIOC_ENUMINPUT, &input) < 0) {
					std::cerr << "error querying v4l2 device inputs " << deocde_ioctl_error() << std::endl;
					std::exit(EXIT_FAILURE);
				}

				v4l2_devices_vec.emplace_back(v4l2_capture_device{entry.path().string(), std::string(reinterpret_cast<char*>(cap.card)), std::string(reinterpret_cast<char*>(cap.bus_info))});
			}
		}

		std::scoped_lock devices_lock{state.capture_devices_mutex};
		std::vector<std::shared_ptr<capture_device>> new_capture_devices{};
		std::unordered_map<capture_device*, v4l2_capture_device> new_v4l2_devices{};

		for (auto& capture_device : state.capture_devices) {
			if (capture_device->api != this) {
				new_capture_devices.emplace_back(capture_device);
			}
		}

		for (auto& device : v4l2_devices_vec) {
			bool already_managed = false;
			v4l2_capture_device* found_v4l2 = nullptr;
			std::shared_ptr<capture_device> found_capture{};
			for (auto& [capture_device_ptr, v4l2_device] : v4l2_devices) {
				if (v4l2_device.name == device.name) {
					found_v4l2 = &v4l2_device;
					for (auto& capture_device : state.capture_devices) {
						if (capture_device.get() == capture_device_ptr) {
							found_capture = capture_device;
						}
					}
					break;
				}
			}
			if (found_v4l2 == nullptr && already_managed == false) {
				auto& new_capture_device = new_capture_devices.emplace_back(std::make_shared<capture_device>());
				new_capture_device->name = device.name;
				new_capture_device->api = this;
				new_v4l2_devices[new_capture_device.get()] = std::move(device);
			} else {
				auto& new_capture_device = new_capture_devices.emplace_back(found_capture);
				new_capture_device->name = device.name;
				new_capture_device->api = this;
				new_v4l2_devices[new_capture_device.get()] = std::move(device);
			}
		}

		state.capture_devices = std::move(new_capture_devices);
		v4l2_devices = std::move(new_v4l2_devices);
	}
} // namespace exaocbot
