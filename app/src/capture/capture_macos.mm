#include "capture.h"
#include "avfoundation.h"

namespace exaocbot {
	void capture_apis_init(capture_state_t& state) noexcept {
		state.capture_apis.emplace_back(std::make_shared<avfoundation>());
	}
} // namespace exaocbot
