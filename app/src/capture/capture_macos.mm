#include "capture.h"
#include "avfoundation.h"
#include "test_cap_src.h"

namespace exaocbot {
	void capture_apis_init(capture_state_t& state) noexcept {
		state.capture_apis.emplace_back(std::make_shared<avfoundation>());
		state.capture_apis.emplace_back(std::make_shared<test_cap_src>());
	}
} // namespace exaocbot
