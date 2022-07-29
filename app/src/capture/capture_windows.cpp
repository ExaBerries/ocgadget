#include "capture.h"
#include "mediafoundation.h"
#include "test_cap_src.h"

namespace ocgadget {
	void capture_apis_init(capture_state_t& state) noexcept {
		state.capture_apis.emplace_back(std::make_shared<media_foundation>());
		state.capture_apis.emplace_back(std::make_shared<test_cap_src>());
	}
} // namespace ocgadget
