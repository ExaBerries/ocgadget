#include "capture.h"
#include "v4l2.h"
#include "test_cap_src.h"

namespace ocgadget {
	void capture_apis_init(capture_state_t& state) noexcept {
		state.capture_apis.emplace_back(std::make_shared<v4l2>());
		state.capture_apis.emplace_back(std::make_shared<test_cap_src>());
	}
} // namespace ocgadget
