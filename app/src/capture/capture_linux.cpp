#include "capture.h"
#if defined(__linux__)
#include "v4l2.h"
#endif

namespace exaocbot {
	void capture_apis_init(capture_state_t& state) noexcept {
		#if defined(__linux__)
		state.capture_apis.emplace_back(std::make_shared<v4l2>());
		#endif
	}
} // namespace exaocbot
