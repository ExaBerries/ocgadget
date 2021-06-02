#include "v4l2.h"
#include "util/image.h"
#include <iostream>
#include <cstring>
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

	v4l2_device::~v4l2_device() noexcept {
		ioctl(fd, VIDIOC_STREAMOFF, &bufferinfo.type);
		close(fd);
	}

	void v4l2_device::load_texture_data(rgb_image& image) noexcept {
		struct timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		int r = select(fd + 1, &fds, NULL, NULL, &tv);
		if (r == 0) {
			capturing = false;
			return;
		}

		capturing = true;

		if (ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0) {
			std::cerr << "error dequeueing " << deocde_ioctl_error() << std::endl;
		}

		if (ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0) {
			std::cerr << "error queueing " << deocde_ioctl_error() << std::endl;
		}

		convert_yuyv_to_rgb(buffer_start, image);
	}

	void initalize_v4l2_device(v4l2_device& device, uint32_t id, uint32_t width, uint32_t height) noexcept {
		device.width = width;
		device.height = height;

		std::string path = "/dev/video" + std::to_string(id);

		device.fd = open(path.c_str(), O_RDWR);

		v4l2_capability cap;
		ioctl(device.fd, VIDIOC_QUERYCAP, &cap);

		v4l2_format format;
		format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
		format.fmt.pix.width = width;
		format.fmt.pix.height = height;

		if (ioctl(device.fd, VIDIOC_S_FMT, &format) < 0) {
			std::cerr << "could not set v4l2 device format" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		v4l2_requestbuffers bufrequest;
		bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		bufrequest.memory = V4L2_MEMORY_MMAP;
		bufrequest.count = 1;

		if (ioctl(device.fd, VIDIOC_REQBUFS, &bufrequest) < 0) {
			std::cerr << "could not request v4l2 device buffer" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		v4l2_buffer rinfo;
		std::memset(&rinfo, 0, sizeof(rinfo));
		rinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		rinfo.memory = V4L2_MEMORY_MMAP;
		rinfo.index = 0;

		if (ioctl(device.fd, VIDIOC_QUERYBUF, &rinfo) < 0) {
			std::cerr << "could not request v4l2 device buffer size" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		device.buffer_start = (uint8_t*) mmap(NULL, rinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, device.fd, rinfo.m.offset);
		//std::memset(device.buffer_start, 0, rinfo.length);

		if (device.buffer_start == MAP_FAILED) {
			std::cerr << "could not create mmap" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		std::memset(&device.bufferinfo, 0, sizeof(device.bufferinfo));

		device.bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		device.bufferinfo.memory = V4L2_MEMORY_MMAP;
		device.bufferinfo.index = 0;

		if (ioctl(device.fd, VIDIOC_QBUF, &device.bufferinfo) < 0) {
			std::cerr << "error queueing before streaming start " << deocde_ioctl_error() << std::endl;
			std::exit(EXIT_FAILURE);
		}

		if (ioctl(device.fd, VIDIOC_STREAMON, &device.bufferinfo.type) < 0) {
			std::cerr << "could not start v4l2 device streaming" << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}
} // namespace exaocbot
