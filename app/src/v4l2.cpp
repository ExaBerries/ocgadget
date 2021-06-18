#include "v4l2.h"
#include "util/image.h"
#include <iostream>
#include <cstring>
#include <filesystem>
#include <unordered_map>
#include <type_traits>
extern "C" {
	#include <fcntl.h>
	#include <sys/ioctl.h>
	#include <unistd.h>
	#include <sys/mman.h>
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

	v4l2_file_descriptor::v4l2_file_descriptor(const char* path) noexcept {
		fd = open(path, O_RDWR);
	}

	v4l2_file_descriptor::~v4l2_file_descriptor() noexcept {
		close(fd);
	}

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

	void find_v4l2_devices(std::vector<std::shared_ptr<v4l2_device>>& devices, std::mutex& devices_mutex, v4l2_config_t& v4l2_config, std::mutex& v4l2_config_mutex) noexcept {
		using devices_t = std::decay_t<decltype(devices)>;
		devices_t new_devices{};

		std::unordered_map<v4l2_device*, bool> devices_found;
		for (auto& device : devices) {
			devices_found[device.get()] = false;
		}

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

				std::string bus_info = reinterpret_cast<char*>(cap.bus_info);
				bool known = false;
				for (auto& device : devices) {
					if (std::string_view{bus_info} == std::string_view{device->bus_info}) {
						known = true;
						devices_found[device.get()] = true;
						break;
					}
				}

				if (!known) {
					std::shared_ptr<v4l2_device> device = std::make_shared<v4l2_device>();
					device->path = entry.path().string();
					device->name = reinterpret_cast<char*>(cap.card);
					device->bus_info = bus_info;

					new_devices.emplace_back(device);

					std::cout << "new device " << device->path << " " << device->name << std::endl;
				}
			}
		}

		std::scoped_lock devices_lock{devices_mutex};
		devices_t devices_new{};
		for (auto& device : devices) {
			if (devices_found[device.get()]) {
				devices_new.emplace_back(device);
			} else if (device.get() == v4l2_config.current_v4l2_device.get()) {
				std::scoped_lock v4l2_config_lock{v4l2_config_mutex};
				v4l2_config.current_v4l2_device = {};
				v4l2_config.dirty = true;
				std::cout << "lost device " << device->path << " " << device->name << std::endl;
			}
		}
		devices_new.insert(devices_new.end(), new_devices.begin(), new_devices.end());

		devices = devices_new;
	}

	[[nodiscard]] v4l2_playback create_v4l2_playback(std::shared_ptr<v4l2_device> device, uint32_t width, uint32_t height) noexcept {
		v4l2_playback playback;

		playback.width = width;
		playback.height = height;

		playback.fd = std::make_shared<v4l2_file_descriptor>(device->path.c_str());

		v4l2_format format;
		format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
		format.fmt.pix.width = width;
		format.fmt.pix.height = height;

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

		return playback;
	}
} // namespace exaocbot
