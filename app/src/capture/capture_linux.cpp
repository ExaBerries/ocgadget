#include "capture.h"
#include "v4l2.h"

namespace exaocbot {
	void capture_apis_init(capture_state_t& state) noexcept {
		state.capture_apis.emplace_back(std::make_shared<v4l2>());
	}
} // namespace exaocbot
