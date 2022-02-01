#pragma once

#include <optional>

#include <libcamera/base/compiler.h>

namespace libcamera {

class Sequence
{
public:
	__nodiscard int update(unsigned int seq);
	void reset() { sequence_.reset(); }

private:
	std::optional<unsigned int> sequence_;
};

}; // namespace libcamera

